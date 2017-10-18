#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#define CONTACT D1

const char* ssid = "KrispyNet";
const char* password = "Australia";

char* overwatchTopic = "kolcun/halloween/overwatch";
char controlTopic[50] = "kolcun/halloween/";
char stateTopic[50];
char onlineMessage[50] = "Halloween Controller online - Controller ID: ";
char* server = "54.156.244.62";
char* mqttMessage;
int controllerId;
boolean ledEnabled = true;
char address[4];
char clientId[50] = "halloween-controller-";

WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

void setup() {
  Serial.begin(115200);
  delay(10);

  //char string[1000] = "this is a long string testing", sub[1000];
  //   int position = 9, length = 6, c = 0;
  //
  //   while (c < length) {
  //      sub[c] = string[position+c-1];
  //      c++;
  //   }
  //   sub[c] = '\0';
  //
  //Serial.println(sub);


  //DIP switch
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);

  pinMode(CONTACT, OUTPUT);
  digitalWrite(CONTACT, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  determineControllerId();
  buildControlTopic();
  buildOnlineMessage();
  buildStateTopic();
  connectToWifi();
  Serial.println(clientId);

  pubSubClient.setServer(server, 1883);
  pubSubClient.setCallback(mqttCallback);

  if (!pubSubClient.connected()) {
    reconnect();
  }

  blink3Times();
  blinkId();
}

void reconnect() {
  while (!pubSubClient.connected()) {
    if (pubSubClient.connect("halloween", "kolcun", "MosquittoMQTTPassword$isVeryLong123")) {
      Serial.println("Connected to MQTT broker");
      pubSubClient.publish(overwatchTopic, onlineMessage);
      Serial.print("sub to: '");
      Serial.print(controlTopic);
      Serial.println("'");
      if (!pubSubClient.subscribe(controlTopic, 1)) {
        Serial.println("MQTT: unable to subscribe");
      }
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

}

void loop() {

  if (!pubSubClient.connected()) {
    reconnect();
  }
  pubSubClient.loop();
}

void connectToWifi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("MQTT Message Arrived ["));
  Serial.print(topic);
  Serial.print(F("] "));
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print("length: ");
  Serial.println(length);

  mqttMessage = (char*) payload;

  if (strcmp(topic, controlTopic) == 0) {
    if (strncmp(mqttMessage, "on", length - 1) == 0) {
      Serial.println("Turn contact on");
      turnOn();
    } else if (strncmp(mqttMessage, "off", length - 1) == 0) {
      Serial.println("Turn contact off");
      turnOff();
    } else if (strncmp(mqttMessage, "momentary", 9) == 0) { //just the first part
      Serial.println("Monentary control");
      momentary(String(mqttMessage).substring(9, length).toInt());
    } else if (strncmp(mqttMessage, "ledon", length - 1) == 0) {
      Serial.println("LED enabled");
      ledEnabled = true;
    } else if (strncmp(mqttMessage, "ledoff", length - 1) == 0) {
      Serial.println("LED disabled");
      ledEnabled = false;
    }
  }

}


void momentary(int millis) {
  Serial.print("Momentary on for millis: ");
  Serial.println(millis);
  turnOn();
  delay(millis);
  turnOff();
}

void turnOn() {
  pubSubClient.publish(stateTopic, "on");
  digitalWrite(CONTACT, HIGH);
  Serial.println("close contact");
  if (ledEnabled) {
    turnOnLed();
  }
}

void turnOff() {
  pubSubClient.publish(stateTopic, "off");
  digitalWrite(CONTACT, LOW);
  Serial.println("open contact");
  if (ledEnabled) {
    turnOffLed();
  }
}

void turnOnLed() {
  Serial.println("led on");
  digitalWrite(LED_BUILTIN, LOW);
}

void turnOffLed() {
  Serial.println("led off");
  digitalWrite(LED_BUILTIN, HIGH);
}

void determineControllerId() {
  int id;
  id = (id << 1) | !digitalRead(D5);
  id = (id << 1) | !digitalRead(D6);
  id = (id << 1) | !digitalRead(D7);
  Serial.print("Controller ID : ");
  Serial.println(id);
  controllerId = id;
  itoa(controllerId, address, 10);
  strcat(clientId, address);
}

void buildControlTopic() {
  strcat(controlTopic, address);
}

void buildStateTopic() {
  strcpy(stateTopic, controlTopic);
  strcat(stateTopic, "/state");
}

void buildOnlineMessage() {
  strcat(onlineMessage, address);
}

void blinkId() {
  for (int i = 0; i < controllerId; i++) {
    turnOnLed();
    delay(300);
    turnOffLed();
    delay(200);

  }
}

void blink3Times() {
  for (int i = 0; i < 3; i++) {
    turnOnLed();
    delay(100);
    turnOffLed();
    delay(100);
  }
  delay(3000);

}

