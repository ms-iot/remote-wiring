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

#include "pch.h"
#include "RemoteDevice.h"

using namespace Concurrency;

using namespace Microsoft::Maker;
using namespace Microsoft::Maker::RemoteWiring;

//******************************************************************************
//* Constructors / Destructors
//******************************************************************************

RemoteDevice::RemoteDevice(
    Serial::IStream ^serial_connection_
    ) :
    _firmata( ref new Firmata::UwpFirmata ),
    _twoWire( nullptr )
{
    initialize();
    _firmata->begin( serial_connection_ );
    _firmata->startListening();
}

RemoteDevice::RemoteDevice(
    Firmata::UwpFirmata ^firmata_
    ) :
    _firmata( firmata_ ),
    _twoWire( nullptr )
{
    initialize();
}

RemoteDevice::~RemoteDevice()
{
    _firmata->finish();
}


//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
RemoteDevice::analogRead(
    uint8_t pin_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::mutex> lock( _device_mutex );

    uint16_t val = -1;

    if (_pin_mode[pin_] != static_cast<uint8_t>(PinMode::ANALOG)) {
        if (_pin_mode[pin_] == static_cast<uint8_t>(PinMode::INPUT)) {
            pinMode(pin_, PinMode::ANALOG);
            _pin_mode[pin_] = static_cast<uint8_t>(PinMode::ANALOG);
        }
        else {
            return static_cast<uint16_t>(val);
        }
    }

    if( pin_ < MAX_PINS )
    {
        val = _analog_pins[ pin_ ];
    }

    return val;
}

void
RemoteDevice::analogWrite(
    uint8_t pin_,
    uint16_t value_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::mutex> lock( _device_mutex );

    if( _pin_mode[ pin_ ] != static_cast<uint8_t>( PinMode::PWM ) ) {
        if( _pin_mode[ pin_ ] == static_cast<uint8_t>( PinMode::OUTPUT ) ) {
            pinMode( pin_, PinMode::PWM );
            _pin_mode[ pin_ ] = static_cast<uint8_t>( PinMode::PWM );
        }
        else {
            return;
        }
    }

    _firmata->sendAnalog( pin_, value_ );
}


PinState
RemoteDevice::digitalRead(
    uint8_t pin_
    )
{
    int port;
    uint8_t port_mask;
    getPinMap( pin_, &port, &port_mask );

    {   //critial section
        std::lock_guard<std::mutex> lock( _device_mutex );
        if( _pin_mode[pin_] != static_cast<uint8_t>( PinMode::INPUT ) ) {
            if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::ANALOG ) ) {
                pinMode( pin_, PinMode::INPUT );
            }
        }

        return static_cast<PinState>( ( _digital_port[port] & port_mask ) > 0 );
    }
}


void
RemoteDevice::digitalWrite(
    uint8_t pin_,
    PinState state_
    )
{
    int port;
    uint8_t port_mask;
    getPinMap( pin_, &port, &port_mask );

    {   //critial section
        std::lock_guard<std::mutex> lock( _device_mutex );
        if( _pin_mode[pin_] != static_cast<uint8_t>( PinMode::OUTPUT ) ) {
            if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::PWM ) ) {
                pinMode( pin_, PinMode::OUTPUT );
            }
            else {
                return;
            }
        }

        if( static_cast<uint8_t>( state_ ) ) {
            _digital_port[port] |= port_mask;
        }
        else {
            _digital_port[port] &= ~port_mask;
        }

        _firmata->sendDigitalPort( port, _digital_port[ port ] );
    }
}


PinMode
RemoteDevice::getPinMode(
    uint8_t pin_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::mutex> lock( _device_mutex );
    return static_cast<PinMode>( _pin_mode[ pin_ ] );
}


