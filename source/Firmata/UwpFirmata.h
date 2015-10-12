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
public delegate void FirmataConnectionCallback();
public delegate void FirmataConnectionCallbackWithMessage( Platform::String ^message );

public ref class UwpFirmata sealed
{
public:
    event CallbackFunction^ DigitalPortValueUpdated;
    event CallbackFunction^ AnalogValueUpdated;
    event StringCallbackFunction^ StringMessageReceived;
    event SysexCallbackFunction^ SysexMessageReceived;
    event SysexCallbackFunction^ PinCapabilityResponseReceived;
    event I2cReplyCallbackFunction^ I2cReplyReceived;
    event SystemResetCallbackFunction^ SystemResetRequested;
    event FirmataConnectionCallback^ FirmataConnectionReady;
    event FirmataConnectionCallbackWithMessage^ FirmataConnectionFailed;
    event FirmataConnectionCallbackWithMessage^ FirmataConnectionLost;

    UwpFirmata(
        void
    );

    virtual
    ~UwpFirmata(
        void
    );

    ///<summary>
    ///Returns the number of bytes available to be read from the backing transport
    ///</summary>
    int
    available(
        void
    );

    ///<summary>
    ///Attaches the given IStream reference as the backing transport for this UwpFirmata instance.
    ///</summary>
    void
    begin(
        Serial::IStream ^s_
    );

    ///<summary>
    ///Returns true if the connection is currently established
    ///</summary>
    bool
    connectionReady(
        void
    );

    ///<summary>
    ///Finishes the usage of this UwpFirmata instance. Any existing connections will be closed.
    ///</summary>
    void
    finish(
        void
    );

    ///<summary>
    ///Flushes any awaiting data from the outbound queue. This function must be called before any data
    ///is sent across an active connection
    ///</summary>
    void
    flush(
        void
    );

    ///<summary>
    ///Locks this instance of the UwpFirmata object, allowing for thread safety and guaranteeing that messages do not interfere with each other.
    ///<para>when explicitly invoking this method, unlock() must be called when the lock is no longer needed.</para>
    ///</summary>
    void
    lock(
        void
    );

    ///<summary>
    ///Writes the firmware version.
    ///</summary>
    void
    printFirmwareVersion(
        void
    );

    ///<summary>
    ///Prints the Firmata version.
    ///</summary>
    void
    printVersion(
        void
    );

    ///<summary>
    ///Allows one byte to be read from an active connection and messages to be parsed. This function will need to be called multiple times
    ///before a single multi-byte message can be completed and the appropriate action taken.
    ///</summary>
    void
    processInput(
        void
    );

    ///<summary>
    ///Sends an analog value for a given pin across an active connection
    ///</summary>
    void
    sendAnalog(
        uint8_t pin_,
        uint16_t value
    );

    ///<summary>
    ///Sends an digital value for a given port across an active connection
    ///</summary>
    void
    sendDigitalPort(
        uint8_t port_number_,
        uint8_t port_data_
    );

    ///<summary>
    ///Sends string data using the STRING_DATA command across an active connection
    ///</summary>
    void
    sendString(
        String ^string_
    );

    ///<summary>
    ///Sends string data with a custom command across an active connection
    ///</summary>
    void
    sendString(
        uint8_t command_,
        String ^string_
    );
    
    ///<summary>
    ///This function will send a sysex message with one of the pre-defined command types reserved by the Firmata protocol
    ///</summary>
    [Windows::Foundation::Metadata::DefaultOverload]
    void
    sendSysex(
        SysexCommand command_,
        IBuffer ^buffer_
    );

    ///<summary>
    ///This function will send a sysex message with the given command byte which must be greater than 0x00 and less than 0x7F.
    ///Custom commands should be in the range of [0x01-0x68], as the Firmata protocol defines commands 0x69 and above.
    ///All data given in the buffer object must be less than or equal to 0x7F as the MSB will always be cleared so as not to be misinterpreted as a command.
    ///It is typical practice to split your data into two or more bytes if the number of significant bits is greater than 7. You will need to reassemble this
    ///data on the other end. The Firmata library defines and follows this behavior for all of its reserved command types.
    ///</summary>
    void
    sendSysex(
        uint8_t command_,
        IBuffer ^buffer_
    );

    ///<summary>
    ///Sends a given byte value as two seven-bit bytes
    ///</summary>
    void
    sendValueAsTwo7bitBytes(
        uint16_t value_
    );

    ///<summary>
    ///Sets the firmware name and version
    ///</summary>
    void
    setFirmwareNameAndVersion(
        String ^name_,
        uint8_t major_,
        uint8_t minor_
    );

    ///<summary>
    ///Spins up a thread which will listen for and process input.
    ///<para>This function must be called before any inputs can be processed and corresponding events can be raised.</para>
    ///</summary>
    void
    startListening(
        void
    );

    ///<summary>
    ///Unlocks this instance of the UwpFirmata object, allowing other threads or actions to use it.
    ///<para>This function must be explicitly invoked after each invocation of the lock() method, when the lock is no longer needed.</para>
    ///</summary>
    void
    unlock(
        void
    );

    ///<summary>
    ///Writes a single byte using an active connection
    ///</summary>
    void
    write(
        uint8_t c_
    );

  private:
    const uint8_t FIRMATA_PROTOCOL_MAJOR_VERSION = 2;
    const uint8_t FIRMATA_PROTOCOL_MINOR_VERSION = 3;
    const double MESSAGE_TIMEOUT_MILLIS = 500.0;

    //version number and name array used with set/printFirmwareVersion
    uint8_t firmwareVersionMajor;
    uint8_t firmwareVersionMinor;
    std::string *firmwareName;

    //common buffer for outgoing messages, limited by the message size that can be read by Android's implementation of Firmata
    const size_t DATA_BUFFER_SIZE = 31;
    std::unique_ptr<uint16_t> _data_buffer;

    //member variables to hold the current input thread & communications
    Serial::IStream ^_firmata_stream;

    //stores the state of the connection
    std::atomic_bool _connection_ready;

    //thread-safe mechanisms. std::unique_lock used to manage the lifecycle of std::mutex
    std::mutex _firmutex;
    std::unique_lock<std::mutex> _firmata_lock;

    //input thread & behavior mechanisms
    std::thread _input_thread;
    std::atomic_bool _input_thread_should_exit;

    String ^
    createStringFromMbs(
        uint8_t *mbs_,
        size_t len_
    );

    void
    inputThread(
        void
    );

    void
    onConnectionEstablished(
        void
    );

    void
    onConnectionFailed(
        Platform::String ^message_
    );

    void
    onConnectionLost(
        Platform::String ^message_
    );

    void
    stopThreads(
        void
    );

    void
    reassembleByteString(
        uint8_t *byte_string_,
        size_t length_
    );
};

} // namespace Firmata
} // namespace Maker
} // namespace Microsoft