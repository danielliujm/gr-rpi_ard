import os
import signal
import threading
import time
from typing import Generator

from flask import Flask, Response, render_template_string, request
import cv2

# ---- Configuration ----
CAMERA_INDEX = int(os.getenv("CAM_INDEX", "0"))     # Change with CAM_INDEX env var if needed
FRAME_WIDTH  = int(os.getenv("CAM_WIDTH", "1280"))  # e.g., 640, 1280, 1920
FRAME_HEIGHT = int(os.getenv("CAM_HEIGHT", "720"))  # e.g., 480, 720, 1080
FRAME_FPS    = int(os.getenv("CAM_FPS", "30"))      # Target fps; webcam may clamp this

# ---- App Setup ----
app = Flask(__name__)

# Global camera object (singleton for simplicity)
cap = None
cap_lock = threading.Lock()
running = True

def open_camera():
    """Open the webcam and set properties."""
    global cap
    # On Windows, CAP_DSHOW can help avoid long startup delays:
    # cap = cv2.VideoCapture(CAMERA_INDEX, cv2.CAP_DSHOW)
    cap = cv2.VideoCapture(CAMERA_INDEX)
    if not cap.isOpened():
        raise RuntimeError(f"Could not open camera index {CAMERA_INDEX}")

    # Try to set resolution / fps (device may not support all)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, FRAME_WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT)
    cap.set(cv2.CAP_PROP_FPS, FRAME_FPS)

def close_camera():
    global cap
    if cap is not None:
        with cap_lock:
            try:
                cap.release()
            except Exception:
                pass
        cap = None

def mjpeg_generator() -> Generator[bytes, None, None]:
    """Yield JPEG frames in multipart/x-mixed-replace format."""
    global cap, running
    while running:
        with cap_lock:
            if cap is None:
                break
            ok, frame = cap.read()
        if not ok:
            # Brief pause then try again
            time.sleep(0.02)
            continue

        # Optional: flip horizontal for "selfie" view
        # frame = cv2.flip(frame, 1)

        # Encode to JPEG
        ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
        if not ok:
            continue
        jpg = buf.tobytes()

        # Multipart frame boundary
        yield (b"--frame\r\n"
               b"Content-Type: image/jpeg\r\n"
               b"Content-Length: " + str(len(jpg)).encode() + b"\r\n\r\n" +
               jpg + b"\r\n")
    # End of stream
    yield b"--frame--\r\n"

INDEX_HTML = """
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <title>Webcam Stream</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html,body{margin:0;background:#0b0b0b;color:#eaeaea;font-family:system-ui,Segoe UI,Roboto,Helvetica,Arial,sans-serif;}
    .wrap{max-width:960px;margin:0 auto;padding:1rem 1rem 2rem;}
    header{display:flex;align-items:center;justify-content:space-between;gap:.75rem}
    h1{font-size:1.1rem;margin:.5rem 0}
    .video{display:flex;justify-content:center;margin-top:1rem}
    img{width:100%;height:auto;max-height:85vh;object-fit:contain;background:#111;border-radius:12px}
    .meta{opacity:.8;font-size:.9rem;margin-top:.5rem}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:.75rem;margin-top:1rem}
    .card{background:#151515;border:1px solid #222;padding:.75rem;border-radius:12px}
    code{background:#111;padding:.15rem .35rem;border-radius:6px}
    a{color:#8cc8ff}
  </style>
</head>
<body>
  <div class="wrap">
    <header>
      <h1>Live Webcam Stream</h1>
    </header>

    <div class="video">
      <img src="/video_feed" alt="Webcam stream (MJPEG)">
    </div>

    <div class="grid">
      <div class="card">
        <div class="meta">If the video doesn’t load, try a different camera index:</div>
        <div>Append <code>?cam=1</code> to the URL → <a href="/?cam=1">/?cam=1</a></div>
      </div>
      <div class="card">
        <div class="meta">Resolution & FPS (requested):</div>
        <div><code>{{w}}x{{h}} @ {{fps}}fps</code></div>
      </div>
      <div class="card">
        <div class="meta">JPEG is streamed as multipart MJPEG. Works in most browsers and media elements.</div>
        <div>Path: <code>/video_feed</code></div>
      </div>
    </div>
  </div>
</body>
</html>
"""

@app.route("/")
def index():
    # Allow quick camera switch from query param
    cam_param = request.args.get("cam")
    global CAMERA_INDEX
    if cam_param is not None:
        try:
            new_index = int(cam_param)
            if new_index != CAMERA_INDEX:
                # Switch cameras
                with cap_lock:
                    close_camera()
                    CAMERA_INDEX = new_index
                    open_camera()
        except Exception:
            pass
    return render_template_string(INDEX_HTML, w=FRAME_WIDTH, h=FRAME_HEIGHT, fps=FRAME_FPS)

@app.route("/video_feed")
def video_feed():
    return Response(mjpeg_generator(),
                    mimetype="multipart/x-mixed-replace; boundary=frame")

def handle_exit(*_):
    global running
    running = False
    close_camera()
    # Flask dev server exits on KeyboardInterrupt; just ensure cleanup
    os._exit(0)

def main():
    open_camera()
    signal.signal(signal.SIGINT, handle_exit)
    signal.signal(signal.SIGTERM, handle_exit)

    # Bind to 0.0.0.0 to be reachable on your LAN; change port if needed
    host = os.getenv("HOST", "0.0.0.0")
    port = int(os.getenv("PORT", "5000"))
    # threaded=True lets Flask handle multiple clients gracefully
    app.run(host=host, port=port, threaded=True)

if __name__ == "__main__":
    main()
