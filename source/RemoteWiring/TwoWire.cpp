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
