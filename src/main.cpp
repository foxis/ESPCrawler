#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <EasyOTA.h>

EasyOTA OTA("ESPCrawler");

void setup() {
}

void loop() {
	unsigned long now = micros();
	OTA.loop(now);
}