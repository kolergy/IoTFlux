


#define I2C
#define WIFI


#ifdef I2C
#define SHT3X  
//#define BMP
#endif //I2C

#ifdef WIFI
#define NTP
#define MQTT
#endif //WIFI

#define LED_PIN 5
#define SENS_FACT 2.255
#define V_BAT_PIN 35
#define SAMPLE_REPEATS 30    // number of sample repeat measurements 
#define HOST_NAME "ioTFlux_sens_001"

#include <Arduino.h>
#include <ArduinoNvs.h>      // By rpolitex
#include <ArduinoJson.h>     // By Benoit Blanchon
#include <THandP.h>          // By Kolergy

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

WiFiClient   wifi_client;
WiFiUDP      ntpUDP;  


#ifdef NTP
#include <NTPClient.h>       // by Fabrice Weinberg
#include <TimeLib.h>         // by Paul Stoffregen  // by Michael Margolis
NTPClient    time_client = NTPClient(ntpUDP);
#endif // NTP


#ifdef MQTT
#include <PubSubClient.h>  //  by Nick O'Leary
PubSubClient mqtt_client(wifi_client);
// "192.168.1.196" //MQTT Artilect
//"192.168.71.206"
const char* mqtt_server   = "192.168.1.196";
const int   mqtt_port     = 1883;  //1883, 12948;


const char* mqtt_topic    = "/here/sens01/";

#endif // MQTT

#endif // WIFI

#define N_CRED 4
String my_cred[N_CRED][2] = {{"wifi_ssid", ""}, {"wifi_pass", ""}, {"MQTT_user", ""}, {"MQTT_pass", ""}};

#ifdef I2C
void  I2CScan();
#endif //I2C

#ifdef MQTT
void    mqtt_reconnect(                         );
void    mqtt_publish(    const char*, String    );
void    mqtt_publish(    const char*, char*     );
#endif // MQTT

float   ReadVoltage(          byte              );
void    displayESP32Info(                       ); 
bool    getCredentialsFromNVS(                  );
bool    RequestCredentials(                     );
char*   getInput(                               );
void    clear_credentials_from_NVS(             ); 
const char* get_credential(String );

char          deviceid[9];
const char*   m_deviceNam = HOST_NAME;
unsigned long t_zero_s    = 0;
unsigned long t_mes_s     = 0;

StaticJsonDocument<10000> j_root;          // Memoire pour l'arbre JSON. ATTENTION: Augmenter la valeur quand on augmente la taille du JSON


void setup() {
  unsigned long t_0_init_s = millis();
  Serial.begin(115200);

  NVS.begin();                             // collecting data stored on the long term memory
  clear_credentials_from_NVS();
  bool cred_ok = getCredentialsFromNVS();
  if(cred_ok==false) RequestCredentials();

  pinMode(     LED_PIN, OUTPUT);
  digitalWrite(LED_PIN,    LOW);           // LED on during Setup

  Serial.flush();  
  Serial.printf("\nWelcome to IOTFlux\n");

  uint64_t chipid = ESP.getEfuseMac();     // some infos about the board
  sprintf(deviceid, "%" PRIu64, chipid);   // Generate the device ID to identify the sensor
  displayESP32Info(); 

  #ifdef I2C
  I2CScan();

  #ifdef SHT3X
  Serial.print("Waking-up the SHT31");
  sht.begin(SHT31_ADDRESS);
  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.println(stat, HEX);
  #endif // SHT3X

  #endif //I2C

  #ifdef WIFI
  Serial.println("Connecting to WiFi ...");
  WiFi.setHostname(HOST_NAME);
  Serial.println("HERE");
  const char* ssid = get_credential("wifi_ssid");
  const char* pass = get_credential("wifi_pass");
  Serial.printf("ssid: %s, Pass: %s", ssid, pass);
  WiFi.begin(ssid, pass);
  //Serial.println("SSID    : " + String(ssid) + " Password: " + String(pass));
  int n = 0;
  while (WiFi.status() != WL_CONNECTED && n < 20) {
    Serial.print(".");
    n++;
    delay(500);
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi connected, IP address:");
    Serial.println(WiFi.localIP());
    #ifdef MQTT
    //*********************** MQTT Setup *****************************************
    mqtt_client.setServer(mqtt_server, mqtt_port);
    mqtt_reconnect();

    #endif // MQTT
  } else Serial.printf("\nWifi connection failed\n");

  #endif // WIFI
  Serial.printf("\nonfiguration complete t:%d ms\n", millis()-t_0_init_s);
  digitalWrite(LED_PIN,  HIGH);  // End of Setup LED off

}

void loop() {
  //Serial.println("Loop");
  delay(1000);
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
    bat_v        += ReadVoltage(V_BAT_PIN)*SENS_FACT; 

    #ifdef WIFI
    wifi_rssi_db += WiFi.RSSI();
    #endif //WIFI
  }

  bat_v          /= SAMPLE_REPEATS;
  wifi_rssi_db   /= SAMPLE_REPEATS;
  temp_c         /= SAMPLE_REPEATS;
  rh_pct         /= SAMPLE_REPEATS;

