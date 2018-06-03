#include <Arduino.h>
#include <EasyOTA.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <math.h>
#include "wificonfig.h"
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include "HexapodDrive.h"
#include "sensors/adns3080.h"

#include "hexapod_motor_config.h"

#define IMG_WIDTH ADNS3080_PIXELS_X
#define IMG_HEIGHT ADNS3080_PIXELS_Y
#define SS 16
#define RESET 3

EasyOTA OTA(ARDUINO_HOSTNAME);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
ADNS3080 a3080(SS, RESET);
Locomotion::HexapodDrive hexapod(&Wire, motor_configs, NUMBER_OF_MOTORS);

bool connected = false;
unsigned long last_updated = 0;

void textThem(const char * text) {
	int tryId = 0;
  for (int count = 0; count < ws.count();) {
    if (ws.hasClient(tryId)) {
      ws.client(tryId)->text(text);
      count++;
    }
    tryId++;
  }
}

void S_printf(const char * format, ...) {
//	char buffer[512];
//	va_list args;
//	va_start (args, format);
//	vsnprintf (buffer, sizeof(buffer), format, args);
//	Serial.println(buffer);
//	va_end (args);
}

void setup() {
	OTA.addAP(WIFI_SSID, WIFI_PASSWORD);
	OTA.onConnect([](const String& ssid, EasyOTA::STATE state) {
		S_printf("Connected %s, state: %s", ssid.c_str(), state == EasyOTA::EOS_STA ? "Station" : "Access Point");
	});

	OTA.onMessage([](const String& msg, int line) {
		S_printf("OTA message: %s", msg.c_str());
	});
	// TODO replace with walker instead of ddrive.init();
	a3080.begin();
	hexapod.begin();
	SPIFFS.begin();

	server.addHandler(&ws);
	server.serveStatic("/", SPIFFS, "/web").setDefaultFile("index.html");
	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});
	server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

	ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
		if (type == WS_EVT_DATA) {
			char cmd[256] = "";
			memcpy(cmd, data, min(len, sizeof(cmd) - 1));
			DynamicJsonBuffer jsonBuffer;
			JsonObject &json = jsonBuffer.parseObject(cmd);

			hexapod.setThrust(Locomotion::Quaternion(json["power"], json["kant"], json["elevation"], json["steering"]));

		} else if (type == WS_EVT_CONNECT) {
			connected = true;
			a3080.reset();
			S_printf("Connected...");
		} else if (type == WS_EVT_DISCONNECT) {
			connected = false;
			S_printf("Disconnected...");
		}
	});

	server.begin();
}

void loop() {
	unsigned long now = millis();
	static unsigned long last_now = millis();

  OTA.loop(now);

	if (now - last_now > 20L && connected) {
		static byte data[ADNS3080_PIXELS_X * ADNS3080_PIXELS_Y];
		a3080.frame_capture(data);
		float voltage = analogRead(0) * 10.0 / 1024.0;

		//
		static char buffer[4000] = "";
		char tmp[10] = "";
		sprintf(buffer, "{\"battery\":%.2f,\"width\":%i,\"height\":%i,\"heap\":%i,\"image\":[%i",
			voltage, ADNS3080_PIXELS_X, ADNS3080_PIXELS_Y, ESP.getFreeHeap(), (int)data[0]);

		for (int i = 1; i < sizeof(data); i++) {
			//image.add((int)data[i]);
			sprintf(tmp, ",%i", (int)data[i]);
			strcat(buffer, tmp);
		}

		strcat(buffer, "]}");

		textThem(buffer);

		last_now = now;
	}

	hexapod.loop(now);
}
