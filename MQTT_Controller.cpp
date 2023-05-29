#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MPU6050.h>
#include <string>

int SEND_FR = 100;
long lastMsg = 0;
//YOUR LOCAL WIFI
const char* ssid     = "SSID";
const char* password = "PASSWORD";

//YOUR MQTT SETTINGS
const char* mqtt_server ="broker.mqttdashboard.com"; 
const char* mqtt_username = "ESP_8266";
const char* mqtt_password = "12345678";
const char* inTopic = "MQTT_PILOT/#";
const int mqtt_port =1883;

WiFiClient espClient;
PubSubClient client(espClient);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

MPU6050 mpu;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
#define POT_PIN A0
#define BUTTON1_PIN D0 //UP
#define BUTTON2_PIN D6 //LEFT
#define BUTTON3_PIN D7 //DOWN
#define BUTTON4_PIN D5 //RIGHT
#define LED1_PIN D8
#define LED2_PIN D4
#define ledPin 2


void blink_led(unsigned int times, unsigned int duration){
  for (int i = 0; i < times; i++) {
    digitalWrite(2, HIGH);
    delay(duration);
    digitalWrite(2, LOW);  
    delay(200);
  }
}

void setup_wifi() {
  delay(50);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    blink_led(2,200); 
    Serial.print(".");
    c=c+1;
    if(c>10){
        ESP.restart(); //restart ESP after 10 seconds
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}
void connect_mqttServer() {
  while (!client.connected()) {

    if(WiFi.status() != WL_CONNECTED){
      setup_wifi();
    }
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP_8266";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe(inTopic);   // subscribe the topics here
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");  
      blink_led(3,200);
      delay(2000);
    }
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  //Serial.print("Message arrived on topic: ");
  //Serial.print(topic);
  //Serial.print(". Message: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
   // Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  if (String(topic) == "MQTT_PILOT/ANSWER") {
   Serial.print("THE ANSWER HAS ARRIVED");
    }
  }

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 Disolay error"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("START");display.println("!");
  display.print("   ");display.println("  ");
  display.print("Witamy w aplikacji pilot");display.println("!");
  display.display();
  delay(3000);

  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON1_PIN, INPUT );
  pinMode(BUTTON2_PIN, INPUT );
  pinMode(BUTTON3_PIN, INPUT );
  pinMode(BUTTON4_PIN, INPUT );
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  Serial.println();
  Serial.print("Connecting to:");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Lacze z WIFI");display.println("!");
  display.print("   ");display.println("  ");
  display.print("Prosze czekac");display.println("");
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
  display.print(".");
  display.display();
  delay(500);
  Serial.print(".");
  }


  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("Adres IP: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("WIFI");display.println("");
  display.print("POLACZONO");display.println("  ");
  display.display();
  delay(1000);
  mpu.initialize();
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  delay(2000);
  
}

void loop() {
  if (!client.connected()) {
    connect_mqttServer();
}
client.loop();

   int16_t ax, ay, az;
   int16_t gx, gy, gz;

  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  int y = constrain(map(ax, -32768, 32767, 0, SCREEN_WIDTH - 1), 0, SCREEN_WIDTH - 1);
  int x = constrain(map(ay, -32768, 32767, 0, SCREEN_HEIGHT - 1), 0, SCREEN_HEIGHT - 1);
  int x2 = constrain(map(ax, -32768, 32767, 0, SCREEN_WIDTH - 1), 0, SCREEN_WIDTH - 1);
  int y2 = constrain(map(ay, -32768, 32767, 0, SCREEN_HEIGHT - 1), 0, SCREEN_HEIGHT - 1);


  int potValue = analogRead(POT_PIN);
  String str_button1 = digitalRead(BUTTON1_PIN) ? "false" : "true";
  String str_button2 = digitalRead(BUTTON2_PIN) ? "false" : "true";
  String str_button3 = digitalRead(BUTTON3_PIN) ? "false" : "true";
  String str_button4 = digitalRead(BUTTON4_PIN) ? "false" : "true";

  int realPOT = (potValue - 18) * 100 / 1006;
  int led_Val = realPOT * 2.55;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  if(realPOT<33){
  display.print("PRZYCISKI");display.println("!");
  display.print("Button 1: "); display.println(str_button1);
  display.print("Button 2: "); display.println(str_button2);
  display.print("Button 3: "); display.println(str_button3);
  display.print("Button 4: "); display.println(str_button4);
  

  if(str_button1 == "true"){
    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR", "100;0"); 
      display.print("SEND: "); display.println("100;0");
    }
  }
  if(str_button2 == "true"){
    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR", "50;-50"); 
      display.print("SEND: "); display.println("50;-50");
    }
  }
  if(str_button3 == "true"){

    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR", "-100;0"); 
      display.print("SEND: "); display.println("-100;0");
    }
  }
  if(str_button4 == "true"){

    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR", "50;50"); 
      display.print("SEND: "); display.println("50;50");
    }
  }
  if((str_button1 == "false")&&(str_button2 == "false")&&(str_button3 == "false")&&(str_button4 == "false")){
    
    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR", "0;0");
      display.print("SEND: "); display.println("0;0");
      }
    } 

    display.display();
  }

  if(realPOT>66){

  int V = map(y, 45, 95, 99, -99);
  int O = map(x, 20, 40, 25, -25);

  display.fillCircle(y2, x2, 5, WHITE); 
  display.print("y");display.println(y);
  display.print("x");display.println(x);
  display.print("Zyroskop");display.println("!");
  String test = (String)V +';'+ (String)O;
  display.print("Test: "); display.println(test);
  if((V>-100)&&(V<100)&&(O>-100)&&(O<100)){
    if((V>-45)&&(V<45)){
      V=45;
    }
    if((V>-10)&&(V<10)){
      V=0;
    }
    String MQTT = (String)V +';'+ (String)O;
    long now = millis();
    if (now - lastMsg > SEND_FR) {
      lastMsg = now;
      client.publish("MQTT_PILOT/CTR",MQTT.c_str()); 
      display.print("SEND: "); display.println(MQTT);
    }
  }
  display.display();
  }

  if((realPOT>33)&&(realPOT<66)){
  display.print("Menu");display.println("!");
  display.print("Tryby"); display.println(":");
  display.print("Przyciski"); display.println(".");
  display.print("Zyroskop"); display.println(".");
  display.print(realPOT); display.println(".");
  display.display();
  }
  analogWrite(LED1_PIN, led_Val);
  delay(400);
}
