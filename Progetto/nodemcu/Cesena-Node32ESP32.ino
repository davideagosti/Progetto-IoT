//--------------------------------DHT------------------------------------------
// libreria per comunicazione con i sensori DHT
#include <DHT.h>  

// definisco i parametri dei sensori effettivamente installati
#define DHTMODEL1 DHT22 //DHT22
#define DHTMODEL2 DHT22 //DHT22
//3//#define DHTMODEL3 DHT11 //DHT22
#define DHTPIN1 25
#define DHTPIN2 22
//3//#define DHTPIN3 27

// istanzio gli oggetti di classe DHT
DHT sensorDht1(DHTPIN1, DHTMODEL1);
DHT sensorDht2(DHTPIN2, DHTMODEL2);
//3//DHT sensorDht3(DHTPIN3, DHTMODEL3);

//--------------------------------WiFi-----------------------------------------
// Libreria per per collegamento Wifi, Servizi NTP e MQTT
#include <WiFi.h> 
// definisco le variabili per il collegamento Wifi
const char* ssid = "SSID";
const char* password =  "pwd";
String macAddress; // ClientId Identificatore univoco della fonte dei dati
//--------------------------------NTP------------------------------------------
// NTP per acquisire il data e ora del rilevamento
#include <NTPClient.h>
#include <WiFiUdp.h>

// Define NTP Client per acquisire l'ora esatta 
WiFiUDP ntpUDP;
const char* ntpServer = "europe.pool.ntp.org";
const long gmtOffset_sec = 3600; //Time Zone Italia GMT +1
const int  daylightOffset_sec = 3600; // DST DayLightSaving vs STD Standard Time
NTPClient timeClient(ntpUDP, ntpServer, daylightOffset_sec, 60000);
// Variabili per gestire data e ora
String formattedDate;
String dayStamp;
String timeStamp;

// ----------------------------------------MQTT--------------------------------
// Libreria per Pubblicare i dati al MQTT Broker
#include <MQTTPubSubClient_Generic.h>

// Instanzio gli oggetti
WiFiClient netClient;
MQTTPubSubClient mqttClient;

// definisco le variabili per MQTT
//#define MQTT_SERVER         "raspberrypi"
// in attesa del RaspBerryPi proviamo MQTT utilizzando un servizio online
#define MQTT_SERVER        "public.cloud.shiftr.io"
#define MQTT_PORT           1883
String SYSMON = "ESP32BO"; 
const char* mqttUser     = "public";
const char* mqttPassword = "public";
// Topic to publish
const char* PubTopic     = "/mqttPubSub";
// Topic Message to publish
String PubMessage1  = "Hello: " + SYSMON + " DHT" + DHTMODEL1;
String PubMessage2  = "Hello form: " + SYSMON + "DHT" + DHTMODEL2;
//3//String PubMessage3  = "Hello form: " + SYSMON + "Type: " + DHTMODEL3;
//-----------------------------------------------------------------------------

void setup() {
  // Debug console
  Serial.begin(115200);
  delay(2000);

  WifiConnect();
  macAddress = printWifiStatus();

  timeClient.update();
  Serial.print("TimeClient.Update()");
  Serial.println(getTimeStampString());
  // Inizializzo NTPClient per ottenere l'orario
  timeClient.begin();
  Serial.print("TimeClient.begin()");
  Serial.println(getTimeStampString());
  //inizializzo i sensori
  sensorDht1.begin();
  sensorDht2.begin();
  //3//sensorDht3.begin();

  MQTTConnection();
}

