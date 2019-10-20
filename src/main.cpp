#include <Arduino.h>
#include <Wire.h>
#include <EasyOTA.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <math.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"

#include "wificonfig.h"

#include "math_utils.h"
#include "kinematics/PlanarKinematics.h"
#include "HexapodDrive.h"

#define SCL 5
#define SDA 6

// Robot configuration
typedef Locomotion::_PlanarJoint_t<Locomotion::real_t> JointConfig_t;
typedef Locomotion::_PlanarKinematics<Locomotion::real_t, 2> Kinematics;
typedef Locomotion::_Limb<Locomotion::real_t, Kinematics, 2> Limb;
typedef Locomotion::_HexapodDrive<Locomotion::real_t, Kinematics, 2> Drive;
typedef Locomotion::_Vector3D<Locomotion::real_t> Vector;
typedef Locomotion::_ConstraintVolume<Locomotion::real_t> Constraint;

JointConfig_t joints[] = {
	{
		.constraints = {
			.min = -1, // TODO
			.max = 1
		}, 
		.length = 10 // TODO
	},
	{
		{-.5, 1}, 
		75
	},
};
Constraint leg_working_volume(Constraint(Vector(45, -20, -40), Vector(65, 20, 40)));
Kinematics kinematics(joints, leg_working_volume);

Limb limbs[] = {
	Limb(kinematics, Vector(30, 0, 110), Vector(0, 0, 0)),
	Limb(kinematics, Vector(30, 0, 0), Vector(0, 0, 0)),
	Limb(kinematics, Vector(30, 0, -90), Vector(0, 0, 0)),
	Limb(kinematics, Vector(-30, 0, 110), Vector(M_PI, 0, 0)),
	Limb(kinematics, Vector(-30, 0, 0), Vector(M_PI, 0, 0)),
	Limb(kinematics, Vector(-30, 0, -90), Vector(M_PI, 0, 0)),
};


// server configuration

EasyOTA OTA(ARDUINO_HOSTNAME);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

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
	char buffer[512];
	va_list args;
	va_start (args, format);
	vsnprintf (buffer, sizeof(buffer), format, args);
	Serial.println(buffer);
	va_end (args);
}

void setup() {
	Locomotion::real_t arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	Locomotion::Vector2D a(0.0);
	Locomotion::Vector2D v(1, 2);
	Locomotion::Vector2D b(1, 1);
	Locomotion::Vector2D c(1, 1);
	Locomotion::Vector2D e(a);
	Locomotion::Vector2D f(arr);
	Locomotion::_VectorStatic<Locomotion::real_t, 2> cc;
	Locomotion::Quaternion q(1, 2, 3, 1);
	c = b;
	S_printf("dot: %f", v.dot(b + cc));
	Locomotion::Matrix3x3 m3x3a;
	Locomotion::Matrix3x3 m3x3b(0.0);
	Locomotion::Matrix3x3 m3x3c(arr);
	Locomotion::Matrix3x3 m3x3d(m3x3a);
	Locomotion::Matrix3x3 m3x3e(0,1,2,3,4,5,6,7,8);
	S_printf("det: %f", m3x3c.det());

	Locomotion::PlanarJoint_t joints[] = {
		{
			.constraints = {
				.min = -1, 
				.max = 1
			}, 
			.length = 10
		},
		{{-.5, 1}, 75},
	};
	Locomotion::PlanarKinematics<Locomotion::real_t, 2> limb(joints);

	OTA.addAP(WIFI_SSID, WIFI_PASSWORD);
	OTA.onConnect([](const String& ssid, EasyOTA::STATE state) {
		S_printf("Connected %s, state: %s", ssid.c_str(), state == EasyOTA::EOS_STA ? "Station" : "Access Point");
	});

	OTA.onMessage([](const String& msg, int line) {
		S_printf("OTA message: %s", msg.c_str());
	});
	// TODO replace with walker instead of ddrive.init();
	SPIFFS.begin();
	Wire.begin(SDA, SCL);

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

		} else if (type == WS_EVT_CONNECT) {
			connected = true;
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
		float voltage = analogRead(0) * 10.0 / 1024.0;

		//
		static char buffer[4000] = "";
		char tmp[10] = "";
		sprintf(buffer, "{\"battery\":%.2f,\"heap\":%i}",
			voltage, ESP.getFreeHeap());

		textThem(buffer);

		last_now = now;
	}
}
