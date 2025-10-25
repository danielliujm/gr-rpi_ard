
import os
import json
import threading
import queue
import time
from datetime import datetime, date

from flask import Flask, Response, request, jsonify, render_template_string
import serial
import cv2

import csv
import numpy as np


# ---------------------- Configuration ----------------------
SER_PORT = os.getenv("PORT", "/dev/ttyACM0")  # e.g., "COM3" on Windows
SER_BAUD = int(os.getenv("BAUD", "9600"))
CAM_INDEX = int(os.getenv("CAM", "0"))
HTTP_HOST = os.getenv("HOST", "0.0.0.0")
HTTP_PORT = int(os.getenv("HTTP_PORT", "5000"))

# PWM channels shown in the UI (adjust as needed)
PWM_CHANNELS = int(os.getenv("PWM_CHANNELS", "8"))
NUM_TMP = 6
NUM_PH = 2
NUM_ORP = 2

# ---------------------- Globals ----------------------------
app = Flask(__name__)

ser = None
ser_lock = threading.Lock()
sensor_q = queue.Queue(maxsize=500)
latest_json = {}
running = True


# ---------------------- File saving -------------------------
log_file = open("data/data_log.csv", "w", newline='')
csv_writer = csv.writer(log_file)

csv_writer.writerow(["Timestamp", *[f"TMP(kpa) {i+1}" for i in range(NUM_TMP)],
                     *[f"PH{i+1}" for i in range(NUM_PH)],
                     *[f"ORP{i+1}" for i in range(NUM_ORP)],
                     *[f"Motor{i+1}" for i in range(PWM_CHANNELS)]])

log_dir = "data"
os.makedirs(log_dir, exist_ok=True)

current_date = date.today()
log_filename = os.path.join(log_dir, f"data_log_{current_date}.csv")
log_file = open(log_filename, "w", newline='')
csv_writer = csv.writer(log_file)

# Write header
csv_writer.writerow(["Timestamp", *[f"TMP(kpa) {i+1}" for i in range(NUM_TMP)],
                     *[f"PH{i+1}" for i in range(NUM_PH)],
                     *[f"ORP{i+1}" for i in range(NUM_ORP)],
                     *[f"Motor{i+1}" for i in range(PWM_CHANNELS)]])

# ---------------------- Serial I/O -------------------------
def check_log_rotation():
    global current_date, log_file, csv_writer
    new_date = date.today()
    if new_date != current_date:
        log_file.close()
        current_date = new_date
        log_filename = os.path.join(log_dir, f"data_log_{current_date}.csv")
        log_file = open(log_filename, "w", newline='')
        csv_writer = csv.writer(log_file)
        csv_writer.writerow(["Timestamp", *[f"TMP(kpa) {i+1}" for i in range(NUM_TMP)],
                             *[f"PH{i+1}" for i in range(NUM_PH)],
                             *[f"ORP{i+1}" for i in range(NUM_ORP)],
                             *[f"Motor{i+1}" for i in range(PWM_CHANNELS)]])

def open_serial():
    global ser
    try:
        ser = serial.Serial(SER_PORT, SER_BAUD, timeout=1)
    except Exception as e:
        print(f"[WARN] Could not open serial {SER_PORT}: {e}")
        ser = None

def write_serial_line(line: str):
    """Thread-safe write of a single text line with newline."""
    global ser
    if ser is None:
        return False, "Serial not open"
    try:
        with ser_lock:
            ser.write((line.rstrip() + "\n").encode("utf-8", errors="ignore"))
            ser.flush()
        return True, ""
    except Exception as e:
        return False, str(e)

def read_serial_forever():
    global ser, latest_json, running
    while running:
        if ser is None:
            time.sleep(1.0)
            open_serial()
            continue
        try:
            raw = ser.readline()
            if not raw:
                continue
            try:
                line = raw.decode("utf-8", errors="ignore").strip()
            except Exception:
                continue
            if not line:
                continue

            try:
                data = json.loads(line)

                if  ("log" in data and data["log"] == "on"):
                
                  check_log_rotation()
                  timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                  row = row = [timestamp] + convert_TMP(data["TMP_data"]) + data["PH_data"] + data["ORP_data"] + [int(entry) for entry in data["pwm_output"]]
                  csv_writer.writerow(row)
                  log_file.flush()
                  print (f"Logged data at time {timestamp}")
            except Exception as e:
              print(f" Error saving to file: {e}")
              
            # Expecting JSON lines from the device; if it's not JSON, we still pass text
            parsed = None
            try:
                parsed = json.loads(line)
                latest_json = parsed
                payload = {"t": time.time(), "data": parsed}
            except json.JSONDecodeError:
                payload = {"t": time.time(), "text": line}
            # push to queue (drop oldest if full)
            try:
                sensor_q.put_nowait(payload)
            except queue.Full:
                try:
                    sensor_q.get_nowait()
                except Exception:
                    pass
                sensor_q.put_nowait(payload)
        except Exception as e:
            print("[Serial read error]", e)
            time.sleep(0.5)

