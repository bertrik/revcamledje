#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <FastLED.h>
#include <PubSubClient.h>

#define PIN_LED     D2

#define MQTT_SERVER "revspace.nl"
#define MQTT_PORT   1883
#define CAMS_TOPIC  "revspace/cams"
#define CAM_IDX     5

static WiFiManager wifiManager;
static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static char mqtt_id[32];

static CRGB led;

static void mqtt_callback(const char *topic, uint8_t * payload, unsigned int length)
{
    char data[32];
    if (length >= sizeof(data)) {
        Serial.printf("%s: data too long!\n", topic);
        return;
    }

    strncpy(data, (char *) payload, length);
    data[length] = '\0';
    Serial.printf("%s: %s\n", topic, data);

    // split
    CRGB color = CRGB::Black;
    char *next = strtok(data, " ");
    for (int idx = 0; next != NULL; idx++) {
        if (idx == CAM_IDX) {
            if (strcmp(next, "0") != 0) {
                color = CRGB::Red;
            }
        }
        next = strtok(NULL, " ");
    }
    FastLED.showColor(color);
}

void setup(void)
{
    Serial.begin(115200);
    Serial.printf("\nREVCAMLEDJE\n");

    // get ESP id
    sprintf(mqtt_id, "ESP-REVCAMLED-%06x", ESP.getChipId());
    Serial.printf("ESP ID: %s\n", mqtt_id);

    FastLED.addLeds < WS2812B, PIN_LED, RGB > (&led, 1);
    FastLED.showColor(CRGB::Blue);

    wifiManager.autoConnect(mqtt_id);

    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
}

void mqtt_alive(void)
{
    if (!mqttClient.connected()) {
        mqttClient.connect(mqtt_id);
        mqttClient.subscribe(CAMS_TOPIC);
    }
    // keep MQTT alive
    mqttClient.loop();
}


void loop(void)
{
    mqtt_alive();
}
