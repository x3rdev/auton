import socket
import struct
import cv2
import numpy as np

HOST = "0.0.0.0"
PORT = 9000

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((HOST, PORT))
server.listen(1)

print("Listening on port 9000...")

conn, addr = server.accept()
print("Connected by", addr)

while True:
  length_data = conn.recv(4)
  length = struct.unpack(">I", length_data)[0]

  data = bytearray()
  while len(data) < length:
        packet = conn.recv(length - len(data))
        # data.insert(0, packet)
        data.extend(packet)
  frame = cv2.imdecode(np.frombuffer(data, dtype=np.uint8), cv2.IMREAD_COLOR)
  if frame is not None:
      cv2.imshow("ESP32-CAM", frame)
      if cv2.waitKey(1) & 0xFF == ord('q'):
          break
  else:
      print("Failed to decode frame")
  

  