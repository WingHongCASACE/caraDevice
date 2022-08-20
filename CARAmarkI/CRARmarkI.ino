#include <DHT.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include "epaperCode.h"

#define DHTPIN 12     // what digital pin the DHT22 is conected to
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float t;
float h;
int CO2ppmValue;
int CO2decaystart;
int CO2decayend;
int CO2HalfHourStart;
int CO2HalfHourEnd;
float probValMaskOnH;
float erValMaskOnH;
float probValMaskOffH;
float erValMaskOffH;
float probValMaskOnD;
float erValMaskOnD;
float probValMaskOffD;
float erValMaskOffD;
float airChangeHour = 0.25;
float CO2source;
int occupancy = 10;
double PRMMaskOnH;
double PRMMaskOffH;
double PRMMaskOnD;
double PRMMaskOffD;

#define ADDR_6713  0x15 // default I2C slave address
int co2data [4];

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

const IPAddress server("[IP Adress of the RPi]");
const String url = "[IP Adress of the RPi:8080/calculator/report-json]";
WiFiClient espClient_http;
const char* mqtt_server = "mqtt.cetools.org";
WiFiClient espClient_mqtt;
PubSubClient client(espClient_mqtt);
long lastMsg = 0;
char msg[500];
int value = 0;
unsigned long TimeLogJSONpayload;
unsigned long TimeLogCO2decaystart;
unsigned long TimeLogCO2decayend;
//unsigned long Time6;
unsigned long TimelogCO2HalfHour;
//unsigned long Time8;
float timeDifference;
String OfficeHourThis;
String OfficeHourNext;
bool decayStartSet;
bool masked;
bool hourMode;

Timezone GB;

void setup()
{
  Serial.begin(115200);
  while (!Serial) { }
  Serial.println();
  Serial.println("setup");
  display.init(115200); // enable diagnostic output on Serial
  showBitmapExample();
  Wire.begin ();
  dht.begin();
  startWifi();
  syncDate();
  client.setServer(mqtt_server, 1884);
  client.setBufferSize(512);
  CO2HalfHourStart = readC02();
}

