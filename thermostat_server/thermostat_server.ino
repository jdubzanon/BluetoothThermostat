// temp sensor logic
//sleep logic
//bluetooth notification logic

#include "FS.h"
#include "SPIFFS.h"
#include "Thermostat.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Adafruit_BME280.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
// #include <BLE2902.h>
// #include "esp_sleep.h"
// #include "esp_bt.h"


#define FORMAT_SPIFFS_IF_FAILED true
#define screenAddress 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define bme_address  0x76
#define bleServerName "THERMOSTAT_ESP32_SERVER"
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

// initalize with constructor
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Thermostat thermostat(&display);

Adafruit_BME280 bme;
//BLUETOOTH TEST
BLEAdvertising *pAdvertising {nullptr};
BLEServer *pServer {nullptr};
BLEService* bmeService {nullptr};
//BLUETOOTH TEST END
int pinplus; 
int pinminus;
int resetpin; 
bool startup {true};
bool DEBUG {true};
void print_function(int start, int end, const char* msg){
   while (start < end){
    Serial.println(F(msg));
    delay(1000);
    start++;
  }
}

/////////////// BLUETOOTH STUFF BEGIN  ////////////////
BLECharacteristic bmeTemperatureFahrenheitCharacteristics("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor bmeTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2902));


BLECharacteristic tempSettingCharacteristic("99151990-e60a-462d-955c-c2fd1caec0a4",BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor tempSettingDescriptor(BLEUUID((uint16_t)0x2904));



void temp_notification(float room_temp){
  	static char temperatureF[6];
  	float converted_temp = (1.8 * room_temp) + 32;
  	dtostrf(converted_temp,6,2,temperatureF);
  	bmeTemperatureFahrenheitCharacteristics.setValue(temperatureF);
  	bmeTemperatureFahrenheitCharacteristics.notify();
}




//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    thermostat.set_thermostat_connection_status(true);
    delay(100);
    if(DEBUG)
    {
      Serial.println("NOW CONNECTED");
      Serial.print("from onConnect ");
      Serial.println(thermostat.get_thermostat_connection_status());
    }
    if (!thermostat.get_screen_on())
    {
      display.ssd1306_command(SSD1306_DISPLAYON);
      thermostat.set_screen_on(true);
      delay(500); 
    }
    thermostat.update_display(bme,thermostat.get_setting(), thermostat.get_thermostat_connection_status());
    thermostat.set_screen_shutoff_timer_start(millis());
  };

  void onDisconnect(BLEServer* pServer) {
    thermostat.set_thermostat_connection_status(false);
    if(DEBUG)
    {
      Serial.println("NOW DISCONNECTED");
    }
    thermostat.disconnected_display();
  }
};
MyServerCallbacks serverCallbacks;
/////////////// BLUETOOTH STUFF END  ////////////////

void BluetoothAdvertise(){
        BLEDevice::init(bleServerName);
        pServer = BLEDevice::createServer();
        BLEDevice::setPower(ESP_PWR_LVL_N12);

        pServer->setCallbacks(&serverCallbacks);
        bmeService = pServer->createService(SERVICE_UUID) ;
        
        bmeService->addCharacteristic(&bmeTemperatureFahrenheitCharacteristics);
        bmeTemperatureFahrenheitDescriptor.setValue("BME temperature fahrenheit");
        bmeTemperatureFahrenheitCharacteristics.addDescriptor(&bmeTemperatureFahrenheitDescriptor);
        
        bmeService->addCharacteristic(&tempSettingCharacteristic);
        tempSettingDescriptor.setValue("Server temp setting");
        tempSettingCharacteristic.addDescriptor(&tempSettingDescriptor);
        
        
        bmeService->start();
        
        
        if (pAdvertising == nullptr)
        {
          pAdvertising = BLEDevice::getAdvertising();
          pAdvertising->setMinInterval(0xF00);
          pAdvertising->setMaxInterval(0xF00);


        }
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pServer->getAdvertising()->start();
}


