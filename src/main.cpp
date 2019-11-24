#include <Arduino.h>
#include <Wire.h>
#include <EasyOTA.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <math.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include "wificonfig.h"

template<typename T, size_t N> void print_arr(const T * v, const char * name) {
    Serial.print("Array ");
    Serial.print(name);
    Serial.print("    {");
    for (int row = 0; row < N; row++) {
        if (row != 0) 
            Serial.print(", ");
        Serial.print(v[row], 6);
    }
    Serial.println("}");
}

#define REAL_T float
#include "kinematics/kinematics.h"
#include "kinematics/PlanarKinematics.h"
#include "drives/MultipodDrive.h"
#include "hal/ServoHAL_PCA9685.h"
#include "controllers/multipod.h"

using namespace Locomotion;

#define SCL 5
#define SDA 16

#include "body_config.h"
#include "servo_config.h"

// robot config
MultipodController<6> controller;
MultipodDrive<6, 12> hexapod(servos, controller, limbs, limb_config, working_space, default_position, default_orientation);

// server configuration

EasyOTA OTA(ARDUINO_HOSTNAME);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool connected = false;

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
	char buffer[512];
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, sizeof(buffer), format, args);
	Serial.println(buffer);
	va_end (args);
}

void setup() {
	Serial.begin(74880);
	Serial.println("Serial initialized");
	SPIFFS.begin();
	Serial.println("SPIFFS initialized");
	Wire.begin(SDA, SCL);
	Serial.println("Wire initialized");

	bool devices[255] = {false,};
	pca.discover(devices, 255, 0);
	for (size_t i = 0; i < 16; i++) {
		for (size_t j = 0; j < 16; j++) {
			Serial.print(" ");
			if (devices[j + i * 16]) {
				Serial.print(j + i * 16, HEX);
			}
			else 
				Serial.print("--");
		}
		Serial.println();
	}

	Serial.println("Initializing OTA...");
	OTA.addAP(WIFI_SSID, WIFI_PASSWORD);
	OTA.onConnect([](const String& ssid, EasyOTA::STATE state) {
		S_printf("Connected %s, state: %s", ssid.c_str(), state == EasyOTA::EOS_STA ? "Station" : "Access Point");
	});

	OTA.onMessage([](const String& msg, int line) {
		S_printf("OTA message: %s", msg.c_str());
	});
	// TODO replace with walker instead of ddrive.init();

	Serial.println("Initializing Server...");
	/*server.addHandler(&ws);
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

		} else if (type == WS_EVT_CONNECT) {
			connected = true;
			S_printf("Connected...");
		} else if (type == WS_EVT_DISCONNECT) {
			connected = false;
			S_printf("Disconnected...");
		}
	});

	server.begin(); */
	Serial.println("Initializing hexapod");
	hexapod.begin(micros(), false);
	Serial.println("Crawler ready");
}

void loop() {
	timestamp_t now = micros();
	static timestamp_t last_now = micros();
	static real_t d = 1;
	static Vector3D pos(0.0);

	OTA.loop(now / 1000L);
	hexapod.loop(now);

	if (now - last_now > 20L) {
		pos.y += d;
		if (pos.y >= hexapod.working_space.max.y || pos.y <= hexapod.working_space.min.y)
			d = -d;
		hexapod.set_position(pos);
	}

	if (now - last_now > 20L && connected) {
		float voltage = analogRead(0) * 10.0 / 1024.0;

		//
		static char buffer[4000] = "";
		char tmp[10] = "";
		sprintf(buffer, "{\"battery\":%.2f,\"heap\":%i}", voltage, ESP.getFreeHeap());

		//textThem(buffer);

		last_now = now;
	}
}