#ifdef SHT3X
  float hi_c          = calcHeatIndex( temp_c, rh_pct );
  float t_dew_c       = calcDewPoint(  temp_c+C2K, rh_pct/100 )-C2K;
  float t_frost_c     = calcFrostPoint(temp_c+C2K, t_dew_c+C2K)-C2K;
  float W_C_kg_m3     = WContent( temp_c+C2K, rh_pct/100 ); 
  //Serial.printf("Rh: %5.2f\%, T:%5.2f°C, HeatIdx:%5.2f°C, dew: %5.2f°C, frost:%5.2f°C\n", rh_pct, temp_c, hi_c, t_dew_c, t_frost_c);
  j_root["deviceid" ] = deviceid;
  j_root["rh_pct"   ] = rh_pct;
  j_root["temp_c"   ] = temp_c;
  j_root["hi_c"     ] = hi_c;
  j_root["t_dew_c"  ] = t_dew_c;
  j_root["t_frost_c"] = t_frost_c;
  j_root["W_C_kg_m3"] = W_C_kg_m3;

#endif //SHT3X  

#ifdef BMP
  float rho_kg_m3    = get_air_density(P_amb_Pa, T_amb_K);
  j_root["rho_kg_m3"   ] = rho_kg_m3;

#endif //BMP

  //Serial.printf("bat_v: %6.3f V\n", bat_v);
  j_root["bat_v"       ] = bat_v;

#ifdef WIFI
  //Serial.printf("RSSI: %5.2f \n", wifi_rssi_db);
  j_root["wifi_rssi_db"] = wifi_rssi_db;
  j_root["local_IP"    ] = WiFi.localIP();


  #ifdef MQTT
  String jsonStr;
  serializeJson(j_root, jsonStr);       // put the json in a string
  //Serial.println("StringB  OK");
  Serial.println(jsonStr);

  mqtt_publish(      mqtt_topic, jsonStr);
  #endif // MQTT
  #endif //WIFI
}


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
  Serial.println("chipID      : " + String(chipid       ));
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


#ifdef MQTT
void mqtt_reconnect() {
  while (!mqtt_client.connected()) {                // Loop until we're reconnected
    Serial.print("Attempting MQTT connection...");
    //if (mqtt_client.connect("SenseClient", mqtt_user, mqtt_password)) {  // Attempt to connect
    if (mqtt_client.connect("SenseClient")) {  // Attempt to connect
      Serial.println("MQTT connected");
    } else {
      Serial.print("MQTT conncetion failed, error code=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}


void mqtt_publish(const char* top_char, String val_str) {   // Publish a message to the MQTT broker
  //Serial.println("String val: " + val_str);
  char val_char[  val_str.length() + 1];
  val_str.toCharArray(val_char, val_str.length() + 1);
  mqtt_publish(top_char, val_char);
}


void mqtt_publish(const char* top_char, char* val_char) {   // Publish a message to the MQTT broker
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  char topic_char[strlen(top_char) + 1];
  strcpy(topic_char, top_char);
  Serial.println("\nPublishing to topic : " + String(topic_char) );
  //Serial.println("Val : " + String(val_char));
  mqtt_client.publish(topic_char,val_char);
}
#endif //MQTT


bool RequestCredentials() {
  bool resS    = false;
  bool cred_ok = true;
  Serial.printf("\nRequesting the credentials via Serial:\n");
  for(int i=0;i<N_CRED;i++) {
    Serial.printf("cred: %s: %s\n", my_cred[i][0], my_cred[i][1]); 
    if(my_cred[i][1] == ""){  // cred miissing request it from user
      Serial.printf("Please enter %s:", my_cred[i][0]);
      my_cred[i][1] = String(getInput());
      resS          = NVS.setString(my_cred[i][0], my_cred[i][1]);
      if(resS) Serial.printf("\n %s Updated with:%s\n", my_cred[i][0], my_cred[i][1]);
      else     cred_ok = false;
    }
  }
  Serial.printf("cred_ok; %s\n", cred_ok);
  return(cred_ok);
}


char* getInput() {               // Get user input from Serial
  char        buf[256];
  static char rtrn[256];
  boolean     newD     = false;
  byte        ndx      = 0;
  char        endMark  = '\n';
  char        rc;

  while (newD == false && ndx < 255) {
    if(Serial.available() > 0) {
      rc = Serial.read();
      if (rc != endMark && int(rc) !=13) {
        buf[ndx] = rc;
        Serial.printf("%c",buf[ndx]);
        ndx++;
      } else {
        buf[ndx] = '\0'; // terminate the string
        newD     = true;
        while (Serial.available()) Serial.read();
      }
    }
  }
  Serial.println("\n:" + String(buf) + ":");
  strncpy(rtrn, buf, 255);
  return rtrn;
  Serial.flush(); 
}


bool getCredentialsFromNVS() {  // Get credentials from NVS memory if available or request them through serial
  bool cred_ok = true;
  for(int i=0;i<N_CRED;i++) {
    my_cred[i][1] = NVS.getString(my_cred[i][0]);
    if(my_cred[i][1] == "") cred_ok = false;
  }
  return(cred_ok);
}

const char* get_credential(String cred_name) {
  Serial.printf("get_credential\n");
  for(int i=0;i<N_CRED;i++) {
    Serial.printf("cred_name: %s, stores:%s\n",cred_name, my_cred[i][0]);
    if(my_cred[i][0] == cred_name) {  
      char name_ch[my_cred[i][1].length() + 1];
      cred_name.toCharArray(name_ch, my_cred[i][1].length() + 1);
      const char* name_cch = name_ch;
      Serial.printf("Return: %s\n",name_cch);
      return(name_cch);
    }
  }
  return("");
}

void clear_credentials_from_NVS() {  // Clear credentials from NVS memory 
  for(int i=0;i<N_CRED;i++) NVS.setString(my_cred[i][0], "");
}