void
RemoteDevice::pinMode(
    uint8_t pin_,
    PinMode mode_
    )
{
    int port;
    uint8_t port_mask;
    getPinMap( pin_, &port, &port_mask );

    {   //critial section
        std::lock_guard<std::mutex> lock( _device_mutex );
        _firmata->lock();
        _firmata->write( static_cast<uint8_t>( Firmata::Command::SET_PIN_MODE ) );
        _firmata->write( pin_ );
        _firmata->write( static_cast<uint8_t>( mode_ ) );

        //lets subscribe to this port if we're setting it to input
        if( mode_ == PinMode::INPUT )
        {
            _subscribed_ports[port] |= port_mask;
            _firmata->write( static_cast<uint8_t>( Firmata::Command::REPORT_DIGITAL_PIN ) | ( port & 0x0F ) );
            _firmata->write( _subscribed_ports[port] );
        }
        //if the selected mode is NOT input and we WERE subscribed to it, unsubscribe
        else if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::INPUT ) )
        {
            //make sure we aren't subscribed to this port
            _subscribed_ports[port] &= ~port_mask;
            _firmata->write( static_cast<uint8_t>( Firmata::Command::REPORT_DIGITAL_PIN ) | ( port & 0x0F ) );
            _firmata->write( _subscribed_ports[port] );
        }
        _firmata->flush();
        _firmata->unlock();

        //if the pin mode is being set to output, and it isn't already in output mode, the pin value is set to 0
        if( mode_ == PinMode::OUTPUT && _pin_mode[pin_] != static_cast<uint8_t>( PinMode::OUTPUT ) )
        {
            _digital_port[port] &= ~port_mask;
        }

        //finally, update the cached pin mode
        _pin_mode[pin_] = static_cast<uint8_t>( mode_ );
    }
}


//******************************************************************************
//* Callbacks
//******************************************************************************


void
RemoteDevice::onDigitalReport(
    Firmata::CallbackEventArgs ^args
    )
{
    uint8_t port = args->getPort();
    uint8_t port_val = static_cast<uint8_t>(args->getValue());
    uint8_t port_xor;

    {   //critial section
        std::lock_guard<std::mutex> lock( _device_mutex );
        //output_state will only set bits which correspond to output pins that are HIGH
        uint8_t output_state = ~_subscribed_ports[port] & _digital_port[port];
        port_val |= output_state;

        //determine which pins have changed
        port_xor = port_val ^ _digital_port[port];

        //update the cache
        _digital_port[port] = port_val;
    }

    //throw a pin event for each pin that has changed
    uint8_t i = 0;
    while( port_xor > 0 )
    {
        if( port_xor & 0x01 )
        {
            DigitalPinUpdatedEvent( ( port * 8 ) + i, ( ( port_val >> i ) & 0x01 ) > 0 ? PinState::HIGH : PinState::LOW );
        }
        port_xor >>= 1;
        ++i;
    }
}


void
RemoteDevice::onAnalogReport(
    Firmata::CallbackEventArgs ^args
    )
{
    uint8_t pin = args->getPort();
    uint16_t val = args->getValue();

    {   //critial section
        std::lock_guard<std::mutex> lock( _device_mutex );
        _analog_pins[pin] = val;
    }

    //throw an event for the pin value update
    AnalogPinUpdatedEvent( pin, val );
}


void
RemoteDevice::onSysexMessage(
    Firmata::SysexCallbackEventArgs ^argv
    )
{
    SysexMessageReceivedEvent( argv->getCommand(), Windows::Storage::Streams::DataReader::FromBuffer( argv->getDataBuffer() ) );
}


void
RemoteDevice::onStringMessage(
    Firmata::StringCallbackEventArgs ^argv
    )
{
    StringMessageReceivedEvent( argv->getString() );
}


//******************************************************************************
//* Private Methods
//******************************************************************************

void const
RemoteDevice::initialize(
    void
    )
{
    _firmata->DigitalPortValueEvent += ref new Firmata::CallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::CallbackEventArgs^ args ) -> void { onDigitalReport( args ); } );
    _firmata->AnalogValueEvent += ref new Firmata::CallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::CallbackEventArgs^ args ) -> void { onAnalogReport( args ); } );
    _firmata->SysexEvent += ref new Firmata::SysexCallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::SysexCallbackEventArgs^ args ) -> void { onSysexMessage( args ); } );
    _firmata->StringEvent += ref new Firmata::StringCallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::StringCallbackEventArgs^ args ) -> void { onStringMessage( args ); } );

    memset( (void*)_digital_port, 0, sizeof( _digital_port ) );
    memset( (void*)_subscribed_ports, 0, sizeof( _subscribed_ports ) );
    memset( (void*)_analog_pins, 0, sizeof( _analog_pins ) );
    memset( (void*)_pin_mode, static_cast<uint8_t>( PinMode::OUTPUT ), sizeof( _pin_mode ) );
}

void
RemoteDevice::getPinMap(
    uint8_t pin,
    int *port,
    uint8_t *port_mask
    )
{
    if( port != nullptr )
    {
        *port = ( pin / 8 );
    }
    if( port_mask != nullptr )
    {
        *port_mask = ( 1 << ( pin % 8 ) );
    }
}