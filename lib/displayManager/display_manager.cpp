#include "display_manager.h"

DisplayManager::DisplayManager(/* args */)
{
    // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
    display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
}

void DisplayManager::init_display()
{
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

  display.clearDisplay();
  display.setTextColor(WHITE);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Starting...");
  display.display();
}

void DisplayManager::display_info(float temp, float humidity, bool relay)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Monitor ambient ");
  display.print(relay?"ON":"OFF");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.setTextSize(2);
  display.print(temp, 1);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  display.setCursor(0, 42);
  display.setTextSize(1);
  display.print("Hum:  ");
  display.setTextSize(2);
  display.print(humidity, 1);
  display.print(" %");

  display.display(); 
}

void DisplayManager::clear()
{
    display.clearDisplay();
    display.display();
}