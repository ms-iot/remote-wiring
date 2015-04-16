#pragma once

#include <cstdint>

using namespace Platform;
using namespace Concurrency;
using namespace Windows::Storage::Streams;

namespace Microsoft {
namespace Maker {
namespace Firmata {

ref class UAPFirmataClient;

public ref class CallbackEventArgs sealed
{
public:
	CallbackEventArgs(
		uint8_t port_,
		uint16_t value_
	) :
		_port( port_ ),
		_value( value_ )
	{
	}

	inline uint8_t getPort( void ) { return _port; }

	inline uint16_t getValue( void ) { return _value; }

private:
	uint8_t _port;
	uint16_t _value;
};

public ref class StringCallbackEventArgs sealed {
  public:
    StringCallbackEventArgs(
        String ^string_
    ) :
      _string(string_)
    {}

    inline String ^ getString(void) { return _string; }

  private:
    String ^_string;
};

public ref class SysexCallbackEventArgs sealed
{
public:
	SysexCallbackEventArgs(
		uint8_t command_,
		String ^sysex_string_
		) :
		_command( command_ ),
		_sysex_string( sysex_string_ )
	{
	}

	inline uint8_t getCommand( void ) { return _command; }

	inline String ^ getSysexString( void ) { return _sysex_string; }

private:
	uint8_t _command;
	String ^_sysex_string;
};

public ref class I2cCallbackEventArgs sealed
{
public:
	I2cCallbackEventArgs(
		uint8_t address_,
		String ^response_
		) :
		_address( address_ ),
		_response( response_ )
	{
	}

	inline uint8_t getAddress( void ) { return _address; }

	inline String ^ getResponseString( void ) { return _response; }

private:
	uint8_t _address;
	String ^_response;
};

public ref class SystemResetCallbackEventArgs sealed {
  public:
      SystemResetCallbackEventArgs() {}
  private:
};

public enum class Command {
    ANALOG_MESSAGE = 0xE0,
    DIGITAL_MESSAGE = 0x90,
    REPORT_ANALOG_PIN = 0xC0,
    REPORT_DIGITAL_PIN = 0xD0,
    SET_PIN_MODE = 0xF4,
    START_SYSEX = 0xF0,
    END_SYSEX = 0xF7,
    PROTOCOL_VERSION = 0xF9,
    SYSTEM_RESET = 0xFF,
};

public enum class SysexCommand {
    ENCODER_DATA = 0x61,
    SERVO_CONFIG = 0x70,
    STRING_DATA = 0x71,
    STEPPER_DATA = 0x72,
    ONEWIRE_DATA = 0x73,
    SHIFT_DATA = 0x75,
    I2C_REQUEST = 0x76,
    I2C_REPLY = 0x77,
    I2C_CONFIG = 0x78,
    EXTENDED_ANALOG = 0x6F,
    PIN_STATE_QUERY = 0x6D,
    PIN_STATE_RESPONSE = 0x6E,
    CAPABILITY_QUERY = 0x6B,
    CAPABILITY_RESPONSE = 0x6C,
    ANALOG_MAPPING_QUERY = 0x69,
    ANALOG_MAPPING_RESPONSE = 0x6A,
    REPORT_FIRMWARE = 0x79,
    SAMPLING_INTERVAL = 0x7A,
    SCHEDULER_DATA = 0x7B,
    SYSEX_NON_REALTIME = 0x7E,
    SYSEX_REALTIME = 0x7F,
};


public delegate void CallbackFunction( UAPFirmataClient ^caller, CallbackEventArgs ^argv );
public delegate void StringCallbackFunction(UAPFirmataClient ^caller, StringCallbackEventArgs ^argv);
public delegate void SysexCallbackFunction(UAPFirmataClient ^caller, SysexCallbackEventArgs ^argv);
public delegate void SystemResetCallbackFunction( UAPFirmataClient ^caller, SystemResetCallbackEventArgs ^argv );
public delegate void I2cReplyCallbackFunction( UAPFirmataClient ^caller, I2cCallbackEventArgs ^argv );

public ref class UAPFirmataClient sealed
{
public:
	event CallbackFunction^ DigitalPortValueEvent;
	event CallbackFunction^ AnalogValueEvent;
	event StringCallbackFunction^ StringEvent;
	event SysexCallbackFunction^ SysexEvent;
	event I2cReplyCallbackFunction^ I2cEvent;
	event SystemResetCallbackFunction^ SystemResetEvent;

	UAPFirmataClient();

	void
	begin(
		Serial::IArduinoStream ^s
	);

	void
	finish(
		void
	);

    void
    printVersion(
        void
    );

    void
    printFirmwareVersion(
        void
    );

    void
    setFirmwareNameAndVersion(
		String ^name,
		uint8_t major,
		uint8_t minor
    );

    int
    available(
        void
    );

    void
    processInput(
        void
    );

    void
    sendAnalog(
		uint8_t pin,
		int value
    );

    void
    sendDigitalPort(
		uint8_t portNumber,
		int portData
    );

	void
	setDigitalReadEnabled(
		uint8_t portNumber,
		int portData
	);

    void
    sendString(
        String ^string
    );

    void
    sendString(
		uint8_t command,
        String ^string
    );

	void
	beginSysex(
		uint8_t command_
	);

	void
	appendSysex(
		uint8_t byte_
	);

	void
	endSysex(
		void
	);

    void
    write(
		uint8_t c
	);

	void
	enableI2c(
		uint16_t i2cReadDelayMicros_
	);

	void
	writeI2c(
		uint8_t address_,
		String ^message_
	);

	void
	readI2c(
		uint8_t address_,
		size_t numBytes_,
		uint8_t reg_,
		bool continuous_
	);

	void
	stopI2c(
		uint8_t address_
	);

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	digitalInvoke(
		UAPFirmataClient ^caller,
		uint8_t port_,
		int value_
	)
	{
		caller->DigitalPortValueEvent( caller, ref new CallbackEventArgs( port_, value_ ) );
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	analogInvoke(
		UAPFirmataClient ^caller,
		uint8_t pin_,
		int value_
	)
	{
		caller->AnalogValueEvent( caller, ref new CallbackEventArgs( pin_, value_ ) );
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	sysexInvoke(
		UAPFirmataClient ^caller,
		uint8_t command_,
		uint8_t argc_,
		uint8_t *argv_
	)
	{
		/*
		 * we need to convert to wchar string, but the data will be replied as 2 7-bit bytes for every actual byte. So we're going to reuse the same memory
		 * space, since we can combine the two bytes back together and zero out the MSB. additionally, cannot use std string conversion functions here because some bytes may be 0.
		 */

		//should never happen, but we'll correct for it just in case
		if( argc_ % 2 == 1 ) --argc_;

		uint8_t i;
		for( i = 0; i < argc_; i += 2 )
		{
			argv_[i] = argv_[i] | ( argv_[i + 1] << 7 );
			argv_[i + 1] = 0;
		}

		if( command_ == static_cast<uint8_t>( SysexCommand::I2C_REPLY ) )
		{
			//if we're receiving an I2C reply we should remove the first 2 characters (4 bytes) from the string, which are the reply address and the register.
			caller->I2cEvent( caller, ref new I2cCallbackEventArgs( argv_[0], ref new String( (const wchar_t *)( argv_ + 4 ), ( argc_ - 4 ) / 2 ) ) );
		}
		else
		{
			caller->SysexEvent( caller, ref new SysexCallbackEventArgs( command_, ref new String( (const wchar_t *)argv_, argc_ / 2 ) ) );
		}
	}

  private:
	//sysex-building
	  uint8_t _sysCommand;
	  uint8_t *_sysBuffer;
	  uint8_t _sysPosition;
	  const size_t MAX_SYSEX_LEN = 16;

	  //threading state member variables
	  volatile bool inputThreadRunning;
	  volatile bool inputThreadExited;

	  //input-thread related functions
	  void inputThread(void);
	  void stopThreads(void);

	void
	sendI2cSysex(
		const uint8_t address_,
		const uint8_t rw_mask_,
		const uint8_t reg_,
		const size_t len,
		const char * data
	);
};

} // namespace Firmata
} // namespace Maker
} // namespace Microsoft