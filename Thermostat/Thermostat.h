#ifndef _THERMOSTAT_H
#define _THERMOSTAT_H
#include "FS.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>

class Thermostat{
	private:
	const char* temperature_settings_file = "/temp_settings.txt";
	const int max_temp;
	const int min_temp;
	const int default_setting;
	int setting;
	bool start_adjustment;
	char* settings_ptr;
	int pinPlus;
	int pinMinus;
	int resetPin;
	float current_temp;
	bool thermostat_connected;
	unsigned long screen_shutoff_timer_start;
	bool screen_on;
	const unsigned long screen_timeout_duration;
	Adafruit_SSD1306 *display;
	
	
	public:
	Thermostat(Adafruit_SSD1306 *disp);
	const char* get_temp_filepath() const;
	int get_pinMinus() const;
	int get_pinPlus() const;
	int get_resetPin() const;
	int get_max_temp() const;
	int get_min_temp() const;
	int get_default_setting() const;
	int get_setting() const;
	float get_current_temp() const;
	void set_current_temp(float& new_temp);
	void set_setting(String setting_str);
	bool get_start_adjustment() const;
	void set_start_adjustment(bool set_var);
	void set_settings_ptr(int setting);
	
	bool get_thermostat_connection_status() const;
	void set_thermostat_connection_status(bool set);	
		
	void adjust_setting(Adafruit_BME280& sensor,int setting, bool deviceConnected);
	void writeFile(fs::FS &fs);
	void write_default_setting_to_file(fs::FS &fs);
	void update_display(Adafruit_BME280& sensor,int setting, bool deviceConnected);
	String readSettingFile(fs::FS &fs);
	
	//SCREEN SHUT OFF TIME
	void set_screen_shutoff_timer_start(unsigned long current_time);	
	unsigned long get_screen_shutoff_timer_start() const;
	void set_screen_on(bool set);
	bool get_screen_on() const;
	unsigned long get_screen_timeout_duration() const;
	void disconnected_display();
	~Thermostat();
	
};


#endif
