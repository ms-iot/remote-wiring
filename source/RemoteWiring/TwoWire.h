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

#include <cstdint>

namespace Microsoft {
namespace Maker {
namespace RemoteWiring {

ref class RemoteDevice;

namespace I2c {

public delegate void I2cReplyCallback( uint8_t address_, uint8_t reg_, Windows::Storage::Streams::DataReader ^response );

public ref class TwoWire sealed
{
public:
	friend ref class RemoteDevice;

	event I2cReplyCallback ^ I2cReplyEvent;

	//enables I2C with a 10ms delay time for requesting a response from the slave device
	void
	inline
	enable(
		void
		)
	{
		enable( 10000 );
	}

	//enables I2C with a given delay time for requesting a response from the slave device
	void
	enable(
		uint16_t i2cReadDelayMicros_
	);

	//writes raw data to the given address
	void
	write(
		uint8_t address_,
		Windows::Storage::Streams::IBuffer ^buff
	);

	//a one-time read which will does not prompt a register for data
	inline
	void
	read(
		uint8_t address_,
		size_t numBytes_
		) 
	{
		read( address_, numBytes_, 0xFF, false );
	}

	//a one-time read which will prompt the given register for data
	inline
	void
	read(
		uint8_t address_,
		size_t numBytes_,
		uint8_t reg_
		)
	{
		read( address_, numBytes_, reg_, false );
	}

	/*
	 * a read function which allows for the option of continous reads.
	 * if continuous reads are desired, a register must be provided.
	 */
	void
	read(
		uint8_t address_,
		size_t numBytes_,
		uint8_t reg_,
		bool continuous_
	);

	void
	stop(
		uint8_t address_
	);

private:
	//since 16 bit values are sent as two 7 bit bytes, you can't send a value larger than this across the wire
	const uint16_t MAX_READ_DELAY_MICROS = 0x3FFF;

	//singleton pattern w/ friend class to instantiate
	TwoWire(
		Firmata::UwpFirmata ^ firmata_
		) :
		_firmata( firmata_ )
	{
		_firmata->I2cReplyEvent += ref new Firmata::I2cReplyCallbackFunction( [this]( Firmata::UwpFirmata ^caller, Firmata::I2cCallbackEventArgs^ args ) -> void { onI2cReply( args ); } );
	}
	
	//a reference to the UAP firmata interface
	Firmata::UwpFirmata ^_firmata;

	//sysex-building
	const size_t MAX_SYSEX_LEN = 15;
	uint8_t _sys_command;
	uint8_t _sys_position;

	void
	sendI2cSysex(
		const uint8_t address_,
		const uint8_t rw_mask_,
		const uint8_t reg_,
		Platform::Array<uint8_t> ^data
	);

	void
	onI2cReply(
		Firmata::I2cCallbackEventArgs ^argv
	);
};

} // namespace I2c
} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
