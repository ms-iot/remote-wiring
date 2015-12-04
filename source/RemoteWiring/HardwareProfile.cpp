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
    _num_analog_pins( ATOMIC_VAR_INIT( 0 ) )
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
    int number_of_digital_pins_,
    int number_of_analog_pins_
    ) :
    _is_valid( ATOMIC_VAR_INIT( false ) ),
    _total_pins( ATOMIC_VAR_INIT( number_of_analog_pins_ + number_of_digital_pins_ ) ),
    _analog_offset( ATOMIC_VAR_INIT( number_of_digital_pins_ ) ),
    _num_analog_pins( ATOMIC_VAR_INIT( number_of_analog_pins_ ) )
{
}


//******************************************************************************
//* Public Methods
//******************************************************************************


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
    byte analog_offset = 0;
    byte num_analog_pins = 0;

    for( unsigned int i = 0; i < size; ++i )
    {
        while( i < size && data[i] != FIRMATA_END_OF_PIN_VALUE )
        {
            PinMode mode = static_cast<PinMode>( data[i] );
            switch( mode )
            {
            case PinMode::INPUT:

                ++i;
                if( i < size && data[i] == MODE_ENABLED )
                {
                    //this should always be true, but we're doing nothing for now.
                }

                //skip over the 'enabled' value, which should always be 1
                ++i;

                break;

            case PinMode::OUTPUT:

                ++i;
                if( i < size && data[i] == MODE_ENABLED )
                {
                    //this should always be true, but we're doing nothing for now.
                }

                //skip over the 'enabled' value, which should always be 1
                ++i;

                break;

            case PinMode::PULLUP:

                ++i;
                if( i < size && data[i] == MODE_ENABLED )
                {
                    //this should always be true, but we're doing nothing for now.
                }

                //skip over the 'enabled' value, which should always be 1
                ++i;

                break;

            case PinMode::ANALOG:

                //analog offset keeps track of the first pin found that supports analog read, allows us to convert analog pins like "A0" to the correct pin number
                if( analog_offset == 0 )
                {
                    analog_offset = total_pins;
                }
                ++num_analog_pins;

                //this statement intentionally left unbroken

            case PinMode::PWM:
            case PinMode::SERVO:
            case PinMode::I2C:
                i += 2;

                break;

            default:
                ++i;

                break;
            }
        }
        total_pins++;
    }

    _total_pins = total_pins;
    _analog_offset = analog_offset;
    _num_analog_pins = num_analog_pins;
    _is_valid = true;
    //initialize();
}
