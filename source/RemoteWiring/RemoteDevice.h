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

public delegate void DigitalPinUpdatedCallback( uint8_t pin, PinState state );
public delegate void AnalogPinUpdatedCallback( uint8_t pin, uint16_t value );

public ref class RemoteDevice sealed {

	//singleton reference for I2C
	I2c::TwoWire ^_twoWire;

public:
	event DigitalPinUpdatedCallback ^ DigitalPinUpdatedEvent;
	event AnalogPinUpdatedCallback ^ AnalogPinUpdatedEvent;

	property I2c::TwoWire ^ I2c
	{
		Microsoft::Maker::RemoteWiring::I2c::TwoWire ^ get()
		{
			if( _twoWire == nullptr )
			{
				_twoWire = ref new Microsoft::Maker::RemoteWiring::I2c::TwoWire( _firmata );
			}
			return _twoWire;
		}
	};

	[Windows::Foundation::Metadata::DefaultOverload]
	RemoteDevice(
		Serial::IStream ^serial_connection_
	);


	RemoteDevice(
		Firmata::UwpFirmata ^firmata_
	);

	virtual ~RemoteDevice();

	///<summary>
	///reads analog data stored in the cache
	///</summary>
	uint16_t
	analogRead(
		uint8_t pin_
	);

	///<summary>
	///writes analog data to the remote device and the cache
	///</summary>
	void
	analogWrite(
		uint8_t pin_,
		uint16_t value_
	);

	///<summary>
	///reads a digital pin state from the cache 
	///</summary>
	PinState
	digitalRead(
		uint8_t pin_
	);

	///<summary>
	///writes a digital state to the remote device and the cache
	///</summary>
	void
	digitalWrite(
		uint8_t pin_,
		PinState state_
	);

	///<summary>
	///sets the given pin to the given mode and stores the value in the cache
	///</summary>
	void
	pinMode(
		uint8_t pin_,
		PinMode mode_
	);

	///<summary>
	/// Retrieves the mode of the given pin from the cache stored by RemoteDevice class. 
	/// This is not a function you will find in the Arduino API, but is an extremely helpful function
	/// that allows the RemoteDevice class to maintain consistency with your apps.
	///</summary>
	PinMode
	getPinMode(
		uint8_t pin_
	);


private:
	//constant members
	static const int MAX_PORTS = 16;
	static const int MAX_PINS = 128;
	static const int ANALOG_PINS = 6;

	//a reference to the UAP firmata interface
	Firmata::UwpFirmata ^_firmata;

	//initialization for constructor
	void const initialize();

	//reporting callbacks
	void onDigitalReport( Firmata::CallbackEventArgs ^argv );
	void onAnalogReport( Firmata::CallbackEventArgs ^argv );

	//maps the given pin number to the correct port and mask
	void getPinMap( uint8_t, int *, uint8_t * );

	//state-tracking cache variables
	uint8_t volatile _subscribed_ports[MAX_PORTS];
	uint8_t volatile _digital_port[MAX_PORTS];
	uint16_t volatile _analog_pins[ANALOG_PINS];
	uint8_t _pin_mode[MAX_PINS];
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
