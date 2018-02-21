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
using namespace Microsoft::Maker::Firmata;
using namespace Microsoft::Maker::RemoteWiring;

//******************************************************************************
//* Constructors / Destructors
//******************************************************************************

RemoteDevice::RemoteDevice(
    Serial::IStream ^serial_connection_
    ) :
    _initialized( ATOMIC_VAR_INIT(false) ),
    _firmata( ref new Firmata::UwpFirmata ),
    _twoWire( nullptr ),
    _hardwareProfile( nullptr )
{
    //subscribe to all relevant connection changes from our new Firmata object and then attach the given IStream object
    _firmata->FirmataConnectionReady += ref new Firmata::FirmataConnectionCallback( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionReady );
    _firmata->FirmataConnectionFailed += ref new Firmata::FirmataConnectionCallbackWithMessage( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionFailed );
    _firmata->FirmataConnectionLost += ref new Firmata::FirmataConnectionCallbackWithMessage( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionLost );
    _firmata->begin( serial_connection_ );
}

RemoteDevice::RemoteDevice(
    Firmata::UwpFirmata ^firmata_
    ) :
    _initialized( ATOMIC_VAR_INIT(false) ),
    _firmata( firmata_ ),
    _twoWire( nullptr ),
    _hardwareProfile( nullptr )
{
    //since the UwpFirmata object is provided, we need to lock its state & verify it is not already in a connected state
    _firmata->lock();

    if( _firmata->connectionReady() )
    {
        _firmata->FirmataConnectionLost += ref new Microsoft::Maker::Firmata::FirmataConnectionCallbackWithMessage( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionLost );
        _firmata->unlock();

        onConnectionReady();
    }
    else
    {
        _firmata->FirmataConnectionReady += ref new Microsoft::Maker::Firmata::FirmataConnectionCallback( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionReady );
        _firmata->FirmataConnectionFailed += ref new Microsoft::Maker::Firmata::FirmataConnectionCallbackWithMessage( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionFailed );
        _firmata->FirmataConnectionLost += ref new Microsoft::Maker::Firmata::FirmataConnectionCallbackWithMessage( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onConnectionLost );
        _firmata->unlock();
    }
}

RemoteDevice::~RemoteDevice(
    void
    )
{
    _firmata->finish();
}


//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
RemoteDevice::analogRead(
	Platform::String^ analog_pin_
	)
{
    uint16_t val = -1;
	uint8_t parsed_pin = parsePinFromAnalogString( analog_pin_ );

    {   //critical section 
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );

        //verify that we were given a valid analog pin number, parsePinFromAnalogString returns -1 as uint if the string is invalid, so this will catch both cases
        if( !_initialized || parsed_pin >= _hardwareProfile->AnalogPinCount )
        {
            return val;
        }

        //get the raw hardware pin number from the analog pin number by adding the digital pin count
        uint8_t analog_pin_num = parsed_pin + _hardwareProfile->AnalogOffset;

        //input and analog modes can be ambiguous, so we perform a courtesy check for the incorrect mode
        if( _pin_mode[analog_pin_num] == static_cast<uint8_t>( PinMode::INPUT ) )
        {
            //attempt to change to the correct mode
            pinMode( analog_pin_num, PinMode::ANALOG );
        }

        if( _pin_mode[analog_pin_num] != static_cast<uint8_t>( PinMode::ANALOG ) )
        {
            //incorrect pin mode, can't perform analog read
            return val;
        }

        val = _analog_pins[parsed_pin];
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
    std::lock_guard<std::recursive_mutex> lock( _device_mutex );

    if( !_initialized )
    {
        return;
    }

    //both PWM and SERVO are valid modes for this function, but OUTPUT is ambiguous with PWM. We perform a courtesy check for the correct mode
    if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::OUTPUT ) )
    {
        //attempt to change the pin mode
        pinMode( pin_, PinMode::PWM );
    }

    if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::PWM ) || _pin_mode[pin_] == static_cast<uint8_t>( PinMode::SERVO ) )
    {
        _firmata->sendAnalog( pin_, value_ );
    }
}


