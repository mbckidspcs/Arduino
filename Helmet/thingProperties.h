
    
    #include <ArduinoIoTCloud.h>
    #include <Arduino_ConnectionHandler.h>

    const char DEVICE_LOGIN_NAME[]  = "251264e9-1965-43ac-871c-a1d03f4f2745";

    const char SSID[]               = "Fiber";    // Network SSID (name)
    const char PASS[]               = "123456789";    // Network password (use for WPA, or use as key for WEP)
    const char DEVICE_KEY[]  = "6EeePwQUA2336!LwrNgL61NGr";    // Secret device password
    
   
void onMobileNumChange();
void onHumidChange();
void onTempChange();
void onSliderChange();
void onLedChange();

String mobileNum;
float humid;
float temp;
int slider;
bool led;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(mobileNum, READWRITE, ON_CHANGE, onMobileNumChange);
  ArduinoCloud.addProperty(humid, READWRITE, ON_CHANGE, onHumidChange);
  ArduinoCloud.addProperty(temp, READWRITE, ON_CHANGE, onTempChange);
  ArduinoCloud.addProperty(slider, READWRITE, ON_CHANGE, onSliderChange);
  ArduinoCloud.addProperty(led, READWRITE, ON_CHANGE, onLedChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
