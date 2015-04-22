
#pragma once

#include <cstdint>
#include "TwoWire.h"

namespace Microsoft {
namespace Maker {
namespace RemoteWiring {

public enum class PinMode
{
	INPUT = 0x00,
	OUTPUT = 0x01,
	ANALOG = 0x02,
	PWM = 0x03,
	SERVO = 0x04,
	SHIFT = 0x05,
	I2C = 0x06,
	ONEWIRE = 0x07,
	STEPPER = 0x08,
	ENCODER = 0x09,
	IGNORED = 0x7F,
	TOTAL_PIN_MODES = 0x0B,
};

public enum class PinState
{
	LOW = 0x00,
	HIGH = 0x01,
};

public delegate void DigitalPinUpdatedCallback( byte pin, PinState state );
public delegate void AnalogPinUpdatedCallback( byte pin, uint16_t value );

public ref class RemoteDevice sealed {

public:
	event DigitalPinUpdatedCallback ^ DigitalPinUpdatedEvent;
	event AnalogPinUpdatedCallback ^ AnalogPinUpdatedEvent;

	[Windows::Foundation::Metadata::DefaultOverload]
	RemoteDevice(
		Serial::IStream ^serial_connection_
	);


	RemoteDevice(
		Firmata::UAPFirmataClient ^firmata_
	);


	///<summary>
	///Perform a clean shutdown
	///</summary>
	void
		finish(
	);

	///<summary>
	///reads analog data stored in the cache
	///</summary>
	uint16_t
	analogRead(
		byte pin_
	);

	///<summary>
	///writes analog data to the remote device and the cache
	///</summary>
	void
	analogWrite(
		byte pin_,
		uint16_t value_
	);

	///<summary>
	///reads a digital pin state from the cache 
	///</summary>
	PinState
	digitalRead(
		byte pin_
	);

	///<summary>
	///writes a digital state to the remote device and the cache
	///</summary>
	void
	digitalWrite(
		byte pin_,
		PinState state_
	);

	///<summary>
	///sets the given pin to the given mode and stores the value in the cache
	///</summary>
	void
	pinMode(
		byte pin_,
		PinMode mode_
	);

	///<summary>
	///retrieves the mode of the given pin from the cache
	///</summary>
	PinMode
	getPinMode(
		byte pin_
	);

	///<summary>
	///singleton pattern returns a current instance of TwoWire for I2C communication
	///</summary>
	inline
	I2c::TwoWire ^
	getI2c(
		void
	)
	{
		if( _twoWire == nullptr )
		{
			_twoWire = ref new I2c::TwoWire( _firmata );
		}
		return _twoWire;
	}


private:
	//constant members
	static const int MAX_PORTS = 16;
	static const int MAX_PINS = 128;
	static const int ANALOG_PINS = 6;

	//singleton reference for I2C
	I2c::TwoWire ^_twoWire;

	//a reference to the UAP firmata interface
	Firmata::UAPFirmataClient ^_firmata;

	//initialization for constructor
	void const initialize();

	//reporting callbacks
	void onDigitalReport( Firmata::CallbackEventArgs ^argv );
	void onAnalogReport( Firmata::CallbackEventArgs ^argv );

	//maps the given pin number to the correct port and mask
	void getPinMap( byte, int *, byte * );

	//state-tracking cache variables
	byte volatile _subscribed_ports[MAX_PORTS];
	byte volatile _digital_port[MAX_PORTS];
	uint16 volatile _analog_pins[ANALOG_PINS];
	byte _pin_mode[MAX_PINS];
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
