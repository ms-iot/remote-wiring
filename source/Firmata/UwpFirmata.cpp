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
#include "UwpFirmata.h"
#include <cstdlib>

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::Firmata;
using namespace std::placeholders;




//******************************************************************************
//* Constructors
//******************************************************************************


UwpFirmata::UwpFirmata(
    void
) :
    _data_buffer(new uint8_t[DATA_BUFFER_SIZE]),
    _firmata_lock(_firmutex, std::defer_lock),
    _firmata_stream(nullptr),
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _input_thread_should_exit(ATOMIC_VAR_INIT(false)),
    _sys_command(0),
    _sys_position(0),
    firmwareVersionMajor(0),
    firmwareVersionMinor(0)
{
}


//******************************************************************************
//* Destructors
//******************************************************************************


UwpFirmata::~UwpFirmata(
    void
    )
{
    finish();
}


//******************************************************************************
//* Public Methods
//******************************************************************************

bool
UwpFirmata::appendSysex(
    uint8_t byte_
    )
{
    if( _sys_command && ( _sys_position < MAX_SYSEX_LEN ) )
    {
        _data_buffer.get()[_sys_position] = byte_;
        ++_sys_position;
        return true;
    }
    return false;
}

int
UwpFirmata::available(
    void
    )
{
    return _firmata_stream->available();
}

void
UwpFirmata::begin(
    Microsoft::Maker::Serial::IStream ^s_
    )
{
    if( s_ == nullptr ) return;

    _firmata_stream = s_;

    //lock the IStream object to guarantee its state won't change while we check if it is already connected.
    _firmata_stream->lock();

    try
    {
        if( _firmata_stream->connectionReady() )
        {
            onConnectionEstablished();
        }
        else
        {
            //we only care about these status changes if the connection is not already established
            _firmata_stream->ConnectionEstablished += ref new Microsoft::Maker::Serial::IStreamConnectionCallback( this, &Microsoft::Maker::Firmata::UwpFirmata::onConnectionEstablished );
            _firmata_stream->ConnectionFailed += ref new Microsoft::Maker::Serial::IStreamConnectionCallbackWithMessage( this, &Microsoft::Maker::Firmata::UwpFirmata::onConnectionFailed );
        }

        //we always care about the connection being lost
        _firmata_stream->ConnectionLost += ref new Microsoft::Maker::Serial::IStreamConnectionCallbackWithMessage( this, &Microsoft::Maker::Firmata::UwpFirmata::onConnectionLost );
        _firmata_stream->unlock();
    }
    catch( Platform::Exception ^e )
    {
        FirmataConnectionFailed( L"An unexpected fatal error occurred in UwpFirmata::begin( IStream ). Message: " + e->Message );
        _firmata_stream->unlock();
    }
}

bool
UwpFirmata::beginSysex(
    uint8_t command_
    )
{
    _sys_command = command_;
    _sys_position = 0;
    return true;
}

bool
UwpFirmata::connectionReady(
    void
    )
{
    return _connection_ready;
}

bool
UwpFirmata::endSysex(
    void
    )
{
    if( _sys_command )
    {
        sendSysex( _sys_command, _sys_position, _data_buffer.get() );
        _sys_command = 0;
        _sys_position = 0;
        return true;
    }
    return false;
}

void
UwpFirmata::finish(
    void
    )
{
    {   //critical section
        std::lock_guard<std::mutex> lock( _firmutex );
        stopThreads();

        _connection_ready = false;
        _firmata_stream = nullptr;
        _sys_command = 0;
        _sys_position = 0;
        _data_buffer = nullptr;

        _firmata_stream->flush();
    }
}

void
UwpFirmata::flush(
    void
    )
{
    return _firmata_stream->flush();
}

void
UwpFirmata::lock(
    void
    )
{
    _firmata_lock.lock();
}

void
UwpFirmata::printVersion(
    void
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    _firmata_stream->write( static_cast<uint8_t>( Command::PROTOCOL_VERSION ) );
    _firmata_stream->write( FIRMATA_PROTOCOL_MAJOR_VERSION );
    _firmata_stream->write( FIRMATA_PROTOCOL_MINOR_VERSION );
    _firmata_stream->flush();
}

void
UwpFirmata::printFirmwareVersion(
    void
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    if( firmwareName )
    {
        _firmata_stream->write( static_cast<uint8_t>( Command::START_SYSEX ) );
        _firmata_stream->write( static_cast<uint8_t>( SysexCommand::REPORT_FIRMWARE ) );
        _firmata_stream->write( firmwareVersionMajor );
        _firmata_stream->write( firmwareVersionMinor );

        for( int i = 0; i < firmwareName->length(); ++i )
        {
            sendAsTwoBytes( firmwareName->at( i ) );
        }

        _firmata_stream->write( static_cast<uint8_t>( Command::END_SYSEX ) );
        _firmata_stream->flush();
    }
}

