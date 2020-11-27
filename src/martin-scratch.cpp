#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MCP23017.h>
#include <Wire.h>
int status = WL_IDLE_STATUS;

char ssid[] = "martin06m"; //  your network SSID (name)
char pass[] = "mathea06m"; // your network password
char adafruitUsername[] = "mendozamartin";
char adafruitKey[] = "aio_XAvI08MHCVVq60Qjz8sUceF3OicR";
char server[] = "io.adafruit.com";
int port = 1883;
int pushButton;

int vacancy = 1;
int vacancyTemp = vacancy;

int isWashingHands = 0;
int isWashingHandsTemp = isWashingHands;

int washCount = 0;
int userCount = 0;
int improperWashCount = 0;

int soapCount = 10; // variables for my soap thing goes hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
int butVal = 0;
int butValPrev = 0;
int soapUsers = 0;

unsigned long startTime;
int timeElapsed;

// Defining the I2C Address of the MCP
#define MCP23017_ADDR 0x20
//#define Band 915E6 ////433E6 for Asia, 866E6 for Europe, 915E6 for North America
// Define an instance of the MCP23017 class
MCP23017 mcp = MCP23017(MCP23017_ADDR);

void portMode(MCP23017Port port, uint8_t directions, uint8_t pullups = 0xFF, uint8_t inverted = 0x00);
void writeRegister(MCP23017Register reg, uint8_t value);
void callback(char *topic, byte *payload, unsigned int length);
void connectToMQTT();
void reconnect();

WiFiClient client;
PubSubClient mqttClient(server, port, callback, client);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  // // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(2000);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Wire.begin();
  mcp.init();
  // Port A as Output (LEDS) || Port B as Input (Switches & Buttons)
  mcp.portMode(MCP23017Port::A, 0);          // 0 = Pin is configured as an output.
  mcp.portMode(MCP23017Port::B, 0b11111111); // 1 = Pin is configured as an input.
  // Reset Port A & B
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); //Reset port A
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); //Reset port B

  mcp.pinMode(8, INPUT_PULLUP);
  mcp.pinMode(9, INPUT_PULLUP);
}

void loop()
{
  char msgBuffer[50]; // make sure this is big enough to hold your string

  // // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    reconnect();
  }

  mqttClient.loop();
  vacancy = mcp.digitalRead(8); // read vacancy switch on pin 8
  if (vacancy == 1 && vacancyTemp == 0)
  {
    userCount += 1;
    mqttClient.publish("mendozamartin/feeds/smart-sink.person-enters", dtostrf(userCount, 6, 2, msgBuffer));
    mqttClient.publish("mendozamartin/feeds/smart-sink.vacancy", dtostrf(vacancy, 6, 2, msgBuffer));

    Serial.println("Someone entered the washroom");
  }
  else if (vacancy == 0 && vacancyTemp == 1)
  {
    mqttClient.publish("mendozamartin/feeds/smart-sink.vacancy", dtostrf(vacancy, 6, 2, msgBuffer));
    Serial.println("Someone has left the washroom");
  }
  vacancyTemp = vacancy;

  isWashingHands = mcp.digitalRead(9); // read hand washing switch on pin 9
  if (isWashingHands == 1 && isWashingHandsTemp == 0)
  {
    washCount += 1;
    Serial.println("User started to wash their hands");
    mqttClient.publish("mendozamartin/feeds/smart-sink.wash-hands", dtostrf(washCount, 6, 2, msgBuffer));

    startTime = millis();
    while (isWashingHands == 1)
    {
      isWashingHands = mcp.digitalRead(9); // read hand washing switch on pin 9
    }
    timeElapsed = (millis() - startTime) / 1000;
    Serial.println(timeElapsed);
    mqttClient.publish("mendozanmartin/feeds/smart-sink.amount-washed", dtostrf(timeElapsed, 6, 2, msgBuffer));
    Serial.println("User has stopped washing their hands.");

    delay(250);
    if (timeElapsed < 20)
    {
      improperWashCount += 1;
      mqttClient.publish("mendozanmartin/feeds/smart-sink.improper-wash", dtostrf(improperWashCount, 6, 2, msgBuffer));
      Serial.println("User has not washed hands enough times.");
    }
  }

  isWashingHandsTemp = isWashingHands;

  butVal = mcp.digitalRead(12);

  if ((butVal == 1) && (butValPrev == 0))
  {
    if (soapCount > 0)
    {
      soapCount--;
      soapUsers++;
      Serial.print("soap level is currently: ");
      mqttClient.publish("mendozanmartin/feeds/smart-sink.soap-level", dtostrf(soapCount, 6, 2, msgBuffer));
      mqttClient.publish("mendozanmartin/feeds/smart-sink.use-soap", dtostrf(soapUsers, 6, 2, msgBuffer));
      Serial.println(soapCount);
    }
    else
    {
      Serial.print("soap level is currently: ");
      Serial.print(soapCount);
      Serial.println("   dat shii eMPTYY");
    }
  }

  butValPrev = butVal;
  delay(250);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
}

void reconnect()
{
  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    connectToMQTT();
  }
  //  return mqttClient.connected();
}

void connectToMQTT()
{
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);
  if (mqttClient.connect(clientId.c_str(), adafruitUsername, adafruitKey))
  {
    //  if (mqttClient.connect(clientId.c_str())) {
    Serial.print("Connected to: ");
    Serial.println(server);
  }
  else
  {
    Serial.print("failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" try again in 2 seconds");
    delay(2000);
  }

  // boolean isSubscribed = mqttClient.subscribe("mendozamartin/feeds/outlet-valve", 1);
  // boolean isSubscribed2 = mqttClient.subscribe("mendozamartin/feeds/inlet-valve", 1);

  // if (isSubscribed)
  // {
  //   Serial.println("MQTT client is subscribed to topic");
  // }
  // else
  // {
  //   Serial.println("MQTT client is not subscribed to topic");
  // }
}