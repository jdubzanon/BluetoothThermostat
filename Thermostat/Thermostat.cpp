#include "Thermostat.h"
#include "SPIFFS.h"
#include <Arduino.h>

Thermostat::Thermostat(Adafruit_SSD1306 *disp)
	:max_temp(80), 
	min_temp(60),
	setting(75),
	default_setting(75),
	start_adjustment(false),
	settings_ptr(nullptr),
	pinPlus(33),
	pinMinus(32),
	resetPin(25),
	thermostat_connected(false),
	screen_shutoff_timer_start(0),
	screen_on(true),
	screen_timeout_duration(30000),
	display(disp){};
	


Thermostat::~Thermostat(){
	delete [] settings_ptr;
	settings_ptr = nullptr;
}
	
const char* Thermostat::get_temp_filepath() const {
	return temperature_settings_file;
}


int Thermostat::get_pinPlus() const{
	return pinPlus;
}

int Thermostat::get_pinMinus() const{
	return pinMinus;
}

int Thermostat::get_resetPin() const{
	return resetPin;
}

int Thermostat::get_max_temp() const {
	return max_temp;
}

int Thermostat::get_min_temp() const {
	return min_temp;
}

int Thermostat::get_setting() const {
	return setting;
}

void Thermostat::set_setting(String setting_str){
	this->setting=setting_str.toInt();
}


int Thermostat::get_default_setting() const {
	return default_setting;
}

bool Thermostat::get_start_adjustment() const {
	return start_adjustment;
}

void Thermostat::set_start_adjustment(bool set_var){
	start_adjustment = set_var;
}

float Thermostat::get_current_temp() const{
	return current_temp;
}

void Thermostat::set_current_temp(float& new_temp){
	current_temp = new_temp;
}

bool Thermostat::get_thermostat_connection_status() const{
	return thermostat_connected;
}

void Thermostat::set_thermostat_connection_status(bool set){
	thermostat_connected = set;
}




void Thermostat::set_settings_ptr(int setting) {
    // Free old memory if necessary
    if (this->settings_ptr != nullptr) {
        delete[] this->settings_ptr;
        this->settings_ptr = nullptr;
    }

    // Allocate new memory for settings_ptr
    this->settings_ptr = new char[4]; 

    // Convert int to string and copy to settings_ptr
    String temp = String(setting);
    strcpy(this->settings_ptr, temp.c_str());
    Serial.printf("Settings ptr: %s\n", this->settings_ptr);
}






//need to finish this to update display;
void Thermostat::adjust_setting(Adafruit_BME280& sensor,int setting, bool deviceConnected){
	unsigned long startMillis = millis();
	const unsigned long period = 15000;
	while ((millis() - startMillis) <= period)
	{
		if (digitalRead(this->pinPlus) == LOW)
		{
			(this->setting >= this->max_temp) ? (this->setting = this->max_temp) : this->setting++;
        	delay(250);
        	Serial.println(this->setting);
        	update_display(sensor,this->setting,deviceConnected);
		}
	    if (digitalRead(this->pinMinus) == LOW)
    	{
		    (this->setting <= this->min_temp) ? (this->setting = this->min_temp) : this->setting--;
		    delay(250);
		    Serial.println(this->setting);
		    update_display(sensor,this->setting,deviceConnected);
    	}
	}
}



void Thermostat::writeFile(fs::FS &fs) {
    Serial.printf("Writing file: %s\r\n", this->temperature_settings_file);

    // Ensure settings_ptr is valid
    if (this->settings_ptr == nullptr) {
        Serial.println("- settings_ptr is null, nothing to write");
        return;
    }

    // Open the file for writing
    File file = fs.open(this->temperature_settings_file, FILE_WRITE);
    if (!file) {
        Serial.println("- failed to open file for writing");
        return;
    }

    // Write settings_ptr to the file
    if (file.print(this->settings_ptr)) {
        Serial.println("- file written successfully");
    } else {
        Serial.println("- write failed");
    }

    // Close the file
    file.close();
}



void Thermostat::write_default_setting_to_file(fs::FS &fs) { //pass in SPIFFS 
  Serial.printf("Writing file: %s\r\n", this->temperature_settings_file);

  File file = fs.open(this->temperature_settings_file, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  
  
  
  if (file.print(String(default_setting).c_str())) 
  {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}






String Thermostat::readSettingFile(fs::FS &fs) {
  String temp = "";
  File file = fs.open(this->temperature_settings_file);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    Serial.println("- Pausing all operations");
    while (true);
  }

  Serial.println("- read from file:");
  while (file.available()) {
    char c = file.read();
    Serial.write(c);
    temp += c;
    
  }
  file.close();
  return temp;
}

void Thermostat::update_display(Adafruit_BME280& sensor,int setting,bool deviceConnected){
	float tempF = (sensor.readTemperature() * 9/5) + 32;
	display->clearDisplay();  
  // display temperature
  display->setTextColor(SSD1306_WHITE);
  display->setTextSize(1);
  display->setCursor(0,0);
  display->print("Temp: ");
  display->setTextSize(2);
  display->setCursor(35,0);
  display->print(tempF);
  display->setTextSize(1);
  display->cp437(true);
  display->write(167);
  display->setTextSize(1);
  display->setCursor(115,0);
  display->print("F");

  display->setTextSize(1);

  //display setPoint 
  display->setTextSize(1);
  display->setCursor(0,25);
  display->print("temp set: ");
  display->setTextSize(1);
  display->setCursor(60,25);
  display->print(setting);
  display->setCursor(100,20);
  display->setTextSize(1);
  if(!deviceConnected)
  {
    display->print("N.C.");

  } else{
    display->print("BT.C.");
  } 

  display->display();

}


//SCREEN SHUT OFF OPERATIONS
void Thermostat::set_screen_shutoff_timer_start(unsigned long current_time){
	screen_shutoff_timer_start = current_time;
}

unsigned long Thermostat::get_screen_shutoff_timer_start() const {
	return screen_shutoff_timer_start;
}

void Thermostat::set_screen_on(bool set){
	screen_on = set;
}

bool Thermostat::get_screen_on() const {
	return screen_on;
}

unsigned long Thermostat::get_screen_timeout_duration() const {
	return screen_timeout_duration;
}

void Thermostat::disconnected_display(){
	if((!screen_on)){
		display->ssd1306_command(SSD1306_DISPLAYON);
		screen_on = true;
	}
	display->clearDisplay();
	display->setTextColor(SSD1306_WHITE);
  	display->setTextSize(1);
  	display->setCursor(5,0);
  	display->print("Disconnected!!");
  	display->setCursor(15,10);
  	display->print("RESET BOTH");
  	display->setCursor(15,20);
  	display->print("DEVICES");
  	display->display();
}