# ---------------------- Webcam MJPEG -----------------------
cap = None
cap_lock = threading.Lock()

# def open_camera():
#     global cap
#     try:
#         # Prefer V4L2 on Linux; OpenCV will choose an available backend otherwise
#         cap = cv2.VideoCapture(CAM_INDEX)
#         if not cap.isOpened():
#             print(f"[WARN] Could not open camera index {CAM_INDEX}")
#             cap = None
#             return
#         # # Request reasonable defaults; camera may clamp
#         # cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
#         # cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
#         # cap.set(cv2.CAP_PROP_FPS, 30)
#         # Prefer MJPEG if available
#         try:
#             fourcc = cv2.VideoWriter_fourcc(*"MJPG")
#             cap.set(cv2.CAP_PROP_FOURCC, fourcc)
#         except Exception:
#             pass
#     except Exception as e:
#         print(f"[WARN] Camera open failed: {e}")
#         cap = None
def open_camera():
    global cap
    try:
        with cap_lock:
            if cap is not None:
                return
            cap = cv2.VideoCapture(CAM_INDEX)
            if not cap.isOpened():
                print(f"[WARN] Could not open camera index {CAM_INDEX}")
                cap = None
                return
            # Ask for reasonable defaults; device may clamp
            cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1280)
            cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)
            cap.set(cv2.CAP_PROP_FPS, 30)
            try:
                fourcc = cv2.VideoWriter_fourcc(*"MJPG")
                cap.set(cv2.CAP_PROP_FOURCC, fourcc)
            except Exception:
                pass
    except Exception as e:
        print(f"[WARN] Camera open failed: {e}")
        with cap_lock:
            cap = None

  
def convert_TMP (data):
  # convert bits to voltage:
  
  data_psi_arr = []
  data_arr = np.array(data)
  voltage_range = 4
  for data in data_arr:
    # print (f'raw data is{data}')
    voltage = int(data) * 5.0/1023.0
    data_psi = -15 + (voltage - 1)/voltage_range * 45
    data_psi_arr.append (data_psi)

  return data_psi_arr


# def gen():
#         while True:
#             ok, frame = cap.read()
#             if not ok:
#                 continue
#             ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
#             if not ok:
#                 continue
#             jpg = buf.tobytes()
#             yield (b"--frame\r\nContent-Type: image/jpeg\r\nContent-Length: "
#                    + str(len(jpg)).encode() + b"\r\n\r\n" + jpg + b"\r\n")

def gen():
    """Resilient MJPEG generator: handles cap=None and read failures."""
    global cap
    while True:
        # If camera isn't open yet, try to (re)open and wait briefly
        if cap is None:
            open_camera()
            time.sleep(0.2)
            continue

        # Read a frame safely
        try:
            with cap_lock:
                ok, frame = cap.read()
        except Exception:
            # Something went wrong; reset and retry
            with cap_lock:
                try:
                    if cap:
                        cap.release()
                except Exception:
                    pass
                cap = None
            time.sleep(0.2)
            continue

        if not ok or frame is None:
            # Camera returned no frame; brief backoff and retry
            time.sleep(0.02)
            continue

        ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
        if not ok:
            continue

        jpg = buf.tobytes()
        yield (b"--frame\r\n"
               b"Content-Type: image/jpeg\r\n"
               b"Content-Length: " + str(len(jpg)).encode() + b"\r\n\r\n" +
               jpg + b"\r\n")