void loop()
{
  if (hour() == 20 && minute() <= 1 && !decayStartSet) {
    CO2decaystart = readC02();
    TimeLogCO2decaystart = millis();
    decayStartSet = true;
    Serial.print("TimeLogCO2decaystart: ");
    Serial.println(TimeLogCO2decaystart);
  }

  if (hour() == 6 && minute() <= 1 && decayStartSet) {
    CO2decayend = readC02();
    TimeLogCO2decayend = millis();
    timeDifference =  (TimeLogCO2decayend - TimeLogCO2decaystart) / 1000 / 3600;
    decayStartSet = false;
    Serial.print("TimeflowCO2decayend: ");
    Serial.println(TimeLogCO2decayend);
    Serial.print("Time difference: ");
    Serial.println(timeDifference);

    if (timeDifference == 0 || CO2decayend < 401 || CO2decaystart < 401)
    { airChangeHour = 2;
      Serial.println("airChangeHour=2");
    }
    else {
      airChangeHour = (-(log(CO2decayend - 400) - log(CO2decaystart - 400))) / timeDifference;
      Serial.print("airchange hour is");
      Serial.println(airChangeHour);
    }
  }

  Serial.println("millis() - TimeLogJSONpayload = ");
  Serial.println(millis() - TimeLogJSONpayload);
  if (millis() - TimeLogJSONpayload >= 60000) {
    if (hour() < 8) {
      OfficeHourThis = String("0" + String(hour() + 1) + ":00");
      OfficeHourNext = String("0" + String(hour() + 2) + ":00");
    }
    else if (hour() == 8) {
      OfficeHourThis = String("0" + String(hour() + 1) + ":00");
      OfficeHourNext = String(String(hour() + 2) + ":00");
    }
    else {
      OfficeHourThis = String(String(hour() + 1) + ":00");
      OfficeHourNext = String(String(hour() + 2) + ":00");
    }
    masked = true;
    hourMode = true;
    String inputMaskOnH = createJSONPayload();
    String responseMaskOnH = makePOSTRequest(inputMaskOnH);
    if (responseMaskOnH.length() > 0) {
      parseMaskOnHResponse(responseMaskOnH);
      Serial.println("makePOSTRequestMaskOnH");
    }
    masked = false;
    String inputMaskOffH = createJSONPayload();
    String responseMaskOffH = makePOSTRequest(inputMaskOffH);
    if (responseMaskOffH.length() > 0) {
      parseMaskOffHResponse(responseMaskOffH);
      Serial.println("makePOSTRequestMaskOffH");
    }
    masked = true;
    hourMode = false;
    String inputMaskOnD = createJSONPayload();
    String responseMaskOnD = makePOSTRequest(inputMaskOnD);
    if (responseMaskOnD.length() > 0) {
      parseMaskOnDResponse(responseMaskOnD);
      Serial.println("makePOSTRequestMaskOnD");
    }
    masked = false;
    String inputMaskOffD = createJSONPayload();
    String responseMaskOffD = makePOSTRequest(inputMaskOffD);
    if (responseMaskOffD.length() > 0) {
      parseMaskOffDResponse(responseMaskOffD);
      Serial.println("makePOSTRequestMaskOffD");
    }
    TimeLogJSONpayload = millis();

    showData("Indoor Environmental Data");
    sendMQTT();
  }

  int CO2 = readC02();
  Serial.print("CO2 Value: ");
  Serial.println(CO2);

  h = dht.readHumidity();
  t = dht.readTemperature();
  Serial.println("Read from DHT sensor!");

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    h = 50;
    t = 24;
    return;
  }


  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");

  Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")


  Serial.println("delay 5s!");
  int infectors = 1;
  Serial.println(infectors);
  float t = 1.0;
  Serial.println(t);
  double qMaskOnH = erValMaskOnH / 50.0 / infectors;
  double qMaskOffH = erValMaskOffH / 50.0 / infectors;
  double qMaskOnD = erValMaskOnD / 50.0 / infectors;
  double qMaskOffD = erValMaskOffD / 50.0 / infectors;
  Serial.println(qMaskOnH);
  Serial.println(qMaskOffH);
  int n = 10;
  Serial.println(n);
  double ff = ((CO2 - 400.0) / 1000000.0) / 0.038;
  Serial.println(ff, 4);
  PRMMaskOnH = (1 - exp(-1.0 * ff * infectors * qMaskOnH * t / n)) * 100.0;
  Serial.println(PRMMaskOnH, 4);
  PRMMaskOffH = (1 - exp(-1.0 * ff * infectors * qMaskOffH * t / n)) * 100.0;
  PRMMaskOnD = (1 - exp(-1.0 * ff * infectors * qMaskOnD * t * 9.0 / n)) * 100.0;
  PRMMaskOffD = (1 - exp(-1.0 * ff * infectors * qMaskOffD * t * 9.0 / n)) * 100.0; delay(5000);
}

int readC02() {
  Serial.println("start read co2");
  Wire.beginTransmission(ADDR_6713);
  Wire.write(0x04); Wire.write(0x13); Wire.write(0x8B); Wire.write(0x00); Wire.write(0x01);
  Wire.endTransmission();
  delay(1000);
  Wire.requestFrom(ADDR_6713, 4);    // request 4 bytes from slave device
  co2data[0] = Wire.read();
  co2data[1] = Wire.read();
  co2data[2] = Wire.read();
  co2data[3] = Wire.read();
  {
    Serial.print("Func code: "); Serial.print(co2data[0], HEX);
    Serial.print(" byte count: "); Serial.println(co2data[1], HEX);
    Serial.print("MSB: 0x");  Serial.print(co2data[2], HEX); Serial.print("  ");
    Serial.print("LSB: 0x");  Serial.print(co2data[3], HEX); Serial.print("  ");
  }
  CO2ppmValue = ((co2data[2] * 0xFF ) + co2data[3]);
  return CO2ppmValue;
  Serial.println("co2 value found");
}

void startWifi() {
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncDate() {
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());
}