void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  if(DEBUG)
  {
    print_function(0,5,"first print for testing");
  }
  
  if (!display.begin(screenAddress, 0x3C)) {
    if(DEBUG)
    {
      print_function(0,2,"SSD1306 allocation failed");
    }
  } else {
    if(DEBUG)
    {
      print_function(0,2,"all good here captain");
    }
  } 


    // temp sensor begin
  unsigned status;
  status = bme.begin(bme_address);

  if (!status){
    if(DEBUG)
    {
      print_function(0,2,"bme_not working");
    }
  } else {
    if(DEBUG)
    {
      print_function(0,2,"bme has begun!");
    }
  }

  pinplus = thermostat.get_pinPlus();
  pinminus =  thermostat.get_pinMinus();
  resetpin = thermostat.get_resetPin();
  pinMode(pinplus,INPUT_PULLUP);
  pinMode(pinminus,INPUT_PULLUP);
  pinMode(resetpin,INPUT_PULLUP);

  
  if (!(SPIFFS.exists(thermostat.get_temp_filepath()))) 
  {
    thermostat.write_default_setting_to_file(SPIFFS);
  }

  String temp_setting = thermostat.readSettingFile(SPIFFS);
  thermostat.set_setting(temp_setting);
  if(DEBUG)
  {
    Serial.print("Thermostat setting on setup:  ");
    Serial.println(thermostat.get_setting());
  }
  
  display.display();
  delay(2000);
  display.clearDisplay();
  thermostat.update_display(bme,thermostat.get_setting(),false);
  
  //set thermostat current temp on startup
  float current_temp = bme.readTemperature();
  thermostat.set_current_temp(current_temp);
  
  
  //BLUETOOTH STUFF
  BluetoothAdvertise();
  delay(250);
  while(!thermostat.get_thermostat_connection_status());
  /////////////END BLUETOOTH SETUP STUFF///////////

  thermostat.set_screen_shutoff_timer_start(millis());
 
}

void loop() {
  //NOTIFY ON STARTUP TO SET CLIENT TEMP
  if (startup)
  {
    delay(5000);
    tempSettingCharacteristic.setValue(String(thermostat.get_setting()).c_str());
    tempSettingCharacteristic.notify();
  	
    float current_temp = bme.readTemperature();
    //this handles current temp float value it converts and notifies
    temp_notification(current_temp);
    startup = false;
  }
  //ADJUST TEMP SETTING
  if (digitalRead(pinplus) == LOW || digitalRead(pinminus) == LOW)
  {
    if(!thermostat.get_screen_on())
    {
      display.ssd1306_command(SSD1306_DISPLAYON); 
      thermostat.update_display(bme,thermostat.get_setting(),thermostat.get_thermostat_connection_status());
      thermostat.set_screen_on(true);
      thermostat.set_screen_shutoff_timer_start(millis());
      delay(500);
    }
    thermostat.set_start_adjustment(true);
    if (thermostat.get_start_adjustment())
    {
      int temp_setting_value = thermostat.get_setting();
      thermostat.adjust_setting(bme,thermostat.get_setting(),thermostat.get_thermostat_connection_status());
      thermostat.set_settings_ptr(thermostat.get_setting());
      
      // only if setting was changed perform operation because buttons are also used to wake up screen
      if(thermostat.get_setting() != temp_setting_value)
      {
        thermostat.writeFile(SPIFFS);
        delay(500);
        tempSettingCharacteristic.setValue(String(thermostat.get_setting()));
        tempSettingCharacteristic.notify();
        delay(250);
        thermostat.set_start_adjustment(false);
      }
    }
  }
  
  //IF TEMPERATURE CHANGES BY ONE THEN UPDATE SCREEN AND CURRENT_TEMP
  float room_temp = bme.readTemperature();
  //room temperature moved up or down by 1 degree
  if( thermostat.get_current_temp() - room_temp >= 1 || room_temp - thermostat.get_current_temp() >= 1)
  {
    if(!thermostat.get_screen_on())
    {
      display.ssd1306_command(SSD1306_DISPLAYON); 
    }
    thermostat.update_display(bme,thermostat.get_setting(),thermostat.get_thermostat_connection_status());
    thermostat.set_current_temp(room_temp);
    
    //update bluetooth characteristic value and broadcast
    if(thermostat.get_thermostat_connection_status())
    {
      temp_notification(room_temp);
    }  
    
    delay(1000);
  }
  if(thermostat.get_thermostat_connection_status())
  {
    if(millis() - thermostat.get_screen_shutoff_timer_start() >= thermostat.get_screen_timeout_duration())
    {
      display.ssd1306_command(SSD1306_DISPLAYOFF); 
      thermostat.set_screen_on(false);
    }
  }

  if (digitalRead(resetpin) == LOW){
    esp_restart();
  }

}