PinState
RemoteDevice::digitalRead(
    uint8_t pin_
    )
{
    int port;
    uint8_t port_mask;
    getPinMap( pin_, &port, &port_mask );

    {   //critical section
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );

        if( !_initialized )
        {
            return PinState::LOW;
        }

        //input and analog modes can be ambiguous, so we perform a courtesy check for the incorrect mode
        if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::ANALOG ) )
        {
            //attempt to change to the correct mode
            pinMode( pin_, PinMode::INPUT );
        }

        //we want to verify that the pin is in INPUT mode, but OUTPUT will technically work as well (mimic Arduino behavior here)
        if( _pin_mode[pin_] != static_cast<uint8_t>( PinMode::INPUT ) && _pin_mode[pin_] != static_cast<uint8_t>( PinMode::OUTPUT ) && _pin_mode[pin_] != static_cast<uint8_t>( PinMode::PULLUP ))
        {
            //incorrect pin mode
            return PinState::LOW;
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

    {   //critical section
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );

        if( !_initialized )
        {
            return;
        }

        //output can be ambiguous with PWM, so we perform a courtesy check for the incorrect mode
        if( _pin_mode[pin_] == static_cast<uint8_t>( PinMode::PWM ) )
        {
            //attempt to change the pin mode
            pinMode( pin_, PinMode::OUTPUT );
        }

        if( _pin_mode[pin_] != static_cast<uint8_t>( PinMode::OUTPUT ) )
        {
            //incorrect pin mode
            return;
        }

        if( static_cast<uint8_t>( state_ ) )
        {
            _digital_port[port] |= port_mask;
        }
        else
        {
            _digital_port[port] &= ~port_mask;
        }

        _firmata->sendDigitalPort( port, _digital_port[port] );
    }
}

PinMode
RemoteDevice::getPinMode(
    uint8_t pin_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::recursive_mutex> lock( _device_mutex );
    return static_cast<PinMode>( _pin_mode[ pin_ ].load() );
}

PinMode
RemoteDevice::getPinMode(
    Platform::String ^analog_pin_
    )
{
    uint8_t parsed_pin = parsePinFromAnalogString( analog_pin_ );
    if( parsed_pin == static_cast<uint8_t>( -1 ) )
    {
        return PinMode::IGNORED;
    }

    return getPinMode( parsed_pin + _hardwareProfile->AnalogOffset );
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

    {   //critical section
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );

        //verify we're initialized properly and the requested pin mode is supported by this pin
        if( !( _initialized && isModeSupported( pin_, mode_ ) ) )
        {
            return;
        }

        _firmata->lock();
        try
        {
            _firmata->write( static_cast<uint8_t>( Firmata::Command::SET_PIN_MODE ) );
            _firmata->write( pin_ );
            _firmata->write( static_cast<uint8_t>( mode_ ) );

            //lets subscribe to this port if we're setting it to input
            if( mode_ == PinMode::INPUT || mode_ == PinMode::PULLUP )
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
        }
        catch( ... )
        {
            //something has gone wrong, any fatal errors should be evented, so we need to exit this function
            _firmata->unlock();
            return;
        }

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

void
RemoteDevice::pinMode(
    Platform::String ^analog_pin_,
    PinMode mode_
    )
{
    uint8_t parsed_pin = parsePinFromAnalogString( analog_pin_ );
    if( parsed_pin == static_cast<uint8_t>( -1 ) )
    {
        return;
    }

    pinMode( parsed_pin + _hardwareProfile->AnalogOffset, mode_ );
}


//******************************************************************************
//* Callbacks
//******************************************************************************


void
RemoteDevice::onDigitalReport(
    Firmata::CallbackEventArgs ^args_
    )
{
    uint8_t port = args_->getPort();
    uint8_t port_val = static_cast<uint8_t>( args_->getValue() );
    uint8_t port_xor;

    {   //critical section
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );
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
            DigitalPinUpdated( ( port * 8 ) + i, ( ( port_val >> i ) & 0x01 ) > 0 ? PinState::HIGH : PinState::LOW );
        }
        port_xor >>= 1;
        ++i;
    }
}

