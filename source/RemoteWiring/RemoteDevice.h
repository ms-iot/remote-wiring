/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#pragma once

#include <cstdint>
#include "TwoWire.h"

namespace Microsoft {
namespace Maker {
namespace RemoteWiring {

public enum class PinMode
{
    INPUT = 0x00,
    OUTPUT = 0x01,
    ANALOG = 0x02,
    PWM = 0x03,
    SERVO = 0x04,
    SHIFT = 0x05,
    I2C = 0x06,
    ONEWIRE = 0x07,
    STEPPER = 0x08,
    ENCODER = 0x09,
    IGNORED = 0x7F,
    TOTAL_PIN_MODES = 0x0B,
};

public enum class PinState
{
    LOW = 0x00,
    HIGH = 0x01,
};

public delegate void DigitalPinUpdatedCallback( uint8_t pin, PinState state );
public delegate void AnalogPinUpdatedCallback( uint8_t pin, uint16_t value );
public delegate void SysexMessageReceivedCallback( uint8_t command, Windows::Storage::Streams::DataReader ^message );
public delegate void StringMessageReceivedCallback( Platform::String ^message );

public ref class RemoteDevice sealed {

    //singleton reference for I2C
    I2c::TwoWire ^_twoWire;

public:
    event DigitalPinUpdatedCallback ^ DigitalPinUpdatedEvent;
    event AnalogPinUpdatedCallback ^ AnalogPinUpdatedEvent;
    event SysexMessageReceivedCallback ^ SysexMessageReceivedEvent;
    event StringMessageReceivedCallback ^ StringMessageReceivedEvent;

    property I2c::TwoWire ^ I2c
    {
        Microsoft::Maker::RemoteWiring::I2c::TwoWire ^ get()
        {
            if( _twoWire == nullptr )
            {
                _twoWire = ref new Microsoft::Maker::RemoteWiring::I2c::TwoWire( _firmata );
            }
            return _twoWire;
        }
    };

    [Windows::Foundation::Metadata::DefaultOverload]
    RemoteDevice(
        Serial::IStream ^serial_connection_
    );


    RemoteDevice(
        Firmata::UwpFirmata ^firmata_
    );

    virtual ~RemoteDevice();

    ///<summary>
    ///Returns the most recently-reported value for the given analog pin.
    ///<para>Analog pins must first be in PinMode.ANALOG before their values will be reported.</para>
    ///</summary>
    uint16_t
    analogRead(
        uint8_t pin_
    );

    ///<summary>
    ///Sets the value of the given pin to the given analog value.
    ///<para>This function should only be called for pins that support PWM. If the given pin is in 
    ///PinMode.OUTPUT, the pin will automatically be changed to PinMode.PWM</para>
    ///</summary>
    void
    analogWrite(
        uint8_t pin_,
        uint16_t value_
    );

    ///<summary>
    ///Returns the most recently-reported value for the given digital pin.
    ///<para>Analog pins must first be in PinMode.INPUT before their values will be reported.</para>
    ///</summary>
    PinState
    digitalRead(
        uint8_t pin_
    );

    ///<summary>
    ///Sets the value of the given pin to the given state.
    ///</summary>
    void
    digitalWrite(
        uint8_t pin_,
        PinState state_
    );

    ///<summary>
    ///Sets the given pin to the given PinMode.
    ///<para>Due to the way that Arduino and Arduino-like devices are engineered, this function is unable to differentiate 
    ///between Digital and Analog pins. Refer to the descriptions below for more information about referring to the correct pin number.</para>
    ///<para>Digital pins are typically zero-indexed. This means that the first digital pin is pin 0, while the last
    ///digital pin is (the number of digital pins) - 1.</para>
    ///<para>Analog pins are numbered starting immediately after the digital pins. This means the first analog pin is indexed as the total
    ///number of digital pins that the device supports.</para>
    ///</summary>
    void
    pinMode(
        uint8_t pin_,
        PinMode mode_
    );

    ///<summary>
    ///Retrieves the mode of the given pin from the cache stored by RemoteDevice class. 
    ///<para>This is not a function you will find in the Arduino API, but is an extremely helpful function 
    ///that allows the RemoteDevice class to maintain consistency with your apps.</para>
    ///</summary>
    PinMode
    getPinMode(
        uint8_t pin_
    );


private:
    //constant members
    static const int MAX_PORTS = 16;
    static const int MAX_PINS = 128;
    static const int ANALOG_PINS = 6;

    //initialization for constructor
    void const initialize();

    //a reference to the UAP firmata interface
    Firmata::UwpFirmata ^_firmata;

    //a mutex for thread safety
    std::mutex _device_mutex;

    //reporting callbacks
    void onDigitalReport( Firmata::CallbackEventArgs ^argv );
    void onAnalogReport( Firmata::CallbackEventArgs ^argv );
    void onSysexMessage( Firmata::SysexCallbackEventArgs ^argv );
    void onStringMessage( Firmata::StringCallbackEventArgs ^argv );

    //maps the given pin number to the correct port and mask
    void getPinMap( uint8_t, int *, uint8_t * );

    //state-tracking cache variables
    uint8_t volatile _subscribed_ports[MAX_PORTS];
    uint8_t volatile _digital_port[MAX_PORTS];
    uint16_t volatile _analog_pins[ANALOG_PINS];
    uint8_t _pin_mode[MAX_PINS];
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
