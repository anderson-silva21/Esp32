#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define BUTTON_PIN_BITMASK 0x200000000 //2^33 in hex
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  10        /* Time ESP32 will go to sleep (in seconds) 12 hours == 43200 seconds*/

RTC_DATA_ATTR int bootCount = 0;
//variaveis globais
int ESP_DC_BOOST = 17;
int LED = 22; // tx ou rx nao configuravel ;(
int BUTTON = 33;
int ADC_SENSOR = 35;
int EstadoBotao = 0;
int ADC_BATT = 32; //esta conectado por um fio o r5 ao gpio32
float voltage = 3.9;
int analogValue;

BluetoothSerial SerialBT;//variavel bluetooth

void print_wakeup_reason(){ //printa oq fez o esp acordar
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

void gotoSleep() {//metodo que faz o esp dormir
  log_d("Indo dormir");
  
  // Permitir acordar se o botão for pressionado
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON, LOW);  // botao: GPIO_NUM_33

  // Permitir RTC acordar o esp depois de X segundos
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Inicia o deepSleep
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  //pinmodes
  pinMode(ESP_DC_BOOST, OUTPUT);
  pinMode(ADC_SENSOR, INPUT);
  pinMode(ADC_BATT, INPUT);
  pinMode(BUTTON, INPUT);
  delay(1000); //Reserve algum tempo para abrir o Serial Monitor
 
  //Incremente o número de inicialização e imprima-o a cada reinicialização
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  
  //Imprima o motivo de ativação do ESP32
  print_wakeup_reason();
  // All done
  Serial.println("Setup completed");
  
}

void loop() {
  
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1){ // se o esp acordou por conta do botao apertado

    SerialBT.begin("Tensiometooth"); //Liga o bluetooth e da um nome
    Serial.println("O dispositivo foi iniciado, agora você pode emparelhá-lo com bluetooth!");

    //calculos
    voltage = analogRead(ADC_BATT)*3.3/4096;//conversao da leitura da bateria
    Serial.println(voltage);
    digitalWrite(ESP_DC_BOOST, HIGH);
    analogValue = analogRead(ADC_SENSOR);
    SerialBT.write(analogValue%256);//conversao da leitura do sensor
    SerialBT.write(analogValue/256);//conversao da leitura do sensor
    
    delay(120000);//2minutos para conectar no bluetooth e receber os dados
    
    Serial.write(SerialBT.read());//escrita da leitura do sensor
    SerialBT.write(voltage);
    delay(500);
    Serial.write(SerialBT.read());//escrita da leitura da bateria 
    Serial.println(analogValue);
    
    Serial.println("Indo dormir");
    delay(1000);
    Serial.flush(); 
    gotoSleep();
    Serial.println("ESSE PRINT NUNCA IRÁ APARECER");
  }

  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER){
    
    //calculos
    voltage = analogRead(ADC_BATT)*3.3/4096;//conversao da leitura da bateria
    Serial.println(voltage);
    digitalWrite(ESP_DC_BOOST, HIGH);
    analogValue = analogRead(ADC_SENSOR);
    SerialBT.write(analogValue%256);
    SerialBT.write(analogValue/256);
    //nvs_set_u16(SerialBT.read()); //gravação de um dado do tipo inteiro de 16 bits sem sinal.
    Serial.write(SerialBT.read());//enviando 2bytes por bluetooth
    SerialBT.write(voltage);
    //nvs_set_u16(SerialBT.read()); //gravação de um dado do tipo inteiro de 16 bits sem sinal.
    Serial.write(SerialBT.read());
    Serial.println(analogValue);
    
    Serial.println("Indo dormir");
    delay(1000);
    Serial.flush(); 
    gotoSleep();
    Serial.println("ESSE PRINT NUNCA IRÁ APARECER");
  }

}