void sendMQTT() {
  if (!client.connected()) {
    Serial.println("MQTT not connected!");
    reconnect();
  }

  snprintf (msg, 50, "%.4f", probValMaskOnH);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.4f", probValMaskOffH);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.1f", probValMaskOnD);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.1f", probValMaskOffD);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.4f", PRMMaskOnH);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.4f", PRMMaskOffH);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.4f", PRMMaskOnD);
  client.publish("[mqtt topic]", msg);
  snprintf (msg, 50, "%.4f", PRMMaskOffD);
  client.publish("[mqtt topic]", msg);

  DynamicJsonDocument doc(512);
  Serial.println("preparing mqtt json!");
  doc["probMaskOnH"] = probValMaskOnH;
  doc["erMaskOnH"] = erValMaskOnH;
  doc["probMaskOffH"] = probValMaskOffH;
  doc["erMaskOffH"] = erValMaskOffH;
  doc["probMaskOnD"] = probValMaskOnD;
  doc["erMaskOnD"] = erValMaskOnD;
  doc["probMaskOffD"] = probValMaskOffD;
  doc["erMaskOffD"] = erValMaskOffD;

  String mqttBuffer;
  serializeJson(doc, mqttBuffer);
  Serial.print("json buffer length: ");
  Serial.println(mqttBuffer.length());
  Serial.println(mqttBuffer);
  client.publish("[mqtt topic]", mqttBuffer.c_str());
  Serial.println("mqtt json pushed");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String createJSONPayload() {
  Serial.println("prepare JSONPayload");
  DynamicJsonDocument doc(1024);
  doc["activity_type"] = "office";
  doc["virus_type"] = "SARS_CoV_2_OMICRON";
  doc["calculator_version"] = "4.2";
  doc["location_latitude"]  = 51.50741;
  doc["location_longitude"] = -0.1276987;
  doc["location_name"] = "London, Greater London, England, GBR";
  doc["room_number"] = "One";
  doc["room_volume"] = 148;
  doc["volume_type"] = "room_volume_explicit";
  doc["infected_people"] = 1;
  doc["simulation_name"] = "Test";
  doc["total_people"] = 10;
  doc["ventilation_type"] = "measured_ventilation";
  doc["air_changes"] = airChangeHour;
  doc["humidity"] = h / 100;
  doc["inside_temp"] = t + 273.15;
  doc["mask_wearing_option"] = "mask_off";
  doc["exposed_lunch_option"] = false;
  doc["infected_lunch_option"] = false;

  if (masked) {
    doc["mask_wearing_option"] = "mask_on";
  } else {
    doc["mask_wearing_option"] = "mask_off";
  }
  if (hourMode) {
    doc["exposed_start"] = OfficeHourThis;
    doc["exposed_finish"] = OfficeHourNext;
  } else {
    doc["exposed_start"] = "08:30";
    doc["exposed_finish"] = "17:30";
  }


  String inputMask;
  serializeJson(doc, inputMask);
  return inputMask;
}


String makePOSTRequest(String input) {
  Serial.println("prepare to make mask off request!");
  if (espClient_http.connect(server, 8080)) {
    espClient_http.println("POST /calculator/report-json HTTP/1.1");
    espClient_http.println("User-Agent: Arduino/1.0");
    espClient_http.print("Content-Length: ");
    espClient_http.println(input.length());
    espClient_http.println();
    espClient_http.println(input);
  }
  delay(5000); //Wait for the response - this is important

  String responseHeader = espClient_http.readStringUntil('{'); //The '{' gets discarded
  Serial.println(responseHeader);
  if (responseHeader.length() == 0) {
    return "";
  }

  int index = responseHeader.lastIndexOf(':');
  String content_length = responseHeader.substring(index + 2, index + 5);
  Serial.println(content_length);
  int contentLength = content_length.toInt();

  byte buffer[contentLength];
  char response[contentLength + 1];

  // Read the response payload
  espClient_http.readBytes(buffer, contentLength);

  response[0] = '{'; //replace the '{'

  for (int i = 0; i < contentLength; i++) {
    response[i + 1] = (char)buffer[i];
  }

  response[contentLength + 1] = '\0'; //null terminate the String
  Serial.println(response);
  return response;
}

void parseMaskOffHResponse(String response) {
  DynamicJsonDocument json(150);
  deserializeJson(json, response);

  probValMaskOffH = json["prob_inf"];
  if (probValMaskOffH < 0.0001) probValMaskOffH = 0.0;
  Serial.println(probValMaskOffH);

  erValMaskOffH = json["emission_rate"];
  Serial.println(erValMaskOffH);

  int exposedOccupantsMaskOff = json["exposed_occupants"];
  Serial.println(exposedOccupantsMaskOff);

  float newCasesMaskOff = json["expected_new_cases"];
  Serial.println(newCasesMaskOff);
}


void parseMaskOnHResponse(String response) {
  DynamicJsonDocument json(150);
  deserializeJson(json, response);

  probValMaskOnH = json["prob_inf"];
  if (probValMaskOnH < 0.0001) probValMaskOnH = 0.0;
  Serial.println(probValMaskOnH);

  erValMaskOnH = json["emission_rate"];
  Serial.println(erValMaskOnH);

  int exposedOccupantsMaskOn = json["exposed_occupants"];
  Serial.println(exposedOccupantsMaskOn);

  float newCasesMaskOn = json["expected_new_cases"];
  Serial.println(newCasesMaskOn);
}

