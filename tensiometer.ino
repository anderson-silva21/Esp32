#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) 12 hours == 43200 seconds*/

RTC_DATA_ATTR int bootCount = 0;

int ESP_DC_BOOST = 17;
int LED = 22; // tx ou rx nao configuravel ;(
int BUTTON = 33;
int ADC_SENSOR = 35;
int EstadoBotao = 0;
int ADC_BATT = 32; //esta conectado por um fio o r5 ao gpio32
float voltage = 3.9;

BluetoothSerial SerialBT;
int analogValue;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  SerialBT.begin("Tensiometooth"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  pinMode(ESP_DC_BOOST, OUTPUT);
  pinMode(ADC_SENSOR, INPUT);
  pinMode(ADC_BATT, INPUT);
  pinMode(BUTTON, INPUT);//interrupcao

  Serial.println("Going to sleep now");
  delay(1000);
  Serial.flush(); 
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  voltage = analogRead(ADC_BATT)*3.3/4096;//conversao da leitura da bateria
  Serial.println(voltage);
  digitalWrite(ESP_DC_BOOST, HIGH);
  analogValue = analogRead(ADC_SENSOR);
  SerialBT.write(analogValue%256);
  SerialBT.write(analogValue/256);
  Serial.write(SerialBT.read());
  SerialBT.write(voltage);
  Serial.write(SerialBT.read());
  Serial.println(analogValue);
  Serial.println(voltage);
}
