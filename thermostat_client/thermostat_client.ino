#include "ThermostatClient.h"
#include "BLEDevice.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

#define bleServerName "THERMOSTAT_ESP32_SERVER"
#define screenAdress 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

bool DEBUG {true};

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,-1);
ThermostatClient thermostatC;
int passthru_pin; 
int turned_on_lightpin;



static BLEUUID thermostatServiceUUID("91bad492-b950-4226-aa2b-4ede9fa42f59");
static BLEUUID temperatureCharacteristicUUID("f78ebbff-c8b7-4107-93de-889a6a06d408");
static BLEUUID tempSettingCharacteristicUUID("99151990-e60a-462d-955c-c2fd1caec0a4");


static BLEAdvertisedDevice *myDevice {nullptr};

BLEScan* pBLEScan {nullptr};

//Characteristics that we want to read
static BLERemoteCharacteristic* temperatureCharacteristic {nullptr};
static BLERemoteCharacteristic* tempSettingCharacteristic {nullptr};



//Callback function that gets called, when another device's advertisement has been received
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
  if (DEBUG)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
  }
    if (
      advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(thermostatServiceUUID)
    ) 
    { 
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      if (myDevice == nullptr)
      {
      	myDevice = new BLEAdvertisedDevice(advertisedDevice);
      } else{
      	*myDevice = advertisedDevice;
      }
      //Set indicator, stating that we are ready to connect
      thermostatC.set_doConnect(true);
      if(DEBUG)
      {
        Serial.print("Device found. Connecting! ");
        Serial.println(advertisedDevice.toString().c_str());
      }
    }
  }
};

// INSTANTIATE ADVERTISE CALLBACKS
MyAdvertisedDeviceCallbacks AdverstisingCallbacks;



void GetScanner(){
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(&AdverstisingCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(60,true);
}


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {
    thermostatC.set_connection_status(true);
    
  }

  void onDisconnect(BLEClient *pclient) {
    thermostatC.set_connection_status(false);
    thermostatC.set_doConnect(false);
    if(digitalRead(thermostatC.get_relay_activation_pin()) == HIGH)
    {
      digitalWrite(thermostatC.get_relay_activation_pin(),LOW);
    }
    thermostatC.update_display(&display);

  }
};


//INSTANTIATE MYCLIENTCALLBACKS
MyClientCallback clientcallbacks;

bool connectToServer() {
   BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(&clientcallbacks);
  // Connect to the remove BLE Server.
  pClient->connect(myDevice);
  if(DEBUG)
  {
    Serial.println(" - Connected to server");
  }
  
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(thermostatServiceUUID);
  if (pRemoteService == nullptr) {
    if(DEBUG)
    {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(thermostatServiceUUID.toString().c_str());
    }
    thermostatC.connetion_issue_display(&display);
    return (false);
  }
 
  // Obtain a reference to the characteristics in the service of the remote BLE server.
    temperatureCharacteristic = pRemoteService->getCharacteristic(temperatureCharacteristicUUID);
    tempSettingCharacteristic = pRemoteService->getCharacteristic(tempSettingCharacteristicUUID);
  
  
  if (temperatureCharacteristic == nullptr || tempSettingCharacteristic == nullptr) {
    if(DEBUG)
    {
      Serial.println("nullptr issues with characteristics");
      Serial.print("Failed to find our characteristic UUID");
    }
    thermostatC.connetion_issue_display(&display);
    return false;
  }
 if(DEBUG)
 {
  Serial.println(" - Found our characteristics");
 }

  if(temperatureCharacteristic->canNotify() && tempSettingCharacteristic->canNotify())
  {
    temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
    tempSettingCharacteristic->registerForNotify(tempSettingNotifyCallback);
  } else {
      if(DEBUG)
      {
        Serial.println("cant register temp characteristic");
      }
      thermostatC.connetion_issue_display(&display);
  }
  
  return true;
}


