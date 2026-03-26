from modulefinder import packagePathMap
import socket
import struct 

class Transmitter: 
    def __init__(self, ip: str , port: int): 
        self.target = (ip, port) 
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, throttle:int, steering:int ): 

        packet = struct.pack("Bb", throttle, steering)
        self.sock.sendto(packet, self.target)

