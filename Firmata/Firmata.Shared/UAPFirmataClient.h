#pragma once

#include <cstdint>

using namespace Platform;

#ifdef IGNORE
#undef IGNORE
#endif

using namespace Windows::Storage::Streams;

namespace Wiring {
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

public ref class UAPFirmataClient sealed
{
public:
	event CallbackFunction^ DigitalCallbackEvent;
	event CallbackFunction^ AnalogCallbackEvent;
	event StringCallbackFunction^ StringCallbackEvent;
	event SysexCallbackFunction^ SysexCallbackEvent;
	event SystemResetCallbackFunction^ SystemResetCallbackEvent;

	UAPFirmataClient();

	void
	begin(
		Serial::ISerial ^s
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
    sendSysex(
		uint8_t command,
		uint8_t bytec,
		String ^bytev
    );

    void
    write(
		uint8_t c
	);


	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	digitalInvoke(
		UAPFirmataClient ^caller,
		uint8_t port_,
		int value_
	) {
		caller->DigitalCallbackEvent( caller, ref new CallbackEventArgs( port_, value_ ) );
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	analogInvoke(
		UAPFirmataClient ^caller,
		uint8_t pin_,
		int value_
	) {
		caller->AnalogCallbackEvent( caller, ref new CallbackEventArgs( pin_, value_ ) );
	}


  private:
};

}
}