void loop(){
  // acquisizione delle rilevazioni dei sensori
  String tm1 = ReadDHTTemperature1();
  String hm1 = ReadDHTHumidity1();
  String tm2 = ReadDHTTemperature2();
  String hm2 = ReadDHTHumidity2();
  //3//String tm3 = ReadDHTTemperature3();
  //3//String hm3 = ReadDHTHumidity3();

  formattedDate = getTimeStampString();
  //// Estaggo data e ora
  int split = formattedDate.indexOf(" ");
  dayStamp = formattedDate.substring(0, split);

  timeStamp = formattedDate.substring(split+1, formattedDate.length());

  Serial.print("ClientId: ");
  Serial.print(macAddress);
  Serial.print(" ;");

  Serial.print("SensorNr: ");
  Serial.print("1");
  Serial.print(" ;");

  Serial.print("SensorType: ");
  Serial.print(DHTMODEL1);
  Serial.print(" ;");

  Serial.print("DATE: ");
  Serial.print(dayStamp);
  Serial.print(" ;");

  Serial.print("HOUR: ");
  Serial.print(timeStamp);
  Serial.print(" ;");

  Serial.print("Celsius: ");
  Serial.print(tm1);
  Serial.print(" ;");
  
  Serial.print("Humidity RH%: ");
  Serial.print(hm1);
  Serial.println(" ;");

  PubMessage1 = (macAddress + ";1;" + DHTMODEL1 + ";" + dayStamp + ";" + timeStamp + ";" + tm1 + ";" + hm1 + ";");
  //2// eventuale secondo sensore
  Serial.print("ClientId: ");
  Serial.print(macAddress);
  Serial.print(" ;");

  Serial.print("SensorNr: ");
  Serial.print("2");
  Serial.print(" ;");

  Serial.print("SensorType: ");
  Serial.print(DHTMODEL2);
  Serial.print(" ;");
  
  Serial.print("DATE: ");
  Serial.print(dayStamp);
  Serial.print(" ;");

  Serial.print("HOUR: ");
  Serial.print(timeStamp);
  Serial.print(" ;");

  Serial.print("Celsius: ");
  Serial.print(tm2);
  Serial.print(" ;");
  
  Serial.print("Humidity RH%: ");
  Serial.print(hm2);
  Serial.println(" ;");

  PubMessage2 = (macAddress + ";2;" + DHTMODEL2 + ";" + dayStamp + ";" + timeStamp + ";" + tm2 + ";" + hm2 + ";");
  //*/
  /* //3// eventuale terzo sensore
  Serial.print("ClientId: ");
  Serial.print(macAddress);
  Serial.print(" ;");

  Serial.print("SensorNr: ");
  Serial.print("3");
  Serial.print(" ;");

  Serial.print("SensorType: ");
  Serial.print(DHTMODEL3);
  Serial.print(" ;");

  Serial.print("DATE: ");
  Serial.print(dayStamp);
  Serial.print(" ;");

  Serial.print("HOUR: ");
  Serial.print(timeStamp);
  Serial.print(" ;");

  Serial.print("Celsius: ");
  Serial.print(tm3);
  Serial.print(" ;");
  
  Serial.print("Humidity RH%: ");
  Serial.print(hm3);
  Serial.println(" ;");
  
  PubMessage3 = (macAddress + ";3;" + DHTMODEL3 + ";" + dayStamp + ";" + timeStamp + ";" + tm3 + ";" + hm3 + ";");
  */
  delay(2000); // i DHT22 effettuano una misurazione ogni 2 secondi
  
  // --------------------------------------------------------------------------
  // pubblico i dati ogni 30 sec al broker MQTT
  // Cisacun sensore ha un PubMessage con questo formato: ClientId + SensorId + Data + Ora + Temperatura + Umidità

  mqttClient.update();  // should be called

  // publish message
  static uint32_t prev_ms = millis();

  if (millis() > prev_ms + 30000)
  {
    prev_ms = millis();
    mqttClient.publish(PubTopic, PubMessage1);
    mqttClient.publish(PubTopic, PubMessage2);
    //3//mqttClient.publish(PubTopic, PubMessage3);
  }
  
}

void WifiConnect(){
  Serial.print("\nConnection to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  delay(1000);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("-");
  }
}

