#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels



class DisplayManager
{
private:
    Adafruit_SSD1306 display;
public:
    DisplayManager();
    void init_display();
    void clear();
    void display_info(float temp, float humidity, bool relay);
};

