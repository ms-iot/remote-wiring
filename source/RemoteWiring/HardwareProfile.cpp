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
#include "HardwareProfile.h"
#include "RemoteDevice.h"

using namespace Microsoft::Maker::Firmata;
using namespace Microsoft::Maker::RemoteWiring;

//******************************************************************************
//* Constructors / Destructors
//******************************************************************************

HardwareProfile::HardwareProfile(
    Windows::Storage::Streams::IBuffer ^buffer_,
    Protocol protocol_
    ) :
    _is_valid( ATOMIC_VAR_INIT( false ) ),
    _total_pins( ATOMIC_VAR_INIT( 0 ) ),
    _analog_offset( ATOMIC_VAR_INIT( 0 ) ),
    _num_analog_pins( ATOMIC_VAR_INIT( 0 ) ),
    _pinCapabilities( nullptr ),
    _analogResolutions( nullptr ),
    _pwmResolutions( nullptr ),
    _servoResolutions( nullptr )
{
    switch( protocol_ )
    {
    default:
    case Protocol::FIRMATA:
        initializeWithFirmata( buffer_ );
        break;
    }
}

HardwareProfile::HardwareProfile(
    int total_number_of_pins_,
    int number_of_analog_pins_
    ) :
    _is_valid( ATOMIC_VAR_INIT( false ) ),
    _total_pins( ATOMIC_VAR_INIT( total_number_of_pins_ ) ),
    _analog_offset( ATOMIC_VAR_INIT( total_number_of_pins_ - number_of_analog_pins_ ) ),
    _num_analog_pins( ATOMIC_VAR_INIT( number_of_analog_pins_ ) ),
    _pinCapabilities( nullptr ),
    _analogResolutions( nullptr ),
    _pwmResolutions( nullptr ),
    _servoResolutions( nullptr )
{
}


HardwareProfile::~HardwareProfile()
{
    _is_valid = false;

    if( _pinCapabilities != nullptr )
    {
        delete( _pinCapabilities );
    }

    if( _analogResolutions != nullptr )
    {
        delete( _analogResolutions );
    }

    if( _pwmResolutions != nullptr )
    {
        delete( _pwmResolutions );
    }

    if( _servoResolutions != nullptr )
    {
        delete( _servoResolutions );
    }
}


//******************************************************************************
//* Public Methods
//******************************************************************************

uint8_t
HardwareProfile::getPinCapabilitiesBitmask(
    uint8_t pin_
    )
{
    if( !_is_valid || _pinCapabilities == nullptr || pin_ < 0 || pin_ >= _total_pins )
    {
        return 0;
    }
    return _pinCapabilities->at( pin_ );
}

bool
HardwareProfile::isAnalogSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::ANALOG );
}

bool
HardwareProfile::isDigitalInputSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::INPUT );
}

bool
HardwareProfile::isDigitalInputPullupSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::INPUT_PULLUP );
}

bool
HardwareProfile::isDigitalOutputSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::OUTPUT );
}

bool
HardwareProfile::isI2cSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::I2C );
}

bool
HardwareProfile::isPwmSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::PWM );
}

bool
HardwareProfile::isServoSupported(
    uint8_t pin_
    )
{
    return getPinCapabilitiesBitmask( pin_ ) & static_cast<uint8_t>( PinCapability::SERVO );
}

//******************************************************************************
//* Private Methods
//******************************************************************************

