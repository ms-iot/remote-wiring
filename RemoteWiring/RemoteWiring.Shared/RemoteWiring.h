
#pragma once

#include <cstdint>

namespace Wiring {

public ref class RemoteWiring sealed {
  public:
    [Windows::Foundation::Metadata::DefaultOverload]
    RemoteWiring(
        Serial::ISerial ^serial_,
        uint32_t baud_
    );

    RemoteWiring(
        Firmata::UAPFirmataClient ^firmata_
    );

    uint16_t
    analogRead(
        byte pin_
    );

    void
    analogWrite(
        byte pin_,
        uint16_t value_
    );

    uint16_t
    digitalRead(
        byte pin_
    );

    void
    digitalWrite(
        byte pin_,
        byte value_
    );

    void
    pinMode(
        byte pin_,
        byte mode_
    );

  private:
    Firmata::UAPFirmataClient ^_firmata;
    byte _digital_port[16];
};

}
