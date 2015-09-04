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
#include "TwoWire.h"

using namespace Microsoft::Maker::Firmata;
using namespace Microsoft::Maker::RemoteWiring::I2c;

void
TwoWire::enable(
    uint16_t i2cReadDelayMicros_
    )
{
	i2cReadDelayMicros_ = (i2cReadDelayMicros_ > MAX_READ_DELAY_MICROS) ? MAX_READ_DELAY_MICROS : i2cReadDelayMicros_;

    using Windows::Storage::Streams::DataWriter;
    DataWriter ^writer = ref new DataWriter();
    writer->WriteByte( i2cReadDelayMicros_ & 0x7F );
    writer->WriteByte( ( i2cReadDelayMicros_ >> 7 ) & 0x7F );

    _firmata->sendSysex( SysexCommand::I2C_CONFIG, writer->DetachBuffer() );
}


void
TwoWire::beginTransmission(
    uint8_t address_
    )
{
    if( _address ) return;
    _address = address_;
    _position = 0;
}

void
TwoWire::write(
    uint8_t data_
    )
{
    if( !_address || _position > MAX_MESSAGE_LEN ) return;
    _data_buffer.get()[_position] = data_;
    ++_position;
}


void
TwoWire::endTransmission(
    void
    )
{
    if( !_address ) return;
    sendI2cSysex( _address, 0, _position, _data_buffer.get() );
    _address = 0;
    _position = 0;
}


//******************************************************************************
//* Private Methods
//******************************************************************************

void
TwoWire::sendI2cSysex(
    const uint8_t address_,
    const uint8_t rw_mask_,
    const uint8_t len_,
    uint8_t *data_
    )
{
    _firmata->lock();
    try
    {
        _firmata->write( static_cast<uint8_t>( Command::START_SYSEX ) );
        _firmata->write( static_cast<uint8_t>( Microsoft::Maker::Firmata::SysexCommand::I2C_REQUEST ) );
        _firmata->write( address_ );
        _firmata->write( rw_mask_ );

        if( data_ != nullptr )
        {
            for( size_t i = 0; i < len_; ++i )
            {
                _firmata->sendValueAsTwo7bitBytes( data_[i] );
            }
        }

        _firmata->write( static_cast<uint8_t>( Command::END_SYSEX ) );
        _firmata->flush();
        _firmata->unlock();
    }
    catch( ... )
    {
        _firmata->unlock();
    }
}


void
TwoWire::onI2cReply(
    I2cCallbackEventArgs ^args
    )
{
    I2cReplyEvent( args->getAddress(), args->getRegister(), Windows::Storage::Streams::DataReader::FromBuffer( args->getDataBuffer() ) );
}
