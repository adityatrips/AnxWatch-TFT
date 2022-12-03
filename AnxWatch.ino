#define WIFI_SSID "ESP8266"
#define WIFI_PASS "ESP8266@PASS"
#define TCAADDR 0x70
#define REPORTING_PERIOD_MS 1000

#include <ESP8266.h>
#include <MAX30100_PulseOximeter.h>
#include <URTouch.h>
#include <UTFT.h>
#include <Wire.h>

UTFT lcd(ILI9341_16, 38, 39, 40, 41);
URTouch touch(6, 5, 4, 3, 2);
PulseOximeter pox;

int x, y;

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

uint32_t tsLastReport = 0;

void TCA9548A(uint8_t bus)
{
    if (bus > 7)
        return;

    Wire.beginTransmission(TCAADDR);
    Wire.write(1 << bus);
    Wire.endTransmission();
}

void onBeatDetected() { Serial.println("Beat!"); }

void setup()
{
    char *ip, ap[31];

    WiFi.reset(WIFI_RESET_HARD);
    WiFi.begin(9600);

    if (WiFi.join(WIFI_SSID, WIFI_PASS) == WIFI_ERR_OK) {
        ip = WiFi.ip(WIFI_MODE_STA);
        if (WiFi.isConnect(ap))
            Serial.println(ap);
    } else {
        while (1)
            ;
    }

    Serial.begin(9600);
    Wire.begin();

    if (!pox.begin()) {
        Serial.println("PulseOximeter failed!");
        for (;;)
            ;
    } else {
        Serial.println("PulseOximeter success!");
    }

    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    pox.setOnBeatDetectedCallback(onBeatDetected);

    for (uint8_t t = 0; t < 8; t++) {
        TCA9548A(t);
        Serial.print("TCA #");
        Serial.println(t);

        for (uint8_t addr = 0; addr <= 127; addr++) {
            if (addr == TCAADDR)
                continue;

            Wire.beginTransmission(addr);
            if (!Wire.endTransmission()) {
                Serial.print("Found I2C 0x");
                Serial.println(addr, HEX);
            }
        }
    }

    lcd.InitLCD(1);
    touch.InitTouch(1);
    touch.setPrecision(PREC_MEDIUM);

    lcd.fillScr(0, 0, 0);
    lcd.setFont(BigFont);
    lcd.setColor(255, 255, 255);
    lcd.setBackColor(0, 0, 0);

    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);

    lcd.clrScr();
    lcd.fillScr(0, 0, 0);

    homeScreen();
}

void loop()
{
    pox.update();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart Rate:  ");
        Serial.print(pox.getHeartRate());
        Serial.println("bpm / SpO2:  ");
        Serial.print(pox.getSpO2());
        Serial.println(" %");
        tsLastReport = millis();
    }

    int x, y;
    if (touch.dataAvailable()) {
        int pg = 1;
        Serial.println(pg);
        touch.read();
        x = touch.getX();
        y = touch.getY();

        if (((x >= 0) && (y <= 40) && (x <= 220) && (y >= 0)) && (pg != 1)) {
            Serial.println("Home");
            pg = 1;
        }

        if (pg == 1) {
            homeScreen();
            if ((x >= 0) && (y <= 230) && (x <= 70) && (y >= 150)) {
                Serial.println("Gyro");
                gyroscopeScreen();
            }
            if ((x >= 71) && (y <= 230) && (x <= 145) && (y >= 150)) {
                Serial.println("Pulse");
                pulseScreen();
            }
            if ((x >= 146) && (y <= 230) && (x <= 230) && (y >= 150)) {
                Serial.println("Alert");
                alertScreen();
            }
        }
    }
}

void homeButton()
{
    lcd.clrScr();
    lcd.setFont(BigFont);
    lcd.setColor(255, 255, 255);
    lcd.fillRect(0, 200, 319, 239);
    lcd.setBackColor(255, 255, 255);
    lcd.setColor(0, 0, 0);
    lcd.print("Home Page", CENTER, 212);
}

void homeScreen()
{
    homeButton();

    lcd.setFont(BigFont);
    lcd.setColor(255, 255, 255);
    lcd.setBackColor(0, 0, 0);

    lcd.setBackColor(255, 255, 255);

    lcd.fillRect(0, 0, 319, 79);
    lcd.fillRect(0, 200, 319, 239);
    lcd.setColor(0, 0, 0);
    lcd.drawLine(106.6, 0, 106.6, 80);

    lcd.setColor(0, 0, 0);
    lcd.printNumI(1, 45.3, 32);
    lcd.printNumI(2, 151.9, 32);
    lcd.setColor(255, 0, 0);
    lcd.print("Alert", 226.5, 32);
    lcd.setColor(0, 0, 0);
    lcd.drawLine(213.3, 0, 213.3, 80);

    lcd.print("Home Page", CENTER, 212);

    lcd.setFont(SmallFont);
    lcd.setBackColor(0x0000);
    lcd.setColor(0xFFFF);
    lcd.print("1 => Anxiety Detection", LEFT + 5, 96);
    lcd.print("2 => Pulse and Blood Oxygen Detection", LEFT + 5, 134);
    lcd.print("3 => Create Alerts", LEFT + 5, 172);
}

void gyroscopeScreen()
{
    homeButton();
    lcd.print("Anxiety Detection", CENTER, 10);
}

void pulseScreen()
{
    homeButton();
    lcd.print("Pulse and Blood", CENTER, 10);
    lcd.print("Oxygen Detection", CENTER, 28);
}

void alertScreen()
{
    homeButton();
    lcd.setBackColor(0, 0, 0);
    lcd.setColor(255, 0, 0);
    lcd.print("Create Alerts", CENTER, 10);

    lcd.setBackColor(255, 0, 0);
    lcd.fillCircle(120, 160, 180);
}