# ---------------------- Flask Routes -----------------------
INDEX_HTML = r"""
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Sensor Dashboard</title>
  <style>
    :root { color-scheme: dark light; }
    body { margin: 0; font-family: system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, sans-serif; }
    header { padding: .8rem 1rem; border-bottom: 1px solid #ddd; display:flex; align-items:center; gap:1rem; }
    main { padding: 1rem; display: grid; gap: 1rem; grid-template-columns: 1fr 1fr; }
    h1 { font-size: 1.1rem; margin: 0; }
    .card { border: 1px solid #ddd; border-radius: 12px; padding: 1rem; }
    table { border-collapse: collapse; width: 100%; }
    th, td { border-bottom: 1px solid #eee; padding: .4rem .6rem; text-align: left; }
    input[type="number"] { width: 5rem; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit,minmax(140px,1fr)); gap: .5rem; }
    .actions { display: flex; gap: .5rem; }
    button { padding: .5rem .8rem; border-radius: 10px; border: 1px solid #ccc; cursor: pointer; }
    img { width: 100%; height: auto; border-radius: 12px; background: #111; }
    .muted { opacity: .7; font-size: .9rem; }
    @media (max-width: 1100px) { main { grid-template-columns: 1fr; } }
    .two-col { display: grid; grid-template-columns: 1fr 1fr; gap: 1rem; }
    @media (max-width: 900px) { .two-col { grid-template-columns: 1fr; } }
  </style>
</head>
<body>
<header>
  <h1>JSON Sensor Monitor & PWM Sender</h1>
  <div class="muted">Serial: {{ser_port}} @ {{baud}} · Camera index: {{cam}}</div>
</header>

<main>
  <section class="card">
    <h2>pH</h2>
    <table><thead><tr><th>Key</th><th>Value</th></tr></thead><tbody id="ph-body"></tbody></table>
  </section>

  <section class="card">
    <h2>Alerts</h2>
    <pre id="raw-serial" style="background:#111;color:#0f0;padding:.6rem;border-radius:8px;height:200px;overflow:auto;"></pre>
    <div class="muted">Text not in any Json object gets shown here</div>
  </section>

  <section class="card">
    <h2>ORP</h2>
    <table><thead><tr><th>Key</th><th>Value</th></tr></thead><tbody id="orp-body"></tbody></table>
  </section>

  <section class="card">
    <h2>TMP</h2>
    <table><thead><tr><th>Key</th><th>Value</th></tr></thead><tbody id="tmp-body"></tbody></table>
  </section>

  <section class="card">
    <h2>PWM Outputs</h2>
    <table><thead><tr><th>Channel</th><th>Value</th></tr></thead><tbody id="pwmout-body"></tbody></table>
  </section>

  <section class="card">
    <h2>Webcam</h2>
    <img src="/video" alt="Webcam stream (MJPEG)">
    <div class="muted">If you see nothing, check camera index or that it isn't in use.</div>
  </section>

  <section class="card">
    <h2>PWM Control</h2>
    <div class="grid" id="pwm-grid"></div>
    <div class="actions" style="margin-top: .6rem;">
      <button id="send">Send PWM</button>
      <button id="estop" style="border-color:#f55;color:#f55;">E‑STOP</button>
    </div>
    <div id="status" class="muted" style="margin-top:.4rem;"></div>
    <div class="muted">Sends JSON like <code>{"pwm":[...]}</code> and <code>{"estop":true}</code> to the device.</div>
  </section>

  <section class="card">
    <h2>Calibration</h2>
    <div class="actions" style="flex-wrap:wrap; gap:.5rem;">
      <button id="cal-mode-btn">Enter calibration mode</button>
      <button id="btn-ph1">PH1</button>
      <button id="btn-ph2">PH2</button>
      <button id="btn-orp1">ORP1</button>
      <button id="btn-orp2">ORP2</button>
    </div>
    <div style="margin-top:.8rem;">
      <label for="cal-input" style="display:block;margin-bottom:.25rem;">Calibration value</label>
      <input id="cal-input" type="text" placeholder="e.g. 7.0" />
      <button id="cal-send">Send value</button>
    </div>
  </section>

  <section class="card" style="grid-column: 1 / -1;">
    <h2>Other Data</h2>
    <table><thead><tr><th>Key</th><th>Value</th></tr></thead><tbody id="other-body"></tbody></table>
    <div class="muted">Any fields not named PH_data, ORP_data, TMP_data, or PWM_outputs are shown here.</div>
  </section>
</main>

<script>
const PWM_CHANNELS = {{pwm_channels}};

let calMode = false;

async function sendCalPayload(obj) {
  try {
    const r = await fetch('/cal', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify(obj)
    });
    const j = await r.json();
    setStatus(j.ok ? 'Calibration cmd sent' : ('Error: ' + (j.error || 'unknown')));
    return j.ok;
  } catch (e) {
    setStatus('Network error sending calibration cmd');
    return false;
  }
}

async function toggleCalMode() {
  const next = !calMode;
  const ok = await sendCalPayload({ "Calibration": next ? "true" : "false" });
  if (ok) {
    calMode = next;
    document.getElementById('cal-mode-btn').textContent =
      calMode ? 'Exit calibration mode' : 'Enter calibration mode';
  }
}

function selectSensor(name) {
  return sendCalPayload({ "sensor_select": name });
}

function sendCalValue() {
  const val = (document.getElementById('cal-input').value || '').trim();
  if (!val) {
    setStatus('Enter a calibration value first');
    return;
  }
  return sendCalPayload({ "cal_value": val });
}

// Hook up listeners after DOM load
document.addEventListener('DOMContentLoaded', () => {
  document.getElementById('cal-mode-btn').addEventListener('click', toggleCalMode);
  document.getElementById('btn-ph1').addEventListener('click', () => selectSensor('PH1'));
  document.getElementById('btn-ph2').addEventListener('click', () => selectSensor('PH2'));
  document.getElementById('btn-orp1').addEventListener('click', () => selectSensor('ORP1'));
  document.getElementById('btn-orp2').addEventListener('click', () => selectSensor('ORP2'));
  document.getElementById('cal-send').addEventListener('click', sendCalValue);
});

function tmpRawToPsi(x) {
  const n = Number(x);
  if (Number.isNaN(n)) return x;
  // default assumes raw is kPa; convert to psi
  voltage_range = 4
  voltage = x * 5.0/1023.0
  data_psi = -15 + (voltage - 1)/voltage_range * 45
  data_psi = data_psi.toFixed(3)
  return data_psi;
}

function putRow(tbodyId, key, val) {
  const body = document.getElementById(tbodyId);
  if (!body) return;
  const tr = document.createElement('tr');
  const td1 = document.createElement('td');
  const td2 = document.createElement('td');
  td1.textContent = key;
  td2.textContent = (typeof val === 'object') ? JSON.stringify(val) : val;
  tr.appendChild(td1); tr.appendChild(td2);
  body.appendChild(tr);
}

function clearBodies() {
  ['ph-body','orp-body','tmp-body','pwmout-body','other-body'].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.innerHTML = '';
  });
}

// Render arrays into dedicated panels
function renderPanels(obj) {
  clearBodies();
  if (!obj) return;

  const ph = obj.PH_data ?? obj.ph ?? obj.PH;
  const orp = obj.ORP_data ?? obj.orp ?? obj.ORP;
  const tmp = obj.TMP_data ?? obj.temp ?? obj.temperature ?? obj.TMP;
  const pwmout = obj.PWM_outputs ?? obj.pwm_output ?? obj.pwm_out ?? obj.pwmOut;

  // helper to expand arrays, objects or scalars

  const expand = (prefix, val, tbodyId, transformFn) => {
    const apply = (v) => (transformFn ? transformFn(v) : v);
    if (val === undefined) return;
    if (Array.isArray(val)) {
      val.forEach((v, i) => putRow(tbodyId, `${prefix}[${i}]`, apply(v)));
    } else if (val && typeof val === 'object') {
      Object.keys(val).forEach(k => putRow(tbodyId, `${prefix}.${k}`, apply(val[k])));
    } else {
      putRow(tbodyId, prefix, apply(val));
    }
  };


  expand('PH', ph, 'ph-body');
  expand('ORP', orp, 'orp-body');
  expand('TMP', tmp, 'tmp-body', tmpRawToPsi);
  expand('PWM_outputs', pwmout, 'pwmout-body');

  // Everything else goes to Other
  Object.keys(obj).forEach(k => {
    if (['PH_data','PH','ph','ORP_data','ORP','orp','TMP_data','TMP','temp','temperature','PWM_outputs','pwm_outputs','pwm_out','pwmOut','pwm_output'].includes(k)) return;
    const v = obj[k];
    if (Array.isArray(v)) {
      v.forEach((val, idx) => putRow('other-body', `${k}[${idx}]`, val));
    } else if (v && typeof v === 'object') {
      Object.keys(v).forEach(sub => putRow('other-body', `${k}.${sub}`, v[sub]));
    } else {
      putRow('other-body', k, v);
    }
  });
}

function setStatus(msg) {
  document.getElementById('status').textContent = msg;
}

function initPWM() {
  const grid = document.getElementById('pwm-grid');
  for (let i = 0; i < PWM_CHANNELS; i++) {
    const div = document.createElement('div');
    const label = document.createElement('label');
    label.textContent = 'PWM ' + (i+1);
    label.style.display = 'block';
    const inp = document.createElement('input');
    inp.type = 'number';
    inp.value = 0;
    inp.min = -255; inp.max = 255; // adjust as needed
    inp.step = 1;
    inp.id = 'pwm_'+i;
    div.appendChild(label); div.appendChild(inp);
    grid.appendChild(div);
  }
}

async function sendPWM() {
  const arr = [];
  for (let i = 0; i < PWM_CHANNELS; i++) {
    const v = parseInt(document.getElementById('pwm_'+i).value || '0', 10);
    arr.push(isNaN(v) ? 0 : v);
  }
  try {
    const r = await fetch('/pwm', {method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({pwm: arr})});
    const j = await r.json();
    setStatus(j.ok ? 'PWM sent' : ('Error: ' + (j.error || 'unknown')));
  } catch (e) {
    setStatus('Network error sending PWM');
  }
}

async function estop() {
  try {
    const r = await fetch('/estop', {method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({estop:true})});
    const j = await r.json();
    setStatus(j.ok ? 'E‑STOP sent' : ('Error: ' + (j.error || 'unknown')));
  } catch (e) {
    setStatus('Network error sending E‑STOP');
  }
}

function appendRaw(line) {
  const pre = document.getElementById('raw-serial');
  if (!pre) return;
  pre.textContent += line + "\n";
  pre.scrollTop = pre.scrollHeight;
}

function showRaw(line){
  const div = document.getElementById('raw-serial');
  if (!div) return;
  div.textContent = line;
}


function startSSE() {
  const es = new EventSource('/events');
  es.onmessage = (ev) => {
    try {
      const payload = JSON.parse(ev.data);
      if (payload.data && typeof payload.data === 'object') {
        renderPanels(payload.data);
      }
      else if (payload.text) {
        showRaw(payload.text);
      }
    } catch (e) {}
  };
  es.onerror = () => {
    setStatus('SSE connection lost, retrying…');
  };
}

document.addEventListener('DOMContentLoaded', () => {
  initPWM();
  startSSE();
  document.getElementById('send').addEventListener('click', sendPWM);
  document.getElementById('estop').addEventListener('click', estop);
  document.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') sendPWM();
    if (e.key.toLowerCase() === 'k') estop();
  });
});
</script>
</body>
</html>
"""

