# # save as minimal_cam_stream.py
# from flask import Flask, Response
# import cv2, os

# app = Flask(__name__)
# cap = cv2.VideoCapture(int(os.getenv("CAM", "0")))

# @app.route("/video")
# def video():
#     def gen():
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
#     return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")

# if __name__ == "__main__":
#     # reachable on LAN; change port with PORT=xxxx
#     app.run(host="0.0.0.0", port=int(os.getenv("PORT", "5000")), threaded=True)

from flask import Flask, Response
import cv2, os, signal, sys

# -------------------
# Config via env vars
# -------------------
CAM_INDEX = int(os.getenv("CAM", "0"))          # which /dev/videoX (0 == /dev/video0)
WIDTH     = int(os.getenv("WIDTH", "1280"))
HEIGHT    = int(os.getenv("HEIGHT", "720"))
FPS       = int(os.getenv("FPS", "5"))          # 5 for YUY2; use 30 if MJPG
FOURCC    = os.getenv("FOURCC", "YUY2")         # "YUY2" or "MJPG"
USE_GST   = os.getenv("USE_GST", "1")           # "1" to allow GStreamer fallback

app = Flask(__name__)
cap = None

def open_camera():
    """Try V4L2 first; if that fails and USE_GST=1, try a proper GStreamer pipeline."""
    global cap

    # --- Attempt 1: V4L2 (simple & reliable on Raspberry Pi) ---
    cap = cv2.VideoCapture(CAM_INDEX, cv2.CAP_V4L2)
    # Set properties (some cams ignore until after first read; still okay to set)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH,  WIDTH)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, HEIGHT)
    # Choose FOURCC/FPS
    cc = cv2.VideoWriter_fourcc(*FOURCC)
    cap.set(cv2.CAP_PROP_FOURCC, cc)
    cap.set(cv2.CAP_PROP_FPS, FPS)

    if cap.isOpened():
        # Sanity read: ensure frames actually arrive
        ok, _ = cap.read()
        if ok:
            return True
        cap.release()

    # --- Attempt 2: GStreamer fallback (only if allowed) ---
    if USE_GST == "1":
        # Build a pipeline that matches the chosen FOURCC
        dev = f"/dev/video{CAM_INDEX}"
        if FOURCC.upper() == "MJPG" or FOURCC.upper() == "MJPEG":
            pipe = (
                f"v4l2src device={dev} ! "
                f"image/jpeg,width={WIDTH},height={HEIGHT},framerate={FPS}/1 ! "
                "jpegdec ! videoconvert ! video/x-raw,format=BGR ! "
                "appsink drop=1 max-buffers=1"
            )
        else:
            # Default to YUY2
            pipe = (
                f"v4l2src device={dev} ! "
                f"video/x-raw,format=YUY2,width={WIDTH},height={HEIGHT},framerate={FPS}/1 ! "
                "videoconvert ! video/x-raw,format=BGR ! "
                "appsink drop=1 max-buffers=1"
            )
        cap = cv2.VideoCapture(pipe, cv2.CAP_GSTREAMER)
        if cap.isOpened():
            ok, _ = cap.read()
            if ok:
                return True
        if cap is not None:
            cap.release()

    return False


@app.route("/")
def index():
    return (
        "<html><body>"
        "<h3>Camera stream</h3>"
        '<p>Open the <a href="/video">/video</a> endpoint for the MJPEG stream.</p>'
        "</body></html>"
    )


@app.route("/video")
def video():
    if cap is None or not cap.isOpened():
        return Response("Camera not available", status=503)

    def gen():
        while True:
            ok, frame = cap.read()
            if not ok:
                # If a read fails, skip this iteration instead of crashing
                continue
            ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
            if not ok:
                continue
            jpg = buf.tobytes()
            yield (
                b"--frame\r\n"
                b"Content-Type: image/jpeg\r\n"
                b"Content-Length: " + str(len(jpg)).encode() + b"\r\n\r\n" +
                jpg + b"\r\n"
            )

    return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")


def cleanup(*_):
    try:
        if cap is not None:
            cap.release()
    finally:
        sys.exit(0)


if __name__ == "__main__":
    # Open camera before starting the server (fail fast with clear error)
    if not open_camera():
        raise RuntimeError(
            "Camera failed to open. "
            "Try adjusting env vars: FOURCC=YUY2 or FOURCC=MJPG, FPS, WIDTH/HEIGHT, CAM index. "
            "If using MJPG, ensure your webcam supports it."
        )

    # Clean shutdown on Ctrl+C / SIGTERM
    signal.signal(signal.SIGINT, cleanup)
    signal.signal(signal.SIGTERM, cleanup)

    # Start Flask (reachable on LAN; change port with PORT=xxxx)
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "5000")), threaded=True)
