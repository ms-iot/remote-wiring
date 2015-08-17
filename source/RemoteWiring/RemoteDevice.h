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
#include <mutex>
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
public delegate void RemoteDeviceConnectionCallback();
public delegate void RemoteDeviceConnectionCallbackWithMessage( Platform::String^ message );

public ref class RemoteDevice sealed {

    //singleton reference for I2C
    I2c::TwoWire ^_twoWire;

public:
    event DigitalPinUpdatedCallback ^ DigitalPinUpdated;
    event AnalogPinUpdatedCallback ^ AnalogPinUpdated;
    event SysexMessageReceivedCallback ^ SysexMessageReceived;
    event StringMessageReceivedCallback ^ StringMessageReceived;
    event RemoteDeviceConnectionCallback ^ DeviceReady;
    event RemoteDeviceConnectionCallbackWithMessage ^ DeviceConnectionFailed;
    event RemoteDeviceConnectionCallbackWithMessage ^ DeviceConnectionLost;

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
    ///<param name="analog_pin_">The analog pin string, where "A0" refers to the first analog pin A0, "A1" refers to A1, and so on.</param>
    ///</summary>
    uint16_t
    analogRead(
        Platform::String ^analog_pin_
    );

    ///<summary>
    ///Sets the value of the given pin to the given analog value.
    ///<para>This function should only be called for pins that support PWM. If the given pin is in 
    ///PinMode.OUTPUT, the pin will automatically be changed to PinMode.PWM</para>
    ///<param name="pin_">A raw pin number which will be treated "as is" and used exactly as given.</param>
    ///<param name="value_">The analog value to write to the given pin.</param>
    ///</summary>
    void
    analogWrite(
        uint8_t pin_,
        uint16_t value_
    );

    ///<summary>
    ///Returns the most recently-reported value for the given digital pin.
    ///<para>Analog pins must first be in PinMode.INPUT before their values will be reported.</para>
    ///<param name="pin_">A raw pin number which will be treated "as is" and used exactly as given.</param>
    ///</summary>
    PinState
    digitalRead(
        uint8_t pin_
    );

    ///<summary>
    ///Sets the value of the given pin to the given state.
    ///<param name="pin_">A raw pin number which will be treated "as is" and used exactly as given.</param>
    ///<param name="state_">The desired state for the given pin.</param>
    ///</summary>
    void
    digitalWrite(
        uint8_t pin_,
        PinState state_
    );

    ///<summary>
    ///Sets the given pin to the given PinMode.
    ///<para>This function uses the given pin number "as is". Due to the way that Arduino and Arduino-like devices are engineered, analog pins like "A0"
    ///actually have a raw pin number which will change from board to board. It is recommended that you use the pinMode( String, PinMode ) overload when working with analog pins.</para>
    ///<para>Digital pins are typically zero-indexed. This means that the first digital pin is pin 0, while the last
    ///digital pin is (the number of digital pins) - 1.</para>
    ///<para>Analog pins are numbered starting immediately after the digital pins. This means the first analog pin is indexed as the total
    ///number of digital pins that the device supports. You may also use the pinMode function which accepts a pin number</para>
    ///<param name="pin_">A raw pin number which will be treated "as is" and used exactly as given.</param>
    ///<param name="mode_">The desired mode for the given pin.</param>
    ///</summary>
    [Windows::Foundation::Metadata::DefaultOverload]
    void
    pinMode(
        uint8_t pin_,
        PinMode mode_
    );

    ///<summary>
    ///Sets the given pin to the given PinMode.
    ///<para>This function accepts a string which should always represent an analog pin, like "A0". It will convert the given string to the correct pin number
    ///based on the configuration reported by the device. The given string must be exactly 'A' followed by a number, or the request will be ignored.</para>
    ///<param name="analog_pin_">The analog pin string, where "A0" refers to the first analog pin A0, "A1" refers to A1, and so on.</param>
    ///<param name="mode_">The desired mode for the given analog pin.</param>
    ///</summary>
    void
    pinMode(
        Platform::String ^analog_pin_,
        PinMode mode_
    );

    ///<summary>
    ///Retrieves the mode of the given pin from the cache stored by RemoteDevice class. 
    ///<para>This is not a function you will find in the Arduino API, but is an extremely helpful function 
    ///that allows the RemoteDevice class to maintain consistency with your apps.</para>
    ///<param name="pin_">A raw pin number which will be treated "as is" and used exactly as given.</param>
    ///</summary>
    [Windows::Foundation::Metadata::DefaultOverload]
    PinMode
    getPinMode(
        uint8_t pin_
    );

    ///<summary>
    ///Retrieves the mode of the given pin from the cache stored by RemoteDevice class. 
    ///<para>This is not a function you will find in the Arduino API, but is an extremely helpful function 
    ///that allows the RemoteDevice class to maintain consistency with your apps.</para>
    ///<param name="analog_pin_">The analog pin string, where "A0" refers to the first analog pin A0, "A1" refers to A1, and so on.</param>
    ///</summary>
    PinMode
    getPinMode(
        Platform::String ^analog_pin_
        );


private:
    //constant members
    static const size_t MAX_PORTS = 16;
    static const size_t MAX_PINS = 128;
    static const size_t MAX_ANALOG_PINS = 16;

    //stateful members received from the device
    std::atomic_int _analog_offset;
    std::atomic_int _num_analog_pins;
    std::atomic_int _total_pins;
    std::atomic_bool _initialized;

    //initialization for constructor
    void const initialize();

    //a reference to the UAP firmata interface
    Firmata::UwpFirmata ^_firmata;

    //a mutex for thread safety
    std::recursive_mutex _device_mutex;

    //state-tracking cache variables
    std::array<std::atomic_uint8_t, MAX_PORTS> _subscribed_ports;
    std::array<std::atomic_uint8_t, MAX_PORTS> _digital_port;
    std::array<std::atomic_uint16_t, MAX_ANALOG_PINS> _analog_pins;
    std::array<std::atomic_uint8_t, MAX_PINS> _pin_mode;

    //maps the given pin number to the correct port and mask
    void
    getPinMap(
        uint8_t, int *,
        uint8_t *
    );

    //returns a uint8_t type parsed from a Platform::String ^
    uint8_t
    parsePinFromAnalogString(
        Platform::String^ string_
    );

    //connection callbacks
    void
    onConnectionReady(
        void
    );

    void
    onConnectionFailed(
        Platform::String^ message_
    );

    void
    onConnectionLost(
        Platform::String^ message_
    );

    //reporting callbacks
    void
    onDigitalReport(
        Firmata::CallbackEventArgs ^argv_
    );

    void
    onAnalogReport(
        Firmata::CallbackEventArgs ^argv_
    );

    void
    onSysexMessage(
        Firmata::SysexCallbackEventArgs ^argv_
    );

    void
    onStringMessage(
        Firmata::StringCallbackEventArgs ^argv_
    );

    void
    onPinCapabilityResponseReceived(
        Microsoft::Maker::Firmata::UwpFirmata ^caller_,
        Microsoft::Maker::Firmata::SysexCallbackEventArgs ^argv_
    );
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
