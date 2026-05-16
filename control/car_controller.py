import pygame
import socket
import struct

ESP_IP = "192.168.4.1"
ESP_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
target = (ESP_IP, ESP_PORT)

pygame.init()
screen = pygame.display.set_mode((540, 360))
pygame.display.set_caption("ESP32 Tank Drive Keyboard Controller")
clock = pygame.time.Clock()
font = pygame.font.SysFont("monospace", 20)

DRIVE_POWER = 100
TURN_POWER = 100

running = True

def draw_text(text, x, y, color=(255, 255, 255)):
    screen.blit(font.render(text, True, color), (x, y))

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    keys = pygame.key.get_pressed()

    left = 0
    right = 0

    if keys[pygame.K_ESCAPE]:
        running = False

    if keys[pygame.K_SPACE]:
        left = 0
        right = 0

    elif keys[pygame.K_w] or keys[pygame.K_UP]:
        left = DRIVE_POWER
        right = DRIVE_POWER

    elif keys[pygame.K_s] or keys[pygame.K_DOWN]:
        left = -DRIVE_POWER
        right = -DRIVE_POWER

    elif keys[pygame.K_a] or keys[pygame.K_LEFT]:
        left = -TURN_POWER
        right = TURN_POWER

    elif keys[pygame.K_d] or keys[pygame.K_RIGHT]:
        left = TURN_POWER
        right = -TURN_POWER

    # Diagonal-style controls
    if (keys[pygame.K_w] or keys[pygame.K_UP]) and (keys[pygame.K_a] or keys[pygame.K_LEFT]):
        left = 0
        right = DRIVE_POWER

    elif (keys[pygame.K_w] or keys[pygame.K_UP]) and (keys[pygame.K_d] or keys[pygame.K_RIGHT]):
        left = DRIVE_POWER
        right = 0

    elif (keys[pygame.K_s] or keys[pygame.K_DOWN]) and (keys[pygame.K_a] or keys[pygame.K_LEFT]):
        left = 0
        right = -DRIVE_POWER

    elif (keys[pygame.K_s] or keys[pygame.K_DOWN]) and (keys[pygame.K_d] or keys[pygame.K_RIGHT]):
        left = -DRIVE_POWER
        right = 0

    packet = struct.pack("bb", left, right)
    sock.sendto(packet, target)  

    print(f"Sent packet to {ESP_IP}:{ESP_PORT} | left={left:+4d}, right={right:+4d}")


    screen.fill((15, 15, 25))

    draw_text("ESP32 Tank Drive Controller", 10, 10, (100, 200, 255))
    draw_text(f"Target: {ESP_IP}:{ESP_PORT}", 10, 40, (160, 160, 160))

    draw_text("Controls:", 10, 85, (255, 255, 120))
    draw_text("W / Up       = Forward", 10, 115)
    draw_text("S / Down     = Reverse", 10, 145)
    draw_text("A / Left     = Spin Left", 10, 175)
    draw_text("D / Right    = Spin Right", 10, 205)
    draw_text("W + A        = Forward Left", 10, 235)
    draw_text("W + D        = Forward Right", 10, 265)
    draw_text("Space        = Stop", 10, 295)
    draw_text("Esc          = Quit", 10, 325)

    draw_text(f"Left Motor:  {left:+4d}", 300, 140, (0, 220, 120))
    draw_text(f"Right Motor: {right:+4d}", 300, 170, (0, 220, 120))

    pygame.display.flip()
    clock.tick(30)

# send stop packets before closing
for _ in range(10):
    sock.sendto(struct.pack("bb", 0, 0), target)

sock.close()
pygame.quit()