# save as minimal_cam_stream.py
from flask import Flask, Response
import cv2, os

app = Flask(__name__)
cap = cv2.VideoCapture(int(os.getenv("CAM", "0")))

@app.route("/video")
def video():
    def gen():
        while True:
            ok, frame = cap.read()
            if not ok:
                continue
            ok, buf = cv2.imencode(".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 80])
            if not ok:
                continue
            jpg = buf.tobytes()
            yield (b"--frame\r\nContent-Type: image/jpeg\r\nContent-Length: "
                   + str(len(jpg)).encode() + b"\r\n\r\n" + jpg + b"\r\n")
    return Response(gen(), mimetype="multipart/x-mixed-replace; boundary=frame")

if __name__ == "__main__":
    # reachable on LAN; change port with PORT=xxxx
    app.run(host="0.0.0.0", port=int(os.getenv("PORT", "5000")), threaded=True)
