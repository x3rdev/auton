from transmitter import Transmitter
import pygame
import time
import os
import sys


ESP32_IP = "192.168.0.141"
ESP32_PORT = 4210

tx = Transmitter(ESP32_IP, ESP32_PORT)

SEND_INTERVAL = 1 / 50 
last_send = 0


print("Starting control loop...")


try:
    while True:
        # throttle, steering = 

        now = time.time()

        if now - last_send >= SEND_INTERVAL:
            tx.send(throttle, steering)
            print(f"Throttle: {throttle} | Steering: {steering} | To: {ESP32_IP}")
            last_send = now

except KeyboardInterrupt:
    print("\nShutting down...")
    pygame.quit()
    sys.exit()
