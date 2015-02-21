#pragma once

#include "ISerial.h"

#include <cstdint>

namespace Wiring {
namespace Serial {

public ref class BluetoothSerial sealed : public ISerial
{
public:
    BluetoothSerial();

    virtual
    uint16_t
    available (
        void
    );

    virtual
    void
    begin (
        uint32_t baud_,
        uint8_t config_
    );

    virtual
    uint16_t
    read (
        void
    );

    virtual
    uint32_t
    write (
        uint8_t c_
    );
};

}
}
