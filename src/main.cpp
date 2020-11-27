#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

int status = WL_IDLE_STATUS;

char ssid[] = ""; //  your network SSID (name)
char pass[] = ""; // your network password
char adafruitUsername[] = "mendozamartin";
char adafruitKey[] = "";
char server[] = "io.adafruit.com";
int port = 1883;

void callback(char *topic, byte *payload, unsigned int length);
void connectToMQTT();
void reconnect();

WiFiClient client;
PubSubClient mqttClient(server, port, callback, client);

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
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
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (!mqttClient.connected())
  {
    reconnect();
  }

  mqttClient.loop();
  delay(1000);
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