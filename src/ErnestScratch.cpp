#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <MCP23017.h>
#include <SPI.h>
#include <Wire.h>


#define MCP23017_ADDR 0x20

int status = WL_IDLE_STATUS;

char ssid[] = ""; //  your network SSID (name)
char pass[] = ""; // your network password
char adafruitUsername[] = "mendozamartin";
char adafruitKey[] = "aio_Siho87ybOa5o0GQX2Pd8Uhesr3m4";
char server[] = "io.adafruit.com";
int port = 1883;

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

  // Join I2C bus - more on this in later chapter 8)
  Wire.begin();
  // Initialize MCP
  mcp.init();
  // Port A as Output (LEDS) || Port B as Input (Switches & Buttons)
  mcp.portMode(MCP23017Port::A, 0);                             // 0 = Pin is configured as an output.
  mcp.portMode(MCP23017Port::B, 0b11111111);              // 1 = Pin is configured as an input.
  // Reset Port A & B
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

  Serial.begin(9600); /*
  // Connect to Wi-Fi network with SSID and password
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
*/
  mcp.pinMode(12, INPUT_PULLUP, true);    // sets the digital pin input
}

int soapCount = 10; // variables for my soap thing goes hereeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
int butVal = 0;
int butValPrev = 0;
int soapUsers = 0;

void loop() // mainloop here
{
  butVal = mcp.digitalRead(12);

  if ((butVal == 1) && (butValPrev == 0)) {
    if (soapCount >0){
      soapCount--; 
      soapUsers++;
      Serial.print("soap level is currently: "); 
      Serial.println(soapCount);
    }
    else {
      Serial.print("soap level is currently: ");
      Serial.print(soapCount); 
      Serial.println("   dat shii eMPTYY");
    }
  }
  
  butValPrev = butVal;

  /*
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    reconnect();
  }
  mqttClient.loop(); */
  //delay(1000); 
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

  boolean isSubscribed = mqttClient.subscribe("mendozamartin/feeds/outlet-valve", 1);
  boolean isSubscribed2 = mqttClient.subscribe("mendozamartin/feeds/inlet-valve", 1);

  if (isSubscribed)
  {
    Serial.println("MQTT client is subscribed to topic");
  }
  else
  {
    Serial.println("MQTT client is not subscribed to topic");
  }
}