//ALL NOTIFICATION CALLBACKS
//When the BLE Server sends a new temperature reading with the notify property
static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  thermostatC.convert_then_set_server_current_temp_reading(pData);
  thermostatC.set_new_current_temperature_status(true);
}

//When the BLE Server sends a new humidity reading with the notify property
static void tempSettingNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  thermostatC.convert_then_set_server_temp_setting(pData,length);
  thermostatC.set_new_temp_setting_status(true);
}


void setup(){
  
  turned_on_lightpin = thermostatC.get_turnon_button_light_pin();
  pinMode(turned_on_lightpin,OUTPUT);
  digitalWrite(turned_on_lightpin,HIGH);
  
  if(DEBUG){
    Serial.begin(115200);
    delay(2000);
    Serial.println("Starting Arduino BLE Client application...");
    Serial.println(thermostatC.get_doConnect());
  }

  passthru_pin = thermostatC.get_passthru_pin();

  pinMode(thermostatC.get_resetPin(),INPUT_PULLUP);
  pinMode(thermostatC.get_relay_activation_pin(),OUTPUT);
  pinMode(passthru_pin,INPUT_PULLUP);
  
  thermostatC.set_passthru(digitalRead(passthru_pin) == LOW);

  if(!display.begin(screenAdress,0x3C))
  {
    Serial.println("DISPLAY ISSUES");
  } else{
    Serial.println("DISPLAY WORKING");
  }
  delay(500);
  

  //TRYING TO CONNECT PROMPT
  thermostatC.startup_display(&display);
  
  BLEDevice::init("");
  GetScanner();
  

}


void loop(){
      
      //TRY TO CONNECT REGARDLESS IF IN PASS THROUGH MODE OR NOT
  
  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (thermostatC.get_doConnect()) 
  {
    if (connectToServer()) 
    {
      if(DEBUG)
      {
        Serial.println("We are now connected to the BLE Server.");
      }
      thermostatC.set_connection_status(true);
      thermostatC.set_doConnect(false);
    } else {
      thermostatC.set_connection_status(false);
      thermostatC.set_doConnect(true);
      if(DEBUG)
      {
        Serial.println("Connection was lost!!");
        delay(1000);
      }
    }
  }

  

  if(!thermostatC.get_doConnect())
  {
    //NON PASSTHRU MODE
    if(!thermostatC.get_passthru())
    {   
        if (thermostatC.get_new_current_temperatrue_status())
        {
          thermostatC.set_new_current_temperature_status(false);
          thermostatC.update_display(&display);
          delay(2000);
          thermostatC.toggle_activation_pin();
        }

        if(thermostatC.get_new_temp_setting_status())
        {
          thermostatC.set_new_temp_setting_status(false);
          thermostatC.update_display(&display);
          delay(2000);
          thermostatC.toggle_activation_pin();
        }

        if (digitalRead(thermostatC.get_resetPin()) == LOW){
          esp_restart();
        }
        
        if (digitalRead(passthru_pin) == LOW)
        {
          if(!thermostatC.get_passthru())
          {
            thermostatC.set_passthru(true);
          }
        }
    } // ends passthru if statement

    
    //PASS THROUGH MODE
    if(thermostatC.get_passthru())
    {         
      thermostatC.passthru_display(&display);
          if(!(digitalRead(thermostatC.get_relay_activation_pin()) == HIGH))
          {
            digitalWrite(thermostatC.get_relay_activation_pin(),HIGH);
          }
          //TAKING OUT OF PASS THROUGH MODE WHILE IN PASS THROUGH MODE
          if(digitalRead(passthru_pin) == HIGH)
          {
            if(DEBUG)
            {
              Serial.println("LEAVING PASS THOURGH MODE");
            }
            thermostatC.set_passthru(false);
            delay(250);
            thermostatC.toggle_activation_pin();
            delay(250);
            thermostatC.update_display(&display);
            delay(250);
          } 
      delay(3000);

    }
  }

}
































