#pragma once

#include "IRfidService.h"
#include "../config/PinConfig.h"
#include <MFRC522.h>

class RfidService : public IRfidService
{
private:
    MFRC522 reader;

public:
    RfidService();

    void begin() override;
    bool isCardPresent() override;
    String readUid() override;
};
