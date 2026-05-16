import socket
import struct
import cv2
import numpy as np
import time
# import torch
# print(torch.cuda.is_available())

HOST = "0.0.0.0"
PORT = 9000

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print("Listening on port 9000...")

conn, addr = server.accept()
print("Connected by", addr)

face_classifier = cv2.CascadeClassifier(
    cv2.data.haarcascades + "haarcascade_frontalface_default.xml"
)

def detect_faces(frame):
  grey_image = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
  faces = face_classifier.detectMultiScale(grey_image, 
                                           scaleFactor = 1.1,
                                           minNeighbors=5)
  for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 4)
  return faces

while True:
  length_data = conn.recv(4)
  length = struct.unpack(">I", length_data)[0]

  data = bytearray()
  while len(data) < length:
        packet = conn.recv(length - len(data))
        # data.insert(0, packet)
        data.extend(packet)

  last = time.time()
  frame = cv2.imdecode(np.frombuffer(data, dtype=np.uint8), cv2.IMREAD_COLOR)
  if frame is not None:
      faces = detect_faces(frame)
      
      now = time.time()
      fps = 1.0 / (now - last) if now > last else 0
      last = now

      cv2.putText(frame, f"{fps:.1f} FPS",
            (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

      cv2.imshow("ESP32-CAM", frame)
      if cv2.waitKey(1) & 0xFF == ord('q'):
          break
  else:
      print("Failed to decode frame")