void parseMaskOffDResponse(String response) {
  DynamicJsonDocument json(150);
  deserializeJson(json, response);

  probValMaskOffD = json["prob_inf"];
  if (probValMaskOffD < 0.0001) probValMaskOffD = 0.0;
  Serial.println(probValMaskOffD);

  erValMaskOffD = json["emission_rate"];
  Serial.println(erValMaskOffD);

  int exposedOccupantsMaskOff = json["exposed_occupants"];
  Serial.println(exposedOccupantsMaskOff);

  float newCasesMaskOff = json["expected_new_cases"];
  Serial.println(newCasesMaskOff);
}


void parseMaskOnDResponse(String response) {
  DynamicJsonDocument json(150);
  deserializeJson(json, response);

  probValMaskOnD = json["prob_inf"];
  if (probValMaskOnD < 0.0001) probValMaskOnD = 0.0;
  Serial.println(probValMaskOnD);

  erValMaskOnD = json["emission_rate"];
  Serial.println(erValMaskOnD);

  int exposedOccupantsMaskOn = json["exposed_occupants"];
  Serial.println(exposedOccupantsMaskOn);

  float newCasesMaskOn = json["expected_new_cases"];
  Serial.println(newCasesMaskOn);
}

void showData(const char name[])
{
  Serial.println("prepare for show data!");
  const GFXfont* f = &FreeMonoBold9pt7b;
  const GFXfont* j = &FreeMonoBold18pt7b;
  const GFXfont* k = &FreeMono9pt7b;

  uint16_t textU_x = GxEPD_WIDTH * 0.2;
  uint16_t textU_y = GxEPD_HEIGHT * 0.05;
  uint16_t textU_w = 100;
  uint16_t textU_h = 60;
  uint16_t cursortextU_y = textU_y + textU_h - 6;

  uint16_t boxU_x = GxEPD_WIDTH * 0.35;
  uint16_t boxU_y = GxEPD_HEIGHT * 0.45;
  uint16_t boxU_w = 100;
  uint16_t boxU_h = 20;
  uint16_t cursorU_y = boxU_y + boxU_h - 6;

  uint16_t textL_x = GxEPD_WIDTH * 0.2;
  uint16_t textL_y = GxEPD_HEIGHT * 0.5;
  uint16_t textL_w = 100;
  uint16_t textL_h = 60;
  uint16_t cursortextL_y = textL_y + textL_h - 6;

  uint16_t boxL_x = GxEPD_WIDTH * 0.35;
  uint16_t boxL_y = GxEPD_HEIGHT * 0.85;
  uint16_t boxL_w = 100;
  uint16_t boxL_h = 20;
  uint16_t cursorL_y = boxL_y + boxL_h - 6;
  Serial.println("flash epaper!");
  display.setTextColor(GxEPD_BLACK);
  Serial.println("set text color!");
  display.setFont(f);
  Serial.println("set font!");
  Serial.println("show info page!");
  display.drawExampleBitmap(BitmapExample3, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
  Serial.println("show small logo!");
  display.drawLine(GxEPD_WIDTH * .1, GxEPD_HEIGHT * .6, GxEPD_WIDTH, GxEPD_HEIGHT * .6, GxEPD_BLACK);
  display.drawLine(GxEPD_WIDTH * .5, GxEPD_HEIGHT * 0.4, GxEPD_WIDTH * .5, GxEPD_HEIGHT * 0.8, GxEPD_BLACK);
  Serial.println("show line!");
  display.setCursor(textU_x - 40, GxEPD_HEIGHT * 0.1 - 2);
  display.print("Infection Probability of");
  display.setCursor(textU_x - 5, GxEPD_HEIGHT * 0.15);
  display.print("SARS_CoV_2_OMICRON");
  display.setCursor(textU_x + 5, GxEPD_HEIGHT * 0.2 + 2);
  display.print("Using CARA model");
  display.setFont(k);
  display.setCursor(textU_x - 50, cursortextU_y + 15);
  display.print("If staying in this room for");
  display.setRotation(3);
  display.setCursor(130, 10);
  display.print("With");
  display.setCursor(130, 25);
  display.print("Mask");
  display.setCursor(70, 10);
  display.print("No");
  display.setCursor(60, 25);
  display.print("Mask");
  display.setRotation(0);
  display.setCursor(boxU_x - 70, cursorU_y - 35 );
  display.print("Current");
  display.setCursor(boxU_x - 60, cursorU_y - 18);
  display.print("Hour");
  display.setCursor(boxU_x + 120, cursorU_y - 35);
  display.print("Entire");
  display.setCursor(boxU_x + 90, cursorU_y - 18);
  display.print("Working Hours");
  display.setFont(j);
  display.setCursor(boxU_x - 70, cursorU_y + 15);
  display.print(probValMaskOnH);
  display.print(" %");
  display.setCursor(boxU_x + 100, cursorU_y + 15);
  display.print(probValMaskOnD);
  display.print(" %");
  display.setCursor(boxL_x - 70, cursorL_y - 40);
  display.print(probValMaskOffH);
  display.print(" %");
  display.setCursor(boxL_x + 100, cursorL_y - 40);
  display.print(probValMaskOffD);
  display.print(" %");
  display.setFont(k);
  display.setCursor(GxEPD_WIDTH * 0.05, GxEPD_HEIGHT * 0.9);
  display.print("Assumptions: total occupancy (10),");
  display.setCursor(GxEPD_WIDTH * 0.05, GxEPD_HEIGHT * 0.95);
  display.print("infected (1) stays 08:30 - 17:30");
  Serial.println("finish printing!");
  display.update();
  Serial.println("finish update!");
  delay(5000);
  if (PRMMaskOffD != 0) {
    display.fillRect(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_WHITE);
    display.setFont(f);
    Serial.println("set font!");
    Serial.println("show info page!");
    display.drawExampleBitmap(BitmapExample3, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    Serial.println("show small logo!");
    display.drawLine(GxEPD_WIDTH * .1, GxEPD_HEIGHT * .6, GxEPD_WIDTH, GxEPD_HEIGHT * .6, GxEPD_BLACK);
    display.drawLine(GxEPD_WIDTH * .5, GxEPD_HEIGHT * 0.4, GxEPD_WIDTH * .5, GxEPD_HEIGHT * 0.8, GxEPD_BLACK);
    Serial.println("show line!");
    display.setCursor(textU_x - 40, GxEPD_HEIGHT * 0.1 - 2);
    display.print("Infection Probability of");
    display.setCursor(textU_x - 5, GxEPD_HEIGHT * 0.15);
    display.print("SARS_CoV_2_OMICRON");
    display.setCursor(textU_x - 68, GxEPD_HEIGHT * 0.2 + 2);
    display.print("Using Rudnick & Milton's model");
    display.setFont(k);
    display.setCursor(textU_x - 50, cursortextU_y + 15);
    display.print("If staying in this room for");
    display.setRotation(3);
    display.setCursor(130, 10);
    display.print("With");
    display.setCursor(130, 25);
    display.print("Mask");
    display.setCursor(70, 10);
    display.print("No");
    display.setCursor(60, 25);
    display.print("Mask");
    display.setRotation(0);
    //  display.setCursor(boxU_x-70, cursorU_y - 35 );
    //  display.print("Current");
    display.setCursor(boxU_x - 60, cursorU_y - 18);
    display.print("1 Hour");
    //  display.setCursor(boxU_x+120, cursorU_y - 35);
    //  display.print("Entire");
    display.setCursor(boxU_x + 90, cursorU_y - 18);
    display.print("9 Hours");
    display.setFont(j);
    display.setCursor(boxU_x - 70, cursorU_y + 15);
    display.print(PRMMaskOnH);
    display.print(" %");
    display.setCursor(boxU_x + 100, cursorU_y + 15);
    display.print(PRMMaskOnD);
    display.print(" %");
    display.setCursor(boxL_x - 70, cursorL_y - 40);
    display.print(PRMMaskOffH);
    display.print(" %");
    if (PRMMaskOffD >= 10)
    {
      display.setCursor(boxL_x + 80, cursorL_y - 40);
    }
    else {
      display.setCursor(boxL_x + 100, cursorL_y - 40);
    }
    display.print(PRMMaskOffD);
    display.print(" %");
    display.setFont(k);
    display.setCursor(GxEPD_WIDTH * 0.05, GxEPD_HEIGHT * 0.9);
    display.print("Assumptions: total occupancy (10),");
    display.setCursor(GxEPD_WIDTH * 0.05, GxEPD_HEIGHT * 0.95);
    display.print("infected (1)");
    Serial.println("finish printing!");
    display.update();
  }
  Serial.println("finish show data!");
}
