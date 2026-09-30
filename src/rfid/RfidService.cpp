#include "RfidService.h"

#include <SPI.h>

RfidService::RfidService()
    : reader(PinConfig::Rfid::SS, PinConfig::Rfid::RST)
{
}

void RfidService::begin()
{
    SPI.begin(
        PinConfig::Rfid::SCK,
        PinConfig::Rfid::MISO,
        PinConfig::Rfid::MOSI,
        PinConfig::Rfid::SS);

    reader.PCD_Init();
}

bool RfidService::isCardPresent()
{
    return reader.PICC_IsNewCardPresent() &&
           reader.PICC_ReadCardSerial();
}

String RfidService::readUid()
{
    String uid;

    for (byte i = 0; i < reader.uid.size; i++)
    {
        if (reader.uid.uidByte[i] < 0x10)
        {
            uid += "0";
        }

        uid += String(reader.uid.uidByte[i], HEX);

        if (i < reader.uid.size - 1)
        {
            uid += ":";
        }
    }

    uid.toUpperCase();

    reader.PICC_HaltA();
    reader.PCD_StopCrypto1();

    return uid;
}
