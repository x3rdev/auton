#pragma once

#include "esp_camera.h"
#include <WiFi.h>

// AI-Thinker / WROVER-KIT PIN Map
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0      5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// Macro to convert 32-bit int to big-endian
#define htonl(x) __builtin_bswap32(x)

void setup();
void loop();
void camera_capture(WiFiClient& client);
void send_image(WiFiClient& client, uint8_t* buf, size_t buf_len);