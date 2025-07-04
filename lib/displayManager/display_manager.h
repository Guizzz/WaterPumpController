#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

struct Info
{
    float temp;
    float humidity;
    bool relay;

    int active_timer;
    int active_routine;
    
    int seconds;
    int minutes;
    int hours;
};

class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    void page_1(Info status);
    void page_2(Info status);
public:
    DisplayManager();
    void init_display();
    void clear();
    void display_info(Info status, int page=1);
};

