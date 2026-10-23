#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

class OledDisplay {
private:
  Adafruit_SSD1306 display;

public:
  OledDisplay() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

  bool begin() {
    return display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  }

  void showBoot(const String& line1, const String& line2) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ELECTROLINK");
    display.setCursor(0, 18);
    display.println(line1);
    display.setCursor(0, 32);
    display.println(line2);
    display.display();
  }

  void render(float current, float power, bool relayOn, const String& status, float nominalVoltage) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("ELECTROLINK IoT");

    display.setCursor(0, 14);
    display.print("I: ");
    display.print(current, 2);
    display.println(" A");

    display.setCursor(0, 26);
    display.print("P: ");
    display.print(power, 2);
    display.println(" W");

    display.setCursor(0, 38);
    display.print("Relay: ");
    display.println(relayOn ? "ON" : "OFF");

    display.setCursor(0, 50);
    display.print("Estado: ");
    display.println(status);

    display.setCursor(0, 56);
    display.print("Vnom: ");
    display.print(nominalVoltage, 0);
    display.println("V");

    display.display();
  }
};
