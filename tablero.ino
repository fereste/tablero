#include "Arduino.h"
#include <SPI.h>
#include "mcp_can.h"
#define BUFSIZE 10

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

void setup()
{
    Serial.begin(115200);

    while (CAN_OK != CAN.begin(CAN_125KBPS, MCP_8MHz))
    {
        Serial.println("CAN BUS Init Failed");
        delay(100);
    }
    Serial.println("CAN BUS Init OK!");
}

void loop()
{
    // byte payload[] = {
    //     0x00, // RPM (16 bits)
    //     0xFF, // RPM
    //     0x00,
    //     0x00,
    //     46, // Velocidad * 2.53
    //     96, // Velocidad * 0.01019
    //     0x00,
    //     0x00
    // };
    // CAN.sendMsgBuf(0x201, 0, 8, payload);

    // byte payload2[] = {
    //     130, // Temperatura - 40°C. Rango: -40 a 214 (0x00 a 0xFE)
    //     0x00,
    //     0x00,
    //     0x00,
    //     0x00, // Luz de motor
    //     0x00, // Luz de aceite
    //     0x00,
    //     0x00
    // };
    // CAN.sendMsgBuf(0x420, 0, 8, payload2);

    //0x2a0 iluminación agujas
    //sendPayload(0x2a0, 0, 0, 0, 0, 0, 0, 0, 0);

    //0x2a1 iluminación numeros
    //sendPayload(0x2a1, 0, 0, 0, 0, 0, 0, 0, 0);

    // 0x265 iluminacion externa
    // sendPayload(
    //     0x265,
    //     0b01100000, // segundo bit luz derecha, 3er bit luz izquierda
    //     0b00100000, // tercer bit tulilun
    //     0x00,
    //     0x00,
    //     0b00000110, // 6to bit rompenieblas delantero, 7mo bit rompenieblas trasero
    //     0x00,
    //     0x00,
    //     0x00
    // );

    // //0x433 luces altas y puerta abierta
    // sendPayload(
    //     0x433,
    //     0b00000010, // 7mo bit, puertas abiertas
    //     0x00,
    //     0x00,
    //     0b01000000, // 2do bit, luces altas
    //     0x00,
    //     0x00,
    //     0x00,
    //     0b00000001 // 8vo bit, luces delanteras (posición o bajas)
    // );

    // 0xc34 luces altas y puerta abierta
    // sendPayload(
    //     0xc34,
    //     0xff,
    //     0xff,
    //     0xff,
    //     0xff,
    //     0xff,
    //     0xff,
    //     0xff,
    //     0xff
    // );

    //sendFromSerial();
    // scanBus();
    // sniffBus();

    delay(16);
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
        Serial.print("(");
        Serial.print(len);
        Serial.print(")   ");

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
