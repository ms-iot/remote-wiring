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
	_firmata->lock();
	_firmata->beginSysex( static_cast<uint8_t>( SysexCommand::I2C_CONFIG ) );
	_firmata->appendSysex( ( i2cReadDelayMicros_ > MAX_READ_DELAY_MICROS ) ? MAX_READ_DELAY_MICROS : i2cReadDelayMicros_ );
	_firmata->endSysex();
	_firmata->unlock();
}


void
TwoWire::write(
	uint8_t address_,
	Windows::Storage::Streams::IBuffer ^buff
	)
{
	if( buff == nullptr || buff->Length == 0 ) return;
	auto stream = Windows::Storage::Streams::DataReader::FromBuffer( buff );
	auto data = ref new Platform::Array<uint8_t>( stream->UnconsumedBufferLength );
	stream->ReadBytes( data );

	sendI2cSysex( address_, 0, 0xFF, data );
}

void
TwoWire::read(
	uint8_t address_,
	size_t numBytes_,
	uint8_t reg_,
	bool continuous_
	)
{
	//if you want to do continuous reads, you must provide a register to prompt for new data
	if( continuous_ && ( reg_ == 0xFF ) ) return;
	auto data = ref new Platform::Array<uint8_t>( 1 );
	data[0] = numBytes_;
	sendI2cSysex( address_, ( continuous_ ? 0x10 : 0x08 ), reg_, data );
}


void
TwoWire::stop(
	uint8_t address_
	)
{
	sendI2cSysex( address_, 0x18, 0xFF, nullptr );
}


//******************************************************************************
//* Private Methods
//******************************************************************************

void
TwoWire::sendI2cSysex(
	const uint8_t address_,
	const uint8_t rw_mask_,
	const uint8_t reg_,
	Platform::Array<uint8_t> ^data
	)
{
	_firmata->lock();
	_firmata->write( static_cast<uint8_t>( Command::START_SYSEX ) );
	_firmata->write( static_cast<uint8_t>( Microsoft::Maker::Firmata::SysexCommand::I2C_REQUEST ) );
	_firmata->write( address_ );
	_firmata->write( rw_mask_ );

	if( reg_ != 0xFF )
	{
		_firmata->sendValueAsTwo7bitBytes( reg_ );
	}

	if( data != nullptr )
	{
		for( size_t i = 0; i < data->Length; ++i )
		{
			_firmata->sendValueAsTwo7bitBytes( data[i] );
		}
	}

	_firmata->write( static_cast<uint8_t>( Command::END_SYSEX ) );
	_firmata->unlock();
}


void
TwoWire::onI2cReply(
	I2cCallbackEventArgs ^args
	)
{
	I2cReplyEvent( args->getAddress(), args->getRegister(), Windows::Storage::Streams::DataReader::FromBuffer( args->getDataBuffer() ) );
}
