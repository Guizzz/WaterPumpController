#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

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

    String ssid;
    String ip;
};

class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    void page_1(Info status);
    void page_2(Info status);
    void page_3(Info status);
    int pages = 3;

    unsigned long _timeout;
    unsigned long _last_activity;
    bool _display_on;
    short current_page = -1;

public:
    DisplayManager(unsigned long timeout_ms = 0);
    void fast_write(String msg);
    void init_display();
    int get_pages_number();
    void clear();
    void display_info(Info status);
    void activity();
    void update();
    bool is_on();
};