@app.route("/")
def index():
    return render_template_string(
        INDEX_HTML,
        ser_port=SER_PORT,
        baud=SER_BAUD,
        cam=CAM_INDEX,
        pwm_channels=PWM_CHANNELS
    )

@app.route("/events")
def events():
    def gen():
        # Send last known first so UI isn't empty
        if latest_json:
            yield f"data: {json.dumps({'t': time.time(), 'data': latest_json})}\n\n"
        # Stream queue items
        while True:
            item = sensor_q.get()
            yield f"data: {json.dumps(item)}\n\n"
    return Response(gen(), mimetype="text/event-stream")

@app.route("/cal", methods=["POST"])
def cal():
    try:
        body = request.get_json(force=True, silent=False)
        if not isinstance(body, dict) or not body:
            return jsonify(ok=False, error="Expected a JSON object with a key/value"), 400
        ok, err = write_serial_line(json.dumps(body))
        return jsonify(ok=ok, error=err if not ok else "")
    except Exception as e:
        return jsonify(ok=False, error=str(e)), 400



@app.route("/pwm", methods=["POST"])
def pwm():
    try:
        body = request.get_json(force=True, silent=False)
        arr = body.get("pwm", [])
        if not isinstance(arr, list):
            return jsonify(ok=False, error="pwm must be a list"), 400
        # Send JSON line to device
        ok, err = write_serial_line(json.dumps({"pwm": arr}))
        return jsonify(ok=ok, error=err if not ok else "")
    except Exception as e:
        return jsonify(ok=False, error=str(e)), 400

@app.route("/estop", methods=["POST"])
def estop():
    ok, err = write_serial_line(json.dumps({"estop": True}))
    return jsonify(ok=ok, error=err if not ok else "")  

@app.route("/video")
def video():
    return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")

def shutdown():
    global running, ser, cap
    running = False
    try:
        if ser:
            with ser_lock:
                ser.close()
    except Exception:
        pass
    try:
        if cap:
            with cap_lock:
                cap.release()
    except Exception:
        pass

def main():
    open_serial()
    open_camera()
    threading.Thread(target=read_serial_forever, daemon=True).start()
    try:
        app.run(host=HTTP_HOST, port=HTTP_PORT, threaded=True)
    finally:
        shutdown()

if __name__ == "__main__":
    main()
