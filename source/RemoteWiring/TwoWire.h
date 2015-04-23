#include <cstdint>

namespace Microsoft {
namespace Maker {
namespace RemoteWiring {

ref class RemoteDevice;

namespace I2c {

public delegate void I2cReplyCallback( uint8_t address_, Platform::String ^ response );

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
		Platform::String ^message_
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
	//singleton pattern w/ friend class to instantiate
	TwoWire(
		Firmata::UapFirmata ^ firmata_
		) :
		_firmata( firmata_ )
	{
		_firmata->I2cReplyEvent += ref new Firmata::I2cReplyCallbackFunction( [this]( Firmata::UapFirmata ^caller, Firmata::I2cCallbackEventArgs^ args ) -> void { onI2cReply( args ); } );
	}

	//since 16 bit values are sent as two 7 bit bytes, you can't send a value larger than this across the wire
	const uint16_t MAX_READ_DELAY_MICROS = 0x3FFF;

	void onI2cReply( Firmata::I2cCallbackEventArgs ^argv );

	//a reference to the UAP firmata interface
	Firmata::UapFirmata ^_firmata;
};

} // namespace I2c
} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