void
RemoteDevice::onAnalogReport(
    Firmata::CallbackEventArgs ^args_
    )
{
    uint8_t pin = args_->getPort();
    uint16_t val = args_->getValue();

    {   //critical section
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );
        _analog_pins[pin] = val;
    }

    //throw an event for the pin value update
    AnalogPinUpdated( L"A" + pin.ToString(), val );
}

void
RemoteDevice::onSysexMessage(
    Firmata::SysexCallbackEventArgs ^argv_
    )
{
    SysexMessageReceived( argv_->getCommand(), Windows::Storage::Streams::DataReader::FromBuffer( argv_->getDataBuffer() ) );
}

void
RemoteDevice::onStringMessage(
    Firmata::StringCallbackEventArgs ^argv_
    )
{
    StringMessageReceived( argv_->getString() );
}


//******************************************************************************
//* Private Methods
//******************************************************************************

void const
RemoteDevice::initialize(
    HardwareProfile ^hardwareProfile_
    )
{
    {   //critical section equivalent to function scope
        std::lock_guard<std::recursive_mutex> lock( _device_mutex );

        if( _initialized ) return;
        _hardwareProfile = hardwareProfile_;
        _firmata->DigitalPortValueUpdated += ref new Firmata::CallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::CallbackEventArgs^ args ) -> void { onDigitalReport( args ); } );
        _firmata->AnalogValueUpdated += ref new Firmata::CallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::CallbackEventArgs^ args ) -> void { onAnalogReport( args ); } );
        _firmata->SysexMessageReceived += ref new Firmata::SysexCallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::SysexCallbackEventArgs^ args ) -> void { onSysexMessage( args ); } );
        _firmata->StringMessageReceived += ref new Firmata::StringCallbackFunction( [ this ]( Firmata::UwpFirmata ^caller, Firmata::StringCallbackEventArgs^ args ) -> void { onStringMessage( args ); } );

        std::fill( _digital_port.begin(), _digital_port.end(), 0 );
        std::fill( _subscribed_ports.begin(), _subscribed_ports.end(), 0 );
        std::fill( _analog_pins.begin(), _analog_pins.end(), 0 );
        std::fill( _pin_mode.begin(), _pin_mode.end(), static_cast<uint8_t>( PinMode::OUTPUT ) );

        _initialized = true;
    }
}

bool
RemoteDevice::isModeSupported(
    uint8_t pin_,
    PinMode mode_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::recursive_mutex> lock( _device_mutex );

    if( !_initialized )
    {
        return false;
    }

    //an initialized state with an invalid profile means we are using unsafe mode
    if( !_hardwareProfile->IsValid )
    {
        return true;
    }

    switch( mode_ )
    {
    case PinMode::ANALOG:
        return _hardwareProfile->isAnalogSupported( pin_ );

    case PinMode::I2C:
        return _hardwareProfile->isI2cSupported( pin_ );

    case PinMode::INPUT:
        return _hardwareProfile->isDigitalInputSupported( pin_ );

    case PinMode::OUTPUT:
        return _hardwareProfile->isDigitalOutputSupported( pin_ );

    case PinMode::PULLUP:
        return _hardwareProfile->isDigitalInputPullupSupported( pin_ );

    case PinMode::PWM:
        return _hardwareProfile->isPwmSupported( pin_ );

    case PinMode::SERVO:
        return _hardwareProfile->isServoSupported( pin_ );

    //these modes have no real purpose in firmata
    case PinMode::IGNORED:
    case PinMode::ENCODER:
    case PinMode::ONEWIRE:
    case PinMode::SERIAL:
    case PinMode::STEPPER:
    case PinMode::SHIFT:
    default:
        return false;
    }
}

void
RemoteDevice::getPinMap(
    uint8_t pin_,
    int *port_,
    uint8_t *port_mask_
    )
{
    if( port_ != nullptr )
    {
        *port_ = ( pin_ / 8 );
    }
    if( port_mask_ != nullptr )
    {
        *port_mask_ = ( 1 << ( pin_ % 8 ) );
    }
}

void
RemoteDevice::onConnectionFailed(
    Platform::String^ message_
    )
{
    DeviceConnectionFailed( message_ );
}

void
RemoteDevice::onConnectionLost(
    Platform::String^ message_
    )
{
    DeviceConnectionLost( message_ );
}