void
UwpFirmata::processInput(
    void
    )
{
    uint16_t data = _firmata_stream->read();
    if( data == static_cast<uint16_t>( -1 ) ) return;
    
    uint8_t byte = data & 0x00FF;
    uint8_t upper_nibble = data & 0xF0;
    uint8_t lower_nibble = data & 0x0F;

    /*
     * the relevant bits in the command depends on the value of the data byte. If it is less than 0xF0 (start sysex), only the upper nibble identifies the command
     * while the lower nibble contains additional data
     */
    Command command = static_cast<Command>( ( data < static_cast<uint16_t>( Command::START_SYSEX ) ? upper_nibble : byte ) );

    //determine the number of bytes remaining in the message
    size_t bytes_remaining = 0;
    bool isMessageSysex = false;
    switch( command )
    {
    default: //command not understood
    case Command::END_SYSEX: //should never happen
        return;

        //commands that require 2 additional bytes
    case Command::ANALOG_MESSAGE:
    case Command::SET_PIN_MODE:
    case Command::PROTOCOL_VERSION:
        bytes_remaining = 2;
        break;

        //commands that require 1 additional byte
    case Command::DIGITAL_MESSAGE:
    case Command::REPORT_ANALOG_PIN:
    case Command::REPORT_DIGITAL_PIN:
        bytes_remaining = 1;
        break;

        //commands that do not require additional bytes
    case Command::SYSTEM_RESET:
        //do nothing, as there is nothing to reset
        return;

    case Command::START_SYSEX:
        //this is a special case with no set number of bytes remaining
        isMessageSysex = true;
        break;
    }

    //read the remaining message
    std::vector<uint8_t> vector;
    size_t bytes_read = 0;
    while( bytes_remaining || ( isMessageSysex && data != static_cast<uint16_t>( Command::END_SYSEX ) ) )
    {
        data = _firmata_stream->read();
        if( data == static_cast<uint16_t>( -1 ) ) continue;

        vector.push_back( static_cast<uint8_t>( data & 0xFF ) );
        ++bytes_read;
    }

    //process the message
    switch( command )
    {
        //ignore these message types
    default:
    case Command::REPORT_ANALOG_PIN:
    case Command::REPORT_DIGITAL_PIN:
    case Command::SET_PIN_MODE:
    case Command::END_SYSEX:
    case Command::SYSTEM_RESET:
    case Command::PROTOCOL_VERSION:
        return;

    case Command::ANALOG_MESSAGE:
        //report analog commands store the pin number in the lower nibble of the command byte, the value is split over two 7-bit bytes
        AnalogValueUpdated( this, ref new CallbackEventArgs( lower_nibble, vector.at( 0 ) | ( vector.at( 1 ) << 7 ) ) );
        break;

    case Command::DIGITAL_MESSAGE:
        //digital messages store the port number in the lower nibble of the command byte, the port value is split over two 7-bit bytes
        DigitalPortValueUpdated( this, ref new CallbackEventArgs( lower_nibble, vector.at( 0 ) | ( vector.at( 1 ) << 7 ) ) );
        break;

    case Command::START_SYSEX:
        //a sysex message must include at least one extended-command byte
        if( bytes_read < 1 ) return;

        //retrieve the raw data array & extract the extended-command byte
        uint8_t *arr = vector.data();
        SysexCommand sysCommand = static_cast<SysexCommand>( arr[0] );
        ++arr;
        --bytes_read;

        DataWriter ^writer = ref new DataWriter();
        switch( sysCommand )
        {
        case SysexCommand::STRING_DATA:

            //condense back into 1-byte data
            reassembleByteString( arr, bytes_read );

            size_t c;
            size_t wlen = bytes_read / 2;
            wchar_t *wstr_data = new wchar_t[wlen];
            mbstowcs_s( &c, wstr_data, wlen, reinterpret_cast<char *>( arr ), wlen + 1 );

            StringMessageReceived( this, ref new StringCallbackEventArgs( ref new String( wstr_data ) ) );
            delete[]( wstr_data );

        break;

        case SysexCommand::CAPABILITY_RESPONSE:

            //Firmata does not handle capability responses in the typical way (separating bytes), so we write them directly to the DataWriter
            for( int i = 0; i < bytes_read; ++i )
            {
                writer->WriteByte( arr[i] );
            }
            PinCapabilityResponseReceived( this, ref new SysexCallbackEventArgs( static_cast<uint8_t>( sysCommand ), writer->DetachBuffer() ) );

            break;

        case SysexCommand::I2C_REPLY:

            //condense back into 1-byte data
            reassembleByteString( arr, bytes_read );

            //if we're receiving an I2C reply, the first two bytes in our reply are the address and register
            for( int i = 2; i < bytes_read / 2; ++i )
            {
                writer->WriteByte( arr[i] );
            }

            I2cReplyReceived( this, ref new I2cCallbackEventArgs( arr[0], arr[1], writer->DetachBuffer() ) );
            break;

        default:

            //condense back into 1-byte data
            reassembleByteString( arr, bytes_read );

            //we pass the data forward as-is for any other type of sysex command
            for( int i = 0; i < bytes_read / 2; ++i )
            {
                writer->WriteByte( arr[i] );
            }

            SysexMessageReceived( this, ref new SysexCallbackEventArgs( static_cast<uint8_t>( sysCommand ), writer->DetachBuffer() ) );

        }

        break;
    }

    //this library does not support digital write, but we need to consume the rest of the message
}

void
UwpFirmata::setFirmwareNameAndVersion(
    String ^name_,
    uint8_t major_,
    uint8_t minor_
    )
{
    std::wstring nameW = name_->ToString()->Begin();

    {   //critical section
        std::lock_guard<std::mutex> lock( _firmutex );
        if( firmwareName )
        {
            free( firmwareName );
        }

        firmwareName = new std::string( nameW.begin(), nameW.end() );
        firmwareVersionMajor = major_;
        firmwareVersionMinor = minor_;
    }
}

void
UwpFirmata::sendAnalog(
    uint8_t pin_,
    uint16_t value_
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    _firmata_stream->write( static_cast<uint8_t>( Command::ANALOG_MESSAGE ) & ( pin_ & 0x0F ) );
    _firmata_stream->write( static_cast<uint8_t>( value_ & 0x007F ) );
    _firmata_stream->write( static_cast<uint8_t>( ( value_ >> 7 ) & 0x007F ) );
    _firmata_stream->flush();
}


void
UwpFirmata::sendDigitalPort(
    uint8_t port_number_,
    uint8_t port_data_
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    _firmata_stream->write( static_cast<uint8_t>( Command::ANALOG_MESSAGE ) & ( port_number_ & 0x0F ) );
    _firmata_stream->write( port_data_ & 0x007F );
    _firmata_stream->write( port_data_ >> 7 );
    _firmata_stream->flush();
}


void
UwpFirmata::sendString(
    String ^string_
    )
{
    return sendString( static_cast<uint8_t>( SysexCommand::STRING_DATA ), string_);
}


void
UwpFirmata::sendString(
    uint8_t command_,
    String ^string_
    )
{
    std::wstring stringW = string_->ToString()->Begin();
    std::string stringA( stringW.begin(), stringW.end() );
    return ::RawFirmata.sendString(command_, stringA.c_str());
}

void
UwpFirmata::sendValueAsTwo7bitBytes(
    int value_
    )
{
    return ::RawFirmata.sendValueAsTwo7bitBytes(value_);
}

void
UwpFirmata::startListening(
    void
    )
{
    //is a thread currently running?
    if( _input_thread.joinable() ) { return; }

    //prepare communications
    _input_thread_should_exit = false;

    //initialize the new input thread
    _input_thread = std::thread( [ this ]() -> void { inputThread(); } );
}

void
UwpFirmata::unlock(
    void
    )
{
    _firmata_lock.unlock();
}

void
UwpFirmata::write(
    uint8_t c_
    )
{
    _firmata_stream->write( c_ );
}


//******************************************************************************
//* Private Methods
//******************************************************************************


void
UwpFirmata::inputThread(
    void
    )
{
    //set state-tracking member variables and begin processing input
    while( !_input_thread_should_exit )
    {
        try
        {
            processInput();
        }
        catch( Platform::Exception ^e )
        {
            OutputDebugString( e->Message->Begin() );
        }
    }
}

void
UwpFirmata::onConnectionEstablished(
    void
    )
{
    {   //critical section guarantees state is only changed when it is not being modified elsewhere
        std::lock_guard<std::mutex> lock( _firmutex );
        _connection_ready = true;
    }

    FirmataConnectionReady();
}

void
UwpFirmata::onConnectionFailed(
    Platform::String ^message_
    )
{
    FirmataConnectionFailed( message_ );
}

void
UwpFirmata::onConnectionLost(
    Platform::String ^message_
    )
{
    _connection_ready = false;
    FirmataConnectionLost( message_ );
}

void
UwpFirmata::reassembleByteString(
    uint8_t *byte_string_,
    size_t length_
    )
{
    //each char must be reassembled from the two 7-bit bytes received, therefore length should always be an even number.
    int i;
    for( i = 0, j = 0; j < length_ - 1; ++i, j += 2 )
    {
        byte_string_[i] = byte_string_[j] | ( byte_string_[j + 1] << 7 );
    }
    byte_string_[i] = 0;
}

void
UwpFirmata::sendAsTwoBytes(
    uint8_t byte
    )
{
    _firmata_stream->write( byte & 0x7F );
    _firmata_stream->write( byte >> 7 );
}

void
UwpFirmata::sendSysex(
    uint8_t command_,
    uint8_t length_,
    uint8_t *buffer_
    )
{
    //critical section equivalent to function scope
    std::lock_guard<std::mutex> lock( _firmutex );

    _firmata_stream->write( static_cast<uint8_t>( Command::START_SYSEX ) );
    _firmata_stream->write( command_ );

    for( uint8_t i = 0; i < length_; ++i )
    {
        sendAsTwoBytes( buffer_[i] );
    }

    _firmata_stream->write( static_cast<uint8_t>( Command::END_SYSEX ) );
    _firmata_stream->flush();
}

void
UwpFirmata::stopThreads(
    void
    )
{
    _input_thread_should_exit = true;
    if( _input_thread.joinable() ) { _input_thread.join(); }
    _input_thread_should_exit = false;
}
