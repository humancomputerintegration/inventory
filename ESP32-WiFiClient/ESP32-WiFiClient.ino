/*
 *  This sketch sends data via HTTP GET requests to 192.168.0.109.
 *
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#define DEBUG_MODE

const char* ssid     = "hci";       //"your-ssid";
const char* password = "mpffmpff";  //"your-password";

//const char* host = "192.168.0.109";
const char* host = "inventory.plopes.org";
const int httpPort = 80;
char* url;

Ticker blinker;
Ticker toggler;
float blinkerPace = 0.1;  //seconds //speed of blinking while send GET to the server
float togglePeriod = 5.0; //seconds //period of blinking to show it alive

// Use WiFiClient class to create TCP connections
WiFiClient client;

struct Button {
    const uint8_t PIN;
    char* url;
};

Button button_decrease = {27, "/decrease-qty/LED"};
Button button_increase = {14, "/increase-qty/LED"};
const uint8_t LED_indicator = 13;
int flag = 0;


void ARDUINO_ISR_ATTR isr(void* arg_url) {
  
    if (flag == 0 ){
      flag = 1;
      url = (char*)arg_url;
    }
    
    else return;
}


void led_blink()
{
  digitalWrite(LED_indicator, !digitalRead(LED_indicator));
}

void led_alive()
{
  digitalWrite(LED_indicator, HIGH);
  delay(100);
  digitalWrite(LED_indicator, LOW);
  delay(100);
  digitalWrite(LED_indicator, HIGH);
  delay(100);
  digitalWrite(LED_indicator, LOW);
  delay(100);
}


void debug_print(char* str){
#ifdef DEBUG_MODE
  Serial.print(str);
#endif
  return;
}


void setup()
{
#ifdef DEBUG_MODE
  Serial.begin(115200);
#endif
  pinMode(LED_indicator, OUTPUT);
  blinker.attach(togglePeriod, led_alive);
  pinMode(button_decrease.PIN, INPUT_PULLUP);
  attachInterruptArg(button_decrease.PIN, isr, button_decrease.url, FALLING);
  pinMode(button_increase.PIN, INPUT_PULLUP);
  attachInterruptArg(button_increase.PIN, isr, button_increase.url, FALLING);
  
  delay(10);

  // We start by connecting to a WiFi network
#ifdef DEBUG_MODE
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
#ifdef DEBUG_MODE
      Serial.print(".");
#endif
  }

#ifdef DEBUG_MODE
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}

int value = 0;

void loop()
{  
  if(flag == 1){
    //digitalWrite(13, HIGH); 
    
    blinker.detach();
    blinker.attach(blinkerPace, led_blink);
    blinkerPace = 0.05;
#ifdef DEBUG_MODE
    Serial.print("connecting to ");
    Serial.println(host);
#endif

    if (!client.connect(host, httpPort)) {
#ifdef DEBUG_MODE
        Serial.println("connection failed");
#endif
        return;
    }

#ifdef DEBUG_MODE
    Serial.print("Requesting URL: ");
    Serial.println(url);
#endif

    // This will send the request to the server
    client.print(String("GET ") + String(url) + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
                 
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 5000) {
#ifdef DEBUG_MODE
            Serial.println(">>> Client Timeout !");
#endif
            client.stop();
            return;
        }
    } 
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
#ifdef DEBUG_MODE
        Serial.print(line);
#endif
    }

#ifdef DEBUG_MODE
    Serial.println();
    Serial.println("closing connection");
    Serial.println();
    Serial.println();
#endif
    flag = 0;
    
    delay(1000);       
    blinker.detach();
    digitalWrite(LED_indicator, LOW);
    blinker.attach(togglePeriod, led_alive);
  }
}
