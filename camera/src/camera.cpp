#include "camera.h"

const char* ssid = "auton";
const char* password = "12345678";

const char* host = "192.168.4.4"; // PC IP
const int port = 9000;

WiFiClient client;

// Camera configuration
camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_SVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .grab_mode = CAMERA_GRAB_LATEST 
};

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Initializing camera...");
    if (esp_camera_init(&camera_config) != ESP_OK) {
        Serial.println("Camera init failed");
        while(true); // halt
    }
    Serial.println("Camera init OK");

    WiFi.begin(ssid, password);
    Serial.println("Connecting to Wi-Fi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());

    client.connect(host, port);
    Serial.println("TCP connection OK");
}

void loop() {
    if (!client.connected()) {
        Serial.println("TCP disconnected, reconnecting...");
        if (!client.connect(host, port)) {
            delay(1000);
            return;
        }
    }

    camera_capture(client);
    delay(1000 / 24); 
}

// Capture a frame and send over TCP
void camera_capture(WiFiClient& client) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        return;
    }

    send_image(client, fb->buf, fb->len);

    esp_camera_fb_return(fb);
}

// Send image bytes with 4-byte length header
void send_image(WiFiClient &client, uint8_t* buf, size_t buf_len) {
    uint32_t len_be = htonl(buf_len); // big-endian length
    client.write((uint8_t*)&len_be, 4); // send length first
    client.write(buf, buf_len);         // then send image
    // Serial.print("Sent frame: ");
    // Serial.println(buf_len);
}