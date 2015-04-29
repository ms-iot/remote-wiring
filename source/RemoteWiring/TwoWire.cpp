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
	_firmata->enableI2c( ( i2cReadDelayMicros_ > MAX_READ_DELAY_MICROS ) ? MAX_READ_DELAY_MICROS : i2cReadDelayMicros_ );
}


void
TwoWire::write(
	uint8_t address_,
	Platform::String ^message_
	)
{
	_firmata->writeI2c( address_, message_ );
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
	_firmata->readI2c( address_, numBytes_, reg_, continuous_ );
}


void
TwoWire::stop(
	uint8_t address_
	)
{
	_firmata->stopI2c( address_ );
}


void
TwoWire::onI2cReply(
	I2cCallbackEventArgs ^args
	)
{
	I2cReplyEvent( args->getAddress(), args->getResponseString() );
}
