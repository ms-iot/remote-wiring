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

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

using namespace Platform;
using namespace Concurrency;
using namespace Windows::Storage::Streams;

namespace Microsoft {
namespace Maker {
namespace Firmata {

ref class UwpFirmata;

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
		IBuffer ^sysex_string_
		) :
		_command( command_ ),
		_sysex_string( sysex_string_ )
	{
	}

	inline uint8_t getCommand( void ) { return _command; }

	inline IBuffer ^ getDataBuffer( void ) { return _sysex_string; }

private:
	uint8_t _command;
	IBuffer ^_sysex_string;
};

public ref class I2cCallbackEventArgs sealed
{
public:
	I2cCallbackEventArgs(
		uint8_t address_,
		uint8_t reg_,
		IBuffer ^response_
		) :
		_address( address_ ),
		_reg( reg_ ),
		_response( response_ )
	{
	}

	inline uint8_t getAddress( void ) { return _address; }

	inline uint8_t getRegister( void ) { return _reg; }

	inline IBuffer ^ getDataBuffer( void ) { return _response; }

private:
	uint8_t _address;
	uint8_t _reg;
	IBuffer ^_response;
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


public delegate void CallbackFunction( UwpFirmata ^caller, CallbackEventArgs ^argv );
public delegate void StringCallbackFunction(UwpFirmata ^caller, StringCallbackEventArgs ^argv);
public delegate void SysexCallbackFunction(UwpFirmata ^caller, SysexCallbackEventArgs ^argv);
public delegate void SystemResetCallbackFunction( UwpFirmata ^caller, SystemResetCallbackEventArgs ^argv );
public delegate void I2cReplyCallbackFunction( UwpFirmata ^caller, I2cCallbackEventArgs ^argv );

public ref class UwpFirmata sealed
{
public:
	event CallbackFunction^ DigitalPortValueEvent;
	event CallbackFunction^ AnalogValueEvent;
	event StringCallbackFunction^ StringEvent;
	event SysexCallbackFunction^ SysexEvent;
	event I2cReplyCallbackFunction^ I2cReplyEvent;
	event SystemResetCallbackFunction^ SystemResetEvent;

	UwpFirmata(
        void
    );

    virtual
    ~UwpFirmata(
        void
    );

	bool
	appendBlob(
		uint8_t byte_
	);

	bool
	appendSysex(
		uint8_t byte_
	);

    int
    available(
        void
    );

	void
	begin(
		Serial::IStream ^s
	);

	bool
	beginBlob(
		void
	);

	bool
	beginSysex(
		uint8_t command_
	);

	bool
	endBlob(
		void
	);

	bool
	endSysex(
		void
	);

    void
    lock(
        void
    );

	void
	finish(
		void
	);

	void
	flush(
		void
	);

    void
    printFirmwareVersion(
        void
    );

    void
    printVersion(
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
    sendString(
        String ^string
    );

    void
    sendString(
		uint8_t command,
        String ^string
    );

    void
	sendValueAsTwo7bitBytes(
		int value
	);
 
    void
    setFirmwareNameAndVersion(
		String ^name,
		uint8_t major,
		uint8_t minor
    );

	void
	startListening(
		void
	);

    void
    unlock(
        void
    );

    void
    write(
		uint8_t c
	);

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	analogInvoke(
		UwpFirmata ^caller,
		uint8_t pin_,
		int value_
	)
	{
		caller->AnalogValueEvent( caller, ref new CallbackEventArgs( pin_, value_ ) );
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	digitalInvoke(
		UwpFirmata ^caller,
		uint8_t port_,
		int value_
	)
	{
		caller->DigitalPortValueEvent( caller, ref new CallbackEventArgs( port_, value_ ) );
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	stringInvoke(
		UwpFirmata ^caller,
		uint8_t *string_data
	)
	{
		size_t len = strlen( reinterpret_cast<char *>( string_data ) ) + 1;
		size_t wlen = len * sizeof( wchar_t );
		wchar_t *wstr_data = new wchar_t[wlen];

		size_t c;
		mbstowcs_s( &c, wstr_data, wlen, reinterpret_cast<char *>(string_data), len + 1 );
		caller->StringEvent( caller, ref new StringCallbackEventArgs( ref new String(wstr_data) ) );
        delete[](wstr_data);
	}

	//when used with std::bind, this allows the Firmata library to invoke the function in the standard way (non-member type) while we redirect it to an object reference
	static inline
	void
	sysexInvoke(
		UwpFirmata ^caller,
		uint8_t command_,
		uint8_t argc_,
		uint8_t *argv_
	)
	{
		/*
		 * data will be replied as 2 7-bit bytes for every actual byte. So we're going to reuse
		 *  the same memory space, since we can combine the two bytes back together.
		 */

		//should never happen, but we'll correct for it just in case
		if( argc_ % 2 == 1 ) --argc_;

		//reassemble all the bytes (which were split into two seven-bit bytes) back into one byte each
		uint8_t i, len;
		for( i = 0, len = 0; i < argc_; i += 2, ++len )
		{
			argv_[len] = argv_[i] | ( argv_[i + 1] << 7 );
		}
		argv_[len] = 0;

		DataWriter ^writer = ref new DataWriter();
		if( command_ == static_cast<uint8_t>( SysexCommand::I2C_REPLY ) )
		{
			//if we're receiving an I2C reply, the first two bytes in our reply are the address and register
			for( i = 2; i < len; ++i )
			{
				writer->WriteByte( argv_[i] );
			}

			caller->I2cReplyEvent( caller, ref new I2cCallbackEventArgs( argv_[0], argv_[1], writer->DetachBuffer() ) );
		}
		else
		{
			for( i = 0; i < len; ++i )
			{
				writer->WriteByte( argv_[i] );
			}

			caller->SysexEvent( caller, ref new SysexCallbackEventArgs( command_, writer->DetachBuffer() ) );
		}
	}

  private:
	//sysex-building
	const size_t MAX_SYSEX_LEN = 15;
	uint8_t _sys_command;
	uint8_t _sys_position;

	//blob-related
	const size_t MAX_BLOB_LEN = 31;
	bool _blob_started;
	uint8_t _blob_position;

	//common buffer
	std::unique_ptr<uint8_t> _data_buffer;

	//member variables to hold the current input thread & communications
	Serial::IStream ^_firmata_stream;

	//thread-safe mechanisms. std::unique_lock used to manage the lifecycle of std::mutex
	std::mutex _firmutex;
	std::unique_lock<std::mutex> _firmata_lock;

	//input thread & behavior mechanisms
	std::thread _input_thread;
	std::atomic_bool _input_thread_should_exit;

	void
    inputThread(
        void
    );

	void
    stopThreads(
        void
    );
};

} // namespace Firmata
} // namespace Maker
} // namespace Microsoft