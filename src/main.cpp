// Acadia University - COMP 5163
//
// Lab Application - Web Server
//
// I, Shahbaz Singh, 0263901s certify that this program is my
// own work and has not been copied from any other source.
//
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// access credentials for WiFi network (commented out for submission).
// const char* ssid = "**************";
// const char* password = "***********";

// WiFI server.  Listen on port 80, which is the default TCP port for HTTP
WiFiServer server(80);

// DS18B20 temperature sensor on D3 (same wiring as the Advanced Sensors lab)
OneWire oneWire(D3);
DallasTemperature DS18B20(&oneWire);

// build the HTTP response: headers, then an HTML page with the sensor data
String buildResponse()
{
  // read the temperature
  DS18B20.requestTemperatures();
  float temp = DS18B20.getTempCByIndex(0);

  // time since start-up as HH:MM:SS
  unsigned long seconds = millis() / 1000;
  char uptime[16];
  sprintf(uptime, "%02lu:%02lu:%02lu", seconds / 3600, (seconds / 60) % 60, seconds % 60);

  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: text/html\r\n";
  response += "Connection: close\r\n";
  response += "\r\n";

  response += "<!DOCTYPE html>\n";
  response += "<html>\n";
  response += "<head>\n";
  response += "<title>D1 Mini Web Server</title>\n";
  response += "<meta http-equiv=\"refresh\" content=\"5\">\n";  // reload every 5 seconds
  response += "</head>\n";
  response += "<body>\n";
  response += "<h1>D1 Mini Web Server</h1>\n";
  response += "<p>Shahbaz Singh - 0263901s</p>\n";
  if (temp == DEVICE_DISCONNECTED_C)
  {
    response += "<p>Temperature: sensor not found</p>\n";
  }
  else
  {
    // describe the temperature and pick a colour for it (cold blue to hot red)
    String label;
    String color;
    if (temp < 10)      { label = "Cold!";    color = "blue"; }
    else if (temp < 15) { label = "Cool";     color = "teal"; }
    else if (temp < 25) { label = "Perfect";  color = "green"; }
    else if (temp < 30) { label = "Warm";     color = "orange"; }
    else if (temp < 35) { label = "Hot";      color = "orangered"; }
    else                { label = "Too Hot!"; color = "red"; }

    response += "<p style=\"color:" + color + "\">Temperature: " + String(temp) +
                " &deg;C (" + label + ")</p>\n";
  }
  response += "<p>IP Address: " + WiFi.localIP().toString() + "</p>\n";
  response += "<p>Uptime: " + String(uptime) + "</p>\n";
  response += "</body>\n";
  response += "</html>\n";

  return response;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\nWeb Server Demo");

  // start the temperature sensor
  DS18B20.begin();

  Serial.printf("\nConnecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n",
                WiFi.localIP().toString().c_str());
}

void loop()
{
  WiFiClient client = server.available();

  // wait for a client (web browser) to connect
  if (client)
  {
    Serial.println("\n>> Client connected");
    Serial.println(">> Client request:");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        // print information about the request (browser, OS, etc)
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request
        if (line.length() == 1 && line[0] == '\n')
        {
          // send the HTML page back to the browser
          client.print(buildResponse());
          Serial.println(">> Response sent to client");
          break;
        }
      }
    }

    // keep read client request data (if any).  After that, we can terminate
    // our client connection
    while (client.available())
    {
      client.read();
    }

    // close the connection:
    client.stop();
    Serial.println(">> Client disconnected");
  }
}