String printWifiStatus() {
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID()); 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("MAC Address: ");
  String macAddress = WiFi.macAddress();
  Serial.println(macAddress);
  // print the signal strength:
  int32_t rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
  return macAddress; // serve come ClientId
}

String getTimeStampString() {
   time_t rawtime = timeClient.getEpochTime(); 
   struct tm * ti;
   ti = localtime (&rawtime);

   uint16_t year = ti->tm_year + 1900;
   String yearStr = String(year);

   uint8_t month = ti->tm_mon + 1;
   String monthStr = month < 10 ? "0" + String(month) : String(month);

   uint8_t day = ti->tm_mday;
   String dayStr = day < 10 ? "0" + String(day) : String(day);

   uint8_t hours = ti->tm_hour;
   String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

   uint8_t minutes = ti->tm_min;
   String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

   uint8_t seconds = ti->tm_sec;
   String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

   return yearStr + "-" + monthStr + "-" + dayStr + " " +
          hoursStr + ":" + minuteStr + ":" + secondStr;
}

String ReadDHTTemperature1(){
  float t = sensorDht1.readTemperature();
  if (isnan(t)){
    Serial.println("mancata lettura dei dati del sensore di Temperatura DHT");
    return "";
  }
  else{
    return String(t);
  }
}
///* //2//
String ReadDHTTemperature2(){
  float t = sensorDht2.readTemperature();
  if (isnan(t)){
    Serial.println("mancata lettura dei dati del sensore di Temperatura DHT");
    return "null";
  }
  else{
    return String(t);
  }
}
//*/
/* //3//
String ReadDHTTemperature3(){
  float t = sensorDht3.readTemperature();
  if (isnan(t)){
    Serial.println("mancata lettura dei dati del sensore di Temperatura DHT");
    return "null";
  }
  else{
    return String(t);
  }
}
*/
String ReadDHTHumidity1(){
  float h1 = sensorDht1.readHumidity();
  if (isnan(h1)){
    Serial.println("mancata lettura dei dati del sensore di Umidità");
    return "";
  }
  else{
    return String(h1);
  }
}
///* //2//
String ReadDHTHumidity2(){
  float h2 = sensorDht2.readHumidity();
  if (isnan(h2)){
    Serial.println("mancata lettura dei dati del sensore di Umidità");
    return "null";
  }
  else{
    return String(h2);
  }
}
//*/
/* //3//
String ReadDHTHumidity3(){
  float h3 = sensorDht3.readHumidity();
  if (isnan(h3)){
    Serial.println("mancata lettura dei dati del sensore di Umidità");
    return "null";
  }
  else{
    return String(h3);
  }
}
*/

void MQTTConnection(){
 
  Serial.print("Connecting to mqtt host...");
  Serial.println(MQTT_SERVER);

  //mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  while (!netClient.connect(MQTT_SERVER, MQTT_PORT))
  {
    Serial.print("+");
    delay(1000);
  }
 
  Serial.println(" connected!");

  // initialize mqtt client
  mqttClient.begin(netClient);

  Serial.print("Connecting to mqtt broker...");

  while (!mqttClient.connect("ESP32BO", mqttUser, mqttPassword)){
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" Broker connected!");

  // subscribe callback which is called when every packet has come
  mqttClient.subscribe([](const String & topic, const String & payload, const size_t size)
  {
    (void) size;
    Serial.println("MQTT received: " + topic + " - " + payload);
  });

  // subscribe topic and callback which is called when /hello has come
  mqttClient.subscribe(PubTopic, [](const String & payload, const size_t size)
  {
    (void) size;

    Serial.print("Subcribed to ");
    Serial.print(PubTopic);
    Serial.print(" => ");
    Serial.println(payload);
  });

  mqttClient.publish(PubTopic, PubMessage1);
  mqttClient.publish(PubTopic, PubMessage1);
  //3//mqttClient.publish(PubTopic, PubMessage1);
}