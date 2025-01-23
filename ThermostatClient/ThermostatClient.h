#ifndef _THERMOSTATCLIENT_H
#define _THERMOSTATCLIENT_H



#include <stdint.h>
#include <Arduino.h>
#include <cstring>
#include "Adafruit_SSD1306.h"
class ThermostatClient{
	private:
		char* c_style_server_temp_setting; // set in notifyCallBack function this needs to evolve into an int
		int server_temp_setting;
		
		char* server_sleep_status; // set in notifyCallBack function will be eitier "on" or "off"
		
		char* c_style_server_current_temp; //set in notifyCallBack function this needs to evolve into a float after
		float server_current_temp;		
		
		const int relay_activation_pin; 
		const int resetPin;
		const int passthru_pin;
		const int turnon_button_light_pin;
		
		
		bool connected;
		bool doConnect;
		bool new_current_temp_status;
		bool new_temp_setting_status;
		bool passthru;
		

	public:
		ThermostatClient();
		int get_server_temp_setting() const;
		float get_server_current_temp() const;
		
		void convert_then_set_server_temp_setting(uint8_t* data,size_t len);
		void convert_then_set_server_current_temp_reading(uint8_t* data);
		
		
		int get_resetPin() const;
		int get_relay_activation_pin() const;
		void toggle_activation_pin();// still need to define
		
		void set_connection_status(bool set);
		bool get_connection_status() const;
		
		void set_new_current_temperature_status(bool set);
		bool get_new_current_temperatrue_status() const;
		
		void set_new_temp_setting_status(bool set);
		bool get_new_temp_setting_status() const;
		
		
		void set_doConnect(bool set);
		bool get_doConnect() const; 
		
		void update_display(Adafruit_SSD1306 *display);
		void startup_display(Adafruit_SSD1306 *display);
		void connetion_issue_display(Adafruit_SSD1306 *display);
		void passthru_display(Adafruit_SSD1306 *display);
		
		int get_passthru_pin() const;
		void set_passthru(bool set);
		bool get_passthru() const;
		
		int get_turnon_button_light_pin() const;

		
		
};	



#endif











