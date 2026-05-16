import pygame
import socket
import struct

# ─── Config ──────────────────────────────────────────────────
ESP_IP   = "192.168.4.1"   # ESP32 AP default IP
ESP_PORT = 4210

sock   = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
target = (ESP_IP, ESP_PORT)

# ─── Pygame / Controller setup ───────────────────────────────
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    raise Exception("No controller detected!")

joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Connected: {joystick.get_name()}")

# ─── Display ─────────────────────────────────────────────────
screen = pygame.display.set_mode((520, 420))
pygame.display.set_caption("ESP32 Car Controller")
clock  = pygame.time.Clock()
font   = pygame.font.SysFont("monospace", 20)

# ─── Helpers ─────────────────────────────────────────────────
def deadzone(val, t=0.07):
    return 0.0 if abs(val) < t else val

def draw_text(text, x, y, color=(255, 255, 255)):
    screen.blit(font.render(text, True, color), (x, y))

def draw_bar(label, value, x, y, color=(0, 200, 255), min_val=-1, max_val=1):
    """Horizontal bar for visualising an axis value."""
    bar_w = 200
    draw_text(label, x, y)
    pygame.draw.rect(screen, (60, 60, 60), (x + 150, y, bar_w, 18))
    fill = int((value - min_val) / (max_val - min_val) * bar_w)
    fill = max(0, min(bar_w, fill))
    pygame.draw.rect(screen, color, (x + 150, y, fill, 18))
    draw_text(f"{value:+.2f}", x + 360, y)

# ─── Axis / Button mapping ────────────────────────────────────
STEER_AXIS    = 0   # Left stick X
THROTTLE_AXIS = 5   # Right trigger (RT)
BRAKE_AXIS    = 2   # Left  trigger (LT)
REVERSE_BTN   = 1   # B button (Xbox) / Circle (PS)

# ─── State ───────────────────────────────────────────────────
reverse_mode  = False
last_b_state  = False
print_counter = 0   # only print every N frames so terminal isn't spammed

# ─── Main loop ───────────────────────────────────────────────
while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sock.close()
            raise SystemExit

    # --- B button toggles reverse mode ---
    b_pressed = joystick.get_button(REVERSE_BTN)
    if b_pressed and not last_b_state:
        reverse_mode = not reverse_mode
        print(f"[MODE] Reverse mode {'ON 🔴' if reverse_mode else 'OFF 🟢'}")
    last_b_state = b_pressed

    # --- Read triggers (reported as -1 released → +1 fully pressed) ---
    rt = (joystick.get_axis(THROTTLE_AXIS) + 1) / 2  # 0.0 – 1.0
    lt = (joystick.get_axis(BRAKE_AXIS)    + 1) / 2  # 0.0 – 1.0

    raw_steer = joystick.get_axis(STEER_AXIS)

    # RT = go (forward normally, reverse when mode is on)
    # LT = brake (always slows/stops)
    if reverse_mode:
        raw_throttle = -(rt - lt)   # flip direction
    else:
        raw_throttle = rt - lt      # normal forward

    # --- Apply deadzone ---
    steer    = deadzone(raw_steer,    t=0.07)
    throttle = deadzone(raw_throttle, t=0.05)
    throttle = max(-1.0, min(1.0, throttle))

    # --- Map to packet values ---
    # throttle byte: 0=full reverse, 127=stop, 255=full forward
    throttle_byte = int((throttle + 1) / 2 * 255)
    steer_byte    = max(-100, min(100, int(steer * 100)))

    packet = struct.pack("Bb", throttle_byte, steer_byte)
    sock.sendto(packet, target)

    # --- Terminal print (every 10 frames = ~6x per second) ---
    print_counter += 1
    if print_counter >= 10:
        print_counter = 0
        mode_str = "REVERSE" if reverse_mode else "FORWARD"
        if abs(throttle) < 0.05:
            mode_str = "STOP"
        print(
            f"[{mode_str:<7}] "
            f"RT={rt:.2f}  LT={lt:.2f}  "
            f"Steer={steer:+.2f}  "
            f"throttle_byte={throttle_byte:3d}  steer_byte={steer_byte:+4d}  "
            f"→ {ESP_IP}:{ESP_PORT}"
        )

    # --- Draw HUD ---
    screen.fill((15, 15, 25))

    draw_text("ESP32 Car Controller", 10, 10, (100, 200, 255))
    draw_text(f"Target: {ESP_IP}:{ESP_PORT}", 10, 35, (120, 120, 120))

    # Reverse mode banner
    if reverse_mode:
        pygame.draw.rect(screen, (120, 20, 20), (0, 55, 520, 24))
        draw_text("  REVERSE MODE ON  (press B to toggle)", 10, 57, (255, 100, 100))
    else:
        draw_text("FORWARD MODE  (press B to reverse)", 10, 57, (80, 160, 80))

    draw_bar("Steering",  steer,    10, 100, color=(0, 220, 120))
    draw_bar("RT Throttle", rt,     10, 130, color=(0, 180, 255), min_val=0, max_val=1)
    draw_bar("LT Brake",    lt,     10, 160, color=(255, 140, 0),  min_val=0, max_val=1)

    # Direction label
    if throttle > 0.05:
        t_label = "REVERSE" if reverse_mode else "FORWARD"
        t_color = (255, 80, 80) if reverse_mode else (0, 255, 100)
    elif throttle < -0.05:
        t_label = "FORWARD" if reverse_mode else "REVERSE"
        t_color = (0, 255, 100) if reverse_mode else (255, 80, 80)
    else:
        t_label, t_color = "STOP", (180, 180, 180)

    draw_text(t_label, 10, 192, t_color)

    # Packet bytes
    draw_text(
        f"throttle_byte={throttle_byte:3d}   steer_byte={steer_byte:+4d}",
        10, 220, (200, 200, 100)
    )

    # Joystick dot
    cx, cy = 260, 320
    pygame.draw.circle(screen, (50, 50, 50), (cx, cy), 80)
    pygame.draw.line(screen, (80, 80, 80), (cx - 80, cy), (cx + 80, cy))
    pygame.draw.line(screen, (80, 80, 80), (cx, cy - 80), (cx, cy + 80))
    dot_x = int(cx + steer    * 75)
    dot_y = int(cy - throttle * 75)
    dot_color = (255, 80, 80) if reverse_mode else (0, 200, 255)
    pygame.draw.circle(screen, dot_color, (dot_x, dot_y), 10)
    draw_text("Steer / Throttle", cx - 65, cy + 88, (120, 120, 120))

    pygame.display.flip()
    clock.tick(60)