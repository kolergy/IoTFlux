


#define I2C
#define WIFI


#ifdef I2C
#define SHT3X  
//#define BMP
#endif //I2C

#ifdef WIFI
//#define NTP
//#define MQTT
#endif //WIFI

#define LED_PIN 5
#define SENS_FACT 2.255
#define V_BAT_PIN 35
#define SAMPLE_REPEATS 30    // number of sample repeat measurements 

#include <Arduino.h>

#ifdef I2C
#include <Wire.h>

#ifdef SHT3X
#include <SHT31.h>  // by Rob Tillaart
SHT31    sht;
#define SHT31_ADDRESS   0x44
#endif //SHT3X
#endif // I2C

#ifdef WIFI

#include <WiFi.h>

#ifdef NTP
#include <NTPClient.h>       // by Fabrice Weinberg
#include <TimeLib.h>         // by Paul Stoffregen  // by Michael Margolis
#endif // NTP


#ifdef MQTT
#include <PubSubClient.h>  //  by Nick O'Leary
#endif // MQTT

#include <THandP.h>



#endif // WIFI

#ifdef I2C
void  I2CScan();
#endif //I2C

float ReadVoltage(           byte );

void setup() {
  Serial.begin(115200);
  if(Serial.available()) Serial.flush();  

  
  #ifdef SHT3X
    sht.begin(SHT31_ADDRESS);
    Wire.setClock(100000);
      uint16_t stat = sht.readStatus();
    Serial.println(stat, HEX);
  #endif // SHT3X

}

void loop() {
  float bat_v        =  0; 
  float wifi_rssi_db =  0;
  float temp_c       =  0;
  float rh_pct       =  0;
  
  for(int i = 0; i < SAMPLE_REPEATS; i++) {
#ifdef SHT3X
    sht.read();         // default = true/fast       slow = false
    temp_c       += sht.getTemperature();
    rh_pct       += sht.getHumidity();
#endif //SHT3X  
    bat_v        += ReadVoltage(V_BAT_PIN); 
    #ifdef WIFI
    wifi_rssi_db += WiFi.RSSI();
    #endif //WIFI

  }
  bat_v        /= SAMPLE_REPEATS;
  wifi_rssi_db /= SAMPLE_REPEATS;
  temp_c       /= SAMPLE_REPEATS;
  rh_pct       /= SAMPLE_REPEATS;

#ifdef SHT3X
  float hi_c         = calcHeatIndex( temp_c, rh_pct );
  float t_dew_c      = calcDewPoint(  temp_c, rh_pct );
  float t_frost_c    = calcFrostPoint(temp_c, t_dew_c);
#endif //SHT3X  

#ifdef BMP
  float rho_kg_m3    = get_air_density(P_amb_Pa, T_amb_K);
#endif //BMP

}

/*float ReadVoltage(byte pin){  // From: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
  long r =  0;
  int n  = 100;
  for(int i=0;i<n;i++) { 
    //Serial.println(r);
    r += analogRead(pin); 
  }
  // read normal Arduino value
  float in0 = r/n;
  //int in0 = analogRead(byte pin);
  float val0 = in0 * 5.0 / 1024.0;
  //Serial.println(in0);

  // read correct supply voltage
  U_ref_V        = readVcc() / 1000.0;
  float val0Corrected = U_ref_V / 5 * val0;
  return(val0Corrected);
} */





#ifdef I2C
void I2CScan() {
  Serial.println();
  Serial.println("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0) {  // Receive 0 = success (ACK response)
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);     // PCF8574 7 bit address
      Serial.println(")");
      count++;
    }
  }
  Serial.print("Found ");
  Serial.print(count, DEC);        // numbers of devices
  Serial.println(" device(s).");
}
#endif //I2C

void displayESP32Info() {
  char     deviceid[21];
  uint64_t chipid              = ESP.getEfuseMac();    // some infos about the board
  uint32_t CPUFreq             = ESP.getCpuFreqMHz();
  int      coreID              = xPortGetCoreID();
  sprintf(deviceid, "%" PRIu64, chipid);
  Serial.println("--------------------------------------");
  Serial.println("Board       : " + String(ARDUINO_BOARD));
  Serial.println("DeviceID    : " + String(deviceid     ));
  Serial.println("CPUFreqency : " + String(CPUFreq      ));
  Serial.println("Used Core ID: " + String(coreID       ));
  Serial.println("--------------------------------------");
}

float ReadVoltage(byte pin){  // From: https://github.com/G6EJD/ESP32-ADC-Accuracy-Improvement-function/blob/master/ESP32_ADC_Read_Voltage_Accurate.ino
  double reading = analogRead(pin); // Reference voltage is 3v3 so maximum reading is 3v3 = 4095 in range 0 to 4095
  if(reading < 1 || reading > 4095) return 0;
  // return -0.000000000009824 * pow(reading,3) + 0.000000016557283 * pow(reading,2) + 0.000854596860691 * reading + 0.065440348345433;
  return float(-0.000000000000016 * pow(reading,4) + 0.000000000118171 * pow(reading,3)- 0.000000301211691 * pow(reading,2)+ 0.001109019271794 * reading + 0.034143524634089);
} // Added an improved polynomial, use either, comment out as required

