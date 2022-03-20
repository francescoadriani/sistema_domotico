#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
SSD1306Wire display(0x3c, D2, D1, GEOMETRY_64_48);   // ADDRESS, SDA, SCL  -  SDA and SCL usually populate automatically based on your board's pins_arduino.h
#define MYSQL_DEBUG_PORT      Serial

#define _MYSQL_LOGLEVEL_      1
#include <MySQL_Generic.h>
#define USING_HOST_NAME     false

#if USING_HOST_NAME
  // Optional using hostname, and Ethernet built-in DNS lookup
  char server[] = "your_account.ddns.net"; // change to your server's hostname/URL
#else
  IPAddress server(10, 2, 238, 105);
#endif
uint16_t server_port = 3306;    //3306;

char default_database[] = "historian";           //"test_arduino";
char default_table[]    = "history";          //"test_arduino";

MySQL_Connection conn((Client *)&client);

MySQL_Query *query_mem;

const char* ssid = "ASUS_RiceWLan";
const char* password = "pippoplutopaperinominnie";
const char* mqtt_server = "10.2.238.105";

WiFiClient espClient;
PubSubClient mqttClient(espClient);
String clientId = "DisplayOnLivingRoom";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
float value;


void runInsert(String topic, String payload, String id)
{
  // Initiate the query class instance
  MySQL_Query query_mem = MySQL_Query(&conn);

  if (conn.connected())
  {
    // Sample query
    String INSERT_SQL = String("INSERT INTO ") + default_database + "." + default_table 
                 + " (name, `data`, `timestamp`, um, topic, senderId) VALUES ('temperature','" + payload + "',CURRENT_TIMESTAMP,'°C','" + topic + "','" + id + "')";
    MYSQL_DISPLAY(INSERT_SQL);
    
    // Execute the query
    // KH, check if valid before fetching
    if ( !query_mem.execute(INSERT_SQL.c_str()) )
    {
      MYSQL_DISPLAY("Insert error");
    }
    else
    {
      MYSQL_DISPLAY("Data Inserted.");
    }
  }
  else
  {
    MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}


void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg="";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    msg = msg + (char)payload[i];
  }
  Serial.println();

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(32, 0, "Temp. (°C):");
  display.drawString(32, 10, msg);
  // write the buffer to the display
  display.display();

  if (conn.connectNonBlocking(server, server_port,"user","password") != RESULT_FAIL) 
  {
    delay(500);
    runInsert(String(topic), msg, clientId);
    conn.close();                     // close the connection
  } 
  else 
  {
    MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      mqttClient.subscribe("home.livingroom.temperature");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  delay(500);
}
