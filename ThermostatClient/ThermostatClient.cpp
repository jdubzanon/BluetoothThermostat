#include "ThermostatClient.h"



ThermostatClient::ThermostatClient()
	:c_style_server_temp_setting(nullptr),
	c_style_server_current_temp(nullptr),
	server_temp_setting(75),
	server_current_temp(0),
	relay_activation_pin(32),
	resetPin(25),
	passthru_pin(27),
	turnon_button_light_pin(4),
	connected(false),
	new_current_temp_status(false),
	new_temp_setting_status(false),
	doConnect(false){};
	
int ThermostatClient::get_server_temp_setting() const{
	return server_temp_setting;
}



void ThermostatClient::convert_then_set_server_temp_setting(uint8_t* data,size_t len){
	
	c_style_server_temp_setting = (char*)data;
	server_temp_setting = String(c_style_server_temp_setting).toInt();
	// for some reason when information is sent its being recieved as (server * 10.X) and i cant figure out why so im just manually adjusting number 
	//This is a hack but i dont know whats going on here
	if(server_temp_setting > 80)
	{
		server_temp_setting = server_temp_setting / 10;
	}
}

void ThermostatClient::convert_then_set_server_current_temp_reading(uint8_t* data){
	c_style_server_current_temp = (char*)data;
	server_current_temp = String(c_style_server_current_temp).toFloat();
}

float ThermostatClient::get_server_current_temp() const{
	return server_current_temp;
}

int ThermostatClient::get_relay_activation_pin() const{
	return relay_activation_pin;
}

int ThermostatClient::get_resetPin() const {
	return resetPin;
}

void ThermostatClient::toggle_activation_pin(){
	if (digitalRead(relay_activation_pin) == LOW)
	{
		if(server_current_temp < server_temp_setting)
			digitalWrite(relay_activation_pin,HIGH);
	}
	
	if (digitalRead(relay_activation_pin) == HIGH)
	{
		if( server_current_temp > (server_temp_setting + 2)  )
			digitalWrite(relay_activation_pin,LOW);
	}
}


void  ThermostatClient::set_connection_status(bool set){
	connected = set;
}

bool ThermostatClient::get_connection_status() const {
	return connected;
}


void ThermostatClient::set_new_current_temperature_status(bool set){
	new_current_temp_status = set;
}

bool ThermostatClient::get_new_current_temperatrue_status() const {
	return new_current_temp_status;
}

void ThermostatClient::set_new_temp_setting_status(bool set){
	new_temp_setting_status = set;
}

bool ThermostatClient::get_new_temp_setting_status() const {
	return new_temp_setting_status;
}


void ThermostatClient::set_doConnect(bool set){
	doConnect = set;
}

bool ThermostatClient::get_doConnect() const {
	return doConnect;
}

void ThermostatClient::update_display(Adafruit_SSD1306* display){
	
	
	if(connected)
	{	
		display->clearDisplay();
		display->setTextColor(SSD1306_WHITE);
		display->setTextSize(1);

		
		display->setCursor(15,0);
		display->print("Room Temp: ");
		
		display->setCursor(80,0);
		display->print(String(server_current_temp).c_str());

		display->setCursor(0,20);
		display->print("Temp setting: ");
		
		display->setCursor(90,20);
		display->print(String(server_temp_setting).c_str());
		display->display();
		
	}
	
	if(!connected)
	{
		display->clearDisplay();
		delay(250);
		display->setCursor(0,0);
		display->setTextSize(1);
		display->print("Disconnected!!");
		display->setTextSize(1);
		display->setCursor(10,15);
		display->print("RESET DEVICE");
		display->display();
		
		
	}

}

void ThermostatClient::startup_display(Adafruit_SSD1306* display){
		display->clearDisplay();
		display->setTextColor(SSD1306_WHITE);
		display->setTextSize(1);
		display->setCursor(0,0);
		display->print("Trying to ");
		display->setCursor(0,15);
		display->print("connect....");
		display->display();
		
}

void ThermostatClient::connetion_issue_display(Adafruit_SSD1306* display){
		display->clearDisplay();
		display->setTextColor(SSD1306_WHITE);
		display->setTextSize(1);
		display->setCursor(0,0);
		display->print("CONNECTION");
		display->setCursor(0,15);
		display->print("ISSUES");
		display->display();
}


void ThermostatClient::passthru_display(Adafruit_SSD1306 *display){
		display->clearDisplay();
		display->setTextColor(SSD1306_WHITE);
		display->setTextSize(1);
		display->setCursor(0,0);
		display->print("PASS THROUGH");
		display->setCursor(0,15);
		display->print("MODE");
		display->display();
}


void ThermostatClient::set_passthru(bool set){
	passthru = set;
}

bool ThermostatClient::get_passthru() const{
	return passthru;
}

int ThermostatClient::get_passthru_pin() const {
	return passthru_pin;
}


int ThermostatClient::get_turnon_button_light_pin() const {
	return turnon_button_light_pin;
}