void
HardwareProfile::initializeWithFirmata(
    Windows::Storage::Streams::IBuffer ^buffer_
    )
{
    if( buffer_ == nullptr ) return;

    const uint8_t MODE_ENABLED = 1;
    const int FIRMATA_END_OF_PIN_VALUE = 0x7F;

    auto reader = Windows::Storage::Streams::DataReader::FromBuffer( buffer_ );
    auto size = buffer_->Length;

    uint8_t *data = (uint8_t *)malloc( sizeof( uint8_t ) * size );
    for( unsigned int i = 0; i < size; ++i )
    {
        data[i] = reader->ReadByte();
    }

    byte total_pins = 0;
    byte analog_offset = 0xFF;
    byte num_analog_pins = 0;
    std::vector<uint8_t> *pinCapabilities = new std::vector<uint8_t>;
    std::map<uint8_t, uint8_t> *analogResolutions = new std::map<uint8_t, uint8_t>; //K = pin number, V = resolution value in bits
    std::map<uint8_t, uint8_t> *pwmResolutions = new std::map<uint8_t, uint8_t>; //K = pin number, V = resolution value in bits
    std::map<uint8_t, uint8_t> *servoResolutions = new std::map<uint8_t, uint8_t>; //K = pin number, V = resolution value in bits

    for( unsigned int i = 0; i < size; ++i )
    {
        uint8_t currentPinCapabilities = 0;

        while( i < size && data[i] != FIRMATA_END_OF_PIN_VALUE )
        {
            PinMode mode = static_cast<PinMode>( data[i] );
            switch( mode )
            {
            case PinMode::INPUT:

                //the next bit determines if the mode is enabled, this should always be true
                ++i;
                if( i < size )
                {
                    if( data[i] == MODE_ENABLED )
                    {
                        currentPinCapabilities |= static_cast<uint8_t>( PinCapability::INPUT );
                    }
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            case PinMode::OUTPUT:

                //the next bit determines if the mode is enabled, this should always be true
                ++i;
                if( i < size )
                {
                    if( data[i] == MODE_ENABLED )
                    {
                        currentPinCapabilities |= static_cast<uint8_t>( PinCapability::OUTPUT );
                    }
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            case PinMode::PULLUP:

                //the next bit determines if the mode is enabled, this should always be true
                ++i;
                if( i < size )
                {
                    if( data[i] == MODE_ENABLED )
                    {
                        currentPinCapabilities |= static_cast<uint8_t>( PinCapability::INPUT_PULLUP );
                    }
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            case PinMode::ANALOG:

                //set the mode enabled & store the analog resolution of this pin
                ++i;
                if( i < size )
                {
                    currentPinCapabilities |= static_cast<uint8_t>( PinCapability::ANALOG );
                    analogResolutions->insert( std::make_pair( total_pins, data[i] ) );
                }
                else return;    //we've failed to get all of the data
                ++i;

                //analog offset keeps track of the first pin found that supports analog read, tells us how many digital pins we have,
                //and allows us to convert analog pins like "A0" to the correct pin number
                if( analog_offset == 0xFF )
                {
                    analog_offset = total_pins;
                }
                ++num_analog_pins;

                break;

            case PinMode::PWM:

                //set the mode enabled & store the pwm resolution of this pin
                ++i;
                if( i < size )
                {
                    currentPinCapabilities |= static_cast<uint8_t>( PinCapability::PWM );
                    pwmResolutions->insert( std::make_pair( total_pins, data[i] ) );
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            case PinMode::SERVO:

                //set the mode enabled & store the servo resolution of this pin
                ++i;
                if( i < size )
                {
                    currentPinCapabilities |= static_cast<uint8_t>( PinCapability::SERVO );
                    servoResolutions->insert( std::make_pair( total_pins, data[i] ) );
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            case PinMode::I2C:

                //the next bit determines if the mode is enabled, this should always be true
                ++i;
                if( i < size )
                {
                    if( data[i] == MODE_ENABLED )
                    {
                        currentPinCapabilities |= static_cast<uint8_t>( PinCapability::I2C );
                    }
                }
                else return;    //we've failed to get all of the data
                ++i;

                break;

            default:
                //this value isn't recognized. it is likely fatal, but also possible that new data was added to the query response. We will attempt to continue
                ++i;

                break;
            }
        }
        pinCapabilities->push_back( currentPinCapabilities );
        total_pins++;
    }

    //last minute error-checking
    if( total_pins != pinCapabilities->size() ||
        num_analog_pins != analogResolutions->size() )
    {
        return;
    }

    //we've successfully parsed a valid capability response. Set all members of this class and mark it as valid.
    _total_pins = total_pins;
    _analog_offset = analog_offset;
    _num_analog_pins = num_analog_pins;
    _pinCapabilities = pinCapabilities;
    _analogResolutions = analogResolutions;
    _pwmResolutions = pwmResolutions;
    _servoResolutions = servoResolutions;
    _is_valid = true;
}
