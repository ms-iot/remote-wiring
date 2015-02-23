
#include "pch.h"
#include "RemoteWiring.h"

using namespace Wiring;

RemoteWiring::RemoteWiring(
    Serial::BluetoothSerial ^bt_serial_
) :
    _firmata(ref new Firmata::UAPFirmataClient)
{
    _firmata->begin(bt_serial_);

    //TODO: Initialize from Firmata
    for ( int i = 0; i < sizeof(_digital_port); ++i ) { _digital_port[i] = 0; }
}

RemoteWiring::RemoteWiring(
    Firmata::UAPFirmataClient ^firmata_
) :
    _firmata(firmata_)
{
    //TODO: Initialize from Firmata
    for ( int i = 0; i < sizeof(_digital_port); ++i ) { _digital_port[i] = 0; }
}

uint16_t
RemoteWiring::analogRead(
byte pin_
) {
    return 0;
}

void
RemoteWiring::analogWrite(
byte pin_,
uint16_t value_
) {
    _firmata->sendAnalog(pin_, value_);
}

uint16_t
RemoteWiring::digitalRead(
byte pin_
) {
    return 0;
}

void
RemoteWiring::digitalWrite(
byte pin_,
byte value_
) {
    const int port = (pin_ / 8);
    const byte port_mask = (1 << (pin_ % 8));

    if ( value_ ) {
        _digital_port[port] |= port_mask;
    } else {
        _digital_port[port] &= !port_mask;
    }

    _firmata->sendDigitalPort(port, static_cast<uint16_t>(_digital_port[port]));
}

void
RemoteWiring::pinMode(
byte pin_,
byte mode_
) {
}
