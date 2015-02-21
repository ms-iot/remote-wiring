#pragma once

#include <cstdint>

namespace Wiring {
namespace Serial {

public interface struct ISerial
{
    virtual
    uint16_t
    available(
        void
    ) = 0;

    virtual
    void
    begin(
        uint32_t baud_,
        uint8_t config_
    ) = 0;

    virtual
    uint16_t
    read(
        void
    ) = 0;

    virtual
    uint32_t
    write(
        uint8_t c_
    ) = 0;
};

}
}
