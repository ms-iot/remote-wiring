
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
    for ( int i = 0; i < sizeof(_pin_mode); ++i ) { _pin_mode[i] = static_cast<byte>(Firmata::FirmataPinMode::OUTPUT); }
}

RemoteWiring::RemoteWiring(
    Firmata::UAPFirmataClient ^firmata_
) :
    _firmata(firmata_)
{
    //TODO: Initialize from Firmata
    for ( int i = 0; i < sizeof(_digital_port); ++i ) { _digital_port[i] = 0; }
    for ( int i = 0; i < sizeof(_pin_mode); ++i ) { _pin_mode[i] = static_cast<byte>(Firmata::FirmataPinMode::OUTPUT); }
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
    if ( _pin_mode[pin_] != static_cast<byte>(Firmata::FirmataPinMode::PWM) ) {
        if ( _pin_mode[pin_] == static_cast<byte>(Firmata::FirmataPinMode::OUTPUT) ) {
            pinMode(pin_, static_cast<byte>(Firmata::FirmataPinMode::PWM));
            _pin_mode[pin_] = static_cast<byte>(Firmata::FirmataPinMode::PWM);
        } else {
            return;
        }
    }

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
    if ( _pin_mode[pin_] != static_cast<byte>(Firmata::FirmataPinMode::OUTPUT) ) {
        if ( _pin_mode[pin_] == static_cast<byte>(Firmata::FirmataPinMode::PWM) ) {
            pinMode(pin_, static_cast<byte>(Firmata::FirmataPinMode::OUTPUT));
            _pin_mode[pin_] = static_cast<byte>(Firmata::FirmataPinMode::OUTPUT);
        } else {
            return;
        }
    }

    if ( value_ ) {
        _digital_port[port] |= port_mask;
    } else {
        _digital_port[port] &= ~port_mask;
    }

    _firmata->sendDigitalPort(port, static_cast<uint16_t>(_digital_port[port]));
}

void
RemoteWiring::pinMode(
    byte pin_,
    byte mode_
) {
    _firmata->write(static_cast<byte>(Firmata::FirmataCommand::SET_PIN_MODE));
    _firmata->write(pin_);
    _firmata->write(mode_);
}
