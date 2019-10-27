#include "Arduino.h"
#include <SPI.h>
#include "mcp_can.h"

#define BUFSIZE 10
#define RPM_HIGH 0
#define RPM_LOW 1
#define SPEED_HIGH 4
#define SPEED_LOW 5
#define COOLANT_TEMPERATURE 0
#define ENGINE_LIGHT 4
#define OIL_LIGHT 5
#define BKLIGHT_NEEDLE_HIGH 0
#define BKLIGHT_NEEDLE_LOW 1
#define BKLIGHT_NUMBERS_HIGH 4
#define BKLIGHT_NUMBERS_LOW 5
#define BKLIGHT_LCD_HIGH 6
#define BKLIGHT_LCD_LOW 7
#define TURNING_SIGNALS 0
#define WARNING_SOUND 1
#define FOG_LAMPS 4
#define DOORS_OPEN 0
#define HIGHBEAMS 3
#define HEADLAMPS 7

const int spiCSPin = 10;
MCP_CAN CAN(spiCSPin);

uint8_t buff[BUFSIZE];
unsigned long id = 0x300;
uint8_t payload[] = {
    0x20, // RPM
    0xEB, // RPM
    0x00,
    0x00,
    0x65, // Velocidad
    0x00, // Velocidad
    0x00,
    0x00};

byte payload201hs[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte payload420hs[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte payload2a0ms[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte payload2a1ms[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte payload265ms[8] = {0, 0, 0, 0, 0, 0, 0, 0};
byte payload433ms[8] = {0, 0, 0, 0, 0, 0, 0, 0};
String line;

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(75);

    while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_8MHz))
    {
        Serial.println("CAN BUS Init Failed");
        delay(100);
    }
    Serial.println("CAN BUS Init OK!");
}

void loop()
{
    if (Serial.available())
    {
        unsigned long a = millis();

        line = Serial.readStringUntil(0);
        int indexOf = line.indexOf('|');
        rpm(line.substring(0, indexOf).toInt());
        
        line = line.substring(indexOf + 1);
        indexOf = line.indexOf('|');
        speed(line.substring(0, indexOf).toInt());
    
        Serial.println(millis() - a);
    }
    
    // High speed CAN bus
    CAN.sendMsgBuf(0x201, 0, 8, payload201hs);
    CAN.sendMsgBuf(0x420, 0, 8, payload420hs);

    // Mid speed CAN bus
    // CAN.sendMsgBuf(0x2a0, 0, 8, payload2a0ms);
    // CAN.sendMsgBuf(0x2a1, 0, 8, payload2a1ms);
    // CAN.sendMsgBuf(0x265, 0, 8, payload265ms);
    // CAN.sendMsgBuf(0x433, 0, 8, payload433ms);
}

void sendPayload(unsigned long arbId, byte b0, byte b1, byte b2, byte b3, byte b4, byte b5, byte b6, byte b7)
{
    byte payload[] = {b0, b1, b2, b3, b4, b5, b6, b7};
    CAN.sendMsgBuf(arbId, 0, 8, payload);
}

void sendFromSerial()
{
    if (Serial.available())
    {
        Serial.readBytes(buff, BUFSIZE);

        id = buff[0] << 8;
        id += buff[1];
        for (int i = 0; i < 8; i++)
        {
            payload[i] = buff[i + 2];
        }
        Serial.print("id");
        Serial.println(id);
        Serial.println(id, HEX);
    }

    CAN.sendMsgBuf(id, 0, 8, payload);
}

void sniffBus()
{
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive())
    {
        CAN.readMsgBuf(&len, buf);

        unsigned long canId = CAN.getCanId();

        Serial.print(canId, HEX);
        Serial.print(" - ");

        for (int i = 0; i < len; i++)
        {
            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void scanBus()
{
    for (unsigned long id = 0x000; id < 0x800; id++)
    {
        Serial.print("ID 0x");
        Serial.println(id, HEX);

        byte testPayload[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        CAN.sendMsgBuf(id, 0, 8, testPayload);

        delay(25);
    }
}


/*******************************
 * Payload control
*******************************/

void rpm(unsigned int rpm)
{
    byte b0 = (rpm & 0xFF00) >> 8;
    byte b1 = rpm & 0xFF;
    payload201hs[RPM_HIGH] = b0;
    payload201hs[RPM_LOW] = b1;
}

void speed(unsigned int speed)
{
    byte b0 = speed / (7.6 / 3.0);
    byte b1 = (speed - b0) / 0.01019;
    payload201hs[SPEED_HIGH] = b0;
    payload201hs[SPEED_LOW] = b1;
}

void coolantTemperature(int temp)
{
    payload420hs[COOLANT_TEMPERATURE] = temp + 40;
}

void engineLight(bool on)
{
    if (on)
        payload420hs[ENGINE_LIGHT] |= 0xFF;
    else
        payload420hs[ENGINE_LIGHT] &= 0x00;
}

void oilLight(bool on)
{
    if (on)
        payload420hs[OIL_LIGHT] |= 0b00010000;
    else
        payload420hs[OIL_LIGHT] &= 0b11101111;
}

void backlightNeedles(unsigned int brightness)
{
}

void backlightLcd(unsigned int brightness)
{
}

void backlightNumbers(unsigned int brightness)
{
}

void turningSignals(bool left, bool right, bool blink)
{
}

void frontFogLamp(bool on)
{
    if (on)
        payload265ms[FOG_LAMPS] |= 0b00000100;
    else
        payload265ms[FOG_LAMPS] &= 0b11111011;
}

void rearFogLamp(bool on)
{
    if (on)
        payload265ms[FOG_LAMPS] |= 0b00000010;
    else
        payload265ms[FOG_LAMPS] &= 0b11111101;
}

void lights(bool on)
{
    if (on)
        payload433ms[HEADLAMPS] |= 0b00000001;
    else
        payload433ms[HEADLAMPS] &= 0b11111110;
}

void highbeams(bool on)
{
    if (on)
        payload433ms[HIGHBEAMS] |= 0b01000000;
    else
        payload433ms[HIGHBEAMS] &= 0b10111111;
}

void warningSound(bool on)
{
    if (on)
        payload265ms[WARNING_SOUND] |= 0b00100000;
    else
        payload265ms[WARNING_SOUND] &= 0x11011111;
}

void doorsOpenLight(bool on)
{
    if (on)
        payload433ms[DOORS_OPEN] |= 0b00000010;
    else
        payload433ms[DOORS_OPEN] &= 0x11111101;
}
