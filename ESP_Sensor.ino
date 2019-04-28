#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "ssid"
#define STAPSK  "pass"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);

  char * gchar = "<html>"
  "<head>"
   "<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>"
   "<script type=\"text/javascript\">"
   "  google.charts.load('current', {'packages':['gauge']});"
   "  google.charts.setOnLoadCallback(drawChart);"
   
   "  function httpGet(theUrl)"
   "  {"
   "      var xmlHttp = new XMLHttpRequest();"
   "      xmlHttp.open( \"GET\", theUrl, false );"
   "      xmlHttp.send( null );"
   "      return xmlHttp.responseText;"
   "   }"
   
   "  function drawChart() {"
   "    var data = google.visualization.arrayToDataTable(["
   "      ['Label', 'Value'],"
   "      ['Temp', 0],"
   "    ]);"
   
   "    var options = {"
   "      width: 600, height: 600,"
   "      redFrom: 90, redTo: 150,"
   "      yellowFrom:70, yellowTo: 90,"
   "      minorTicks: 5, max: 150"
   "    };"
   
   "    var chart = new google.visualization.Gauge(document.getElementById('chart_div'));"
   "    chart.draw(data, options);"
   
   "    setInterval(function() {" 
   "       var host = window.location.hostname;"  
   "       data.setValue(0, 1, parseInt(httpGet(\"http://\" + host + \"/val1\")));"
   "       chart.draw(data, options);"
   "    }, 10000);"
   "  }"
   " </script>"
   "</head>"
   "<body>"
   "<div id=\"chart_div\" style=\"width: 600px; height: 600px;\"></div>"
   "</body>"
   "</html>";
   
  server.send(strlen(gchar), "text/html", gchar);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

int sensorVal = 0;
int ThermistorPin = A0;
float Vo;
float R1 = 10000;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(A0, INPUT);
  
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/val1", []() {
    Vo = analogRead(A0);
    R2 = R1 * (1023.0 / (float)Vo - 1.0);
    logR2 = log(R2);
    T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    T = T - 273.15;
    T = (T * 9.0)/ 5.0 + 32.0; 
    server.send(4, "text/plain",  String(T));
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  //sensorVal = ;
  MDNS.update();
}