void
RemoteDevice::onConnectionReady(
    void
    )
{
    _firmata->PinCapabilityResponseReceived += ref new Microsoft::Maker::Firmata::SysexCallbackFunction( this, &Microsoft::Maker::RemoteWiring::RemoteDevice::onPinCapabilityResponseReceived );
    _firmata->startListening();

	//this async task will send a request for pin capability report from the device, wait for increasing intervals as long as the device
	//has not correctly responded. If, after a set amount of time, it is determined that no response has been received, it will repeat
	//the process for a set number of attempts. A device response will be received in the form of a PinCapabilityResponseReceived event.
    Concurrency::create_task( [ this ]
    {
        const int MAX_ATTEMPTS = 30;
		const int MAX_DELAY_LOOP = 5;
		const int INIT_DELAY_MS = 10;
        int attempts = 0;
		int delay_loop = 0;
		int delay_ms = INIT_DELAY_MS;
        
		//this loop will send the pin capability report and repeatedly lock the _device_mutex while checking the state of the object's initialization.
		//locking guarantees that the state is able to be read only entirely before or after a complete initialization.
        for( ;; )
        {
            {   //critical section
                std::lock_guard<std::recursive_mutex> lock( _device_mutex );
                if( _initialized ) return true;
            }

            if( attempts >= MAX_ATTEMPTS ) return false;

            //manually sending a sysex message asking for the pin configuration will guarantee it is sent properly even if a user has started a sysex message themselves
            _firmata->lock();
            try
            {
                _firmata->write( static_cast<uint8_t>( Command::START_SYSEX ) );
                _firmata->write( static_cast<uint8_t>( SysexCommand::CAPABILITY_QUERY ) );
                _firmata->write( static_cast<uint8_t>( Command::END_SYSEX ) );
                _firmata->flush();
            }
            catch( ... )
            {
                //if an error occurs here we count it as an attempt and continue.
            }

            _firmata->unlock();
            ++attempts;
			
			//this loop is responsible for waiting at increasing intervals until the response is received or MAX_DELAY_LOOP number of iterations have occurred.
			for( delay_loop = 0, delay_ms = INIT_DELAY_MS; delay_loop < MAX_DELAY_LOOP; ++delay_loop, delay_ms *= 2 )
			{
				Sleep( delay_ms );
				
				{   //critical section
					std::lock_guard<std::recursive_mutex> lock( _device_mutex );
					if( _initialized ) return true;
				}
			}
        }

        return false;
    } )
        .then( [ this ] ( task<bool> t )
    {
        try
        {
            bool result = t.get();
            if( !result ) throw ref new Platform::Exception( E_UNEXPECTED, L"Pin configuration not received." );

            DeviceReady();
        }
        catch( Platform::Exception ^e )
        {
            DeviceConnectionFailed( L"A device connection was established, but the device failed handshaking procedures. Verify that your device is configured with StandardFirmata. Message: " + e->Message );
        }
    } );
}


void
RemoteDevice::onPinCapabilityResponseReceived(
    UwpFirmata ^caller_,
    SysexCallbackEventArgs ^argv_
    )
{
    if( _initialized || argv_ == nullptr ) return;

    HardwareProfile ^hardwareProfile = ref new HardwareProfile( argv_->getDataBuffer() );
    if( hardwareProfile->IsValid )
    {
        initialize( hardwareProfile );
    }
}

uint8_t
RemoteDevice::parsePinFromAnalogString(
    Platform::String^ string_
    )
{
    //a valid string must contain at least 2 characters, 'a' or 'A' followed by a number
    if( string_ == nullptr || string_->Length() < 2 )
    {
        return -1;
    }

    const wchar_t *data = string_->Data();
    if( !( data[0] == 'a' || data[0] == 'A' ) )
    {
        return -1;
    }

    long int parsed_num = wcstol( data + 1, nullptr, 10 );

    //wcstol returns 0 on error condition, but 0 is also a valid pin. one last verification step
    if( parsed_num == 0 && data[1] != L'0' )
    {
        return -1;
    }
    return static_cast<uint8_t>( parsed_num );
}