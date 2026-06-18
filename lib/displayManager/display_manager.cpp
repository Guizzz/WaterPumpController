#include "display_manager.h"

DisplayManager::DisplayManager(unsigned long timeout_ms)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
      _timeout(timeout_ms),
      _last_activity(0),
      _display_on(true)
{
}

void DisplayManager::fast_write(String msg)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(msg);
  display.display();
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
}

void DisplayManager::display_info(Info status)
{
  display.clearDisplay();

  if (current_page == 0)
    page_1(status);
  else if (current_page == 1)
    page_2(status);
  else if (current_page == 2)
    page_3(status);
  display.display(); 
}

void DisplayManager::page_1(Info status)
{
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Monitor ambient ");
  display.print(status.relay?"ON":"OFF");

  display.setCursor(0, 16);
  display.print("Temp: ");
  display.setTextSize(2);
  display.print(status.temp, 1);
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
  display.print(status.humidity, 1);
  display.print(" %");
}

void DisplayManager::page_2(Info status)
{
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(status.hours);
  display.print(":");
  display.print(status.minutes);
  display.print(":");
  display.print(status.seconds);

  display.setCursor(0, 16);
  display.print("Active timer:   ");
  display.setTextSize(2);
  display.print(status.active_timer);

  display.setCursor(0, 42);
  display.setTextSize(1);
  display.print("Active routine: ");
  display.setTextSize(2);
  display.print(status.active_routine);
}

void DisplayManager::page_3(Info status)
{
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(status.hours);
  display.print(":");
  display.print(status.minutes);
  display.print(":");
  display.print(status.seconds);

  display.setCursor(0, 16);
  display.print("SSID:");
  display.setCursor(0, 25);
  display.print(status.ssid);

  display.setCursor(0, 42);
  display.print("IP: ");
  display.setCursor(0, 51);
  display.print(status.ip);
}

int DisplayManager::get_pages_number()
{
  return pages;
}

void DisplayManager::clear()
{
    display.clearDisplay();
    display.display();
}

void DisplayManager::activity()
{
    _last_activity = millis();
    if (!_display_on)
    {
        _display_on = true;
        display.ssd1306_command(0xAF);
        display.display();
    }
    current_page = (current_page + 1) % pages;
}

void DisplayManager::update()
{
    if (!_display_on) return;
    if (_timeout > 0 && millis() - _last_activity >= _timeout)
    {
        _display_on = false;
        current_page = -1;
        display.ssd1306_command(0xAE);
        clear();
    }
}

bool DisplayManager::is_on()
{
    return _display_on;
}
