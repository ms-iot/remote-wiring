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
#include "Firmata\Firmata.h"
#include <cstdlib>

using namespace Microsoft::Maker::Serial;
using namespace Microsoft::Maker::Firmata;
using namespace std::placeholders;

namespace {
    FirmataClass RawFirmata;
}


//******************************************************************************
//* Constructors
//******************************************************************************


UwpFirmata::UwpFirmata(
    void
) :
    _data_buffer(new uint8_t[31]),
    _firmata_lock(_firmutex, std::defer_lock),
    _firmata_stream(nullptr),
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _input_thread_should_exit(ATOMIC_VAR_INIT(false)),
    _sys_command(0),
    _sys_position(0)
{
    RawFirmata.attach( static_cast<uint8_t>( DIGITAL_MESSAGE ), static_cast<callbackFunction>( std::bind( &UwpFirmata::digitalInvoke, this, _1, _2 ) ) );
    RawFirmata.attach( static_cast<uint8_t>( ANALOG_MESSAGE ), static_cast<callbackFunction>( std::bind( &UwpFirmata::analogInvoke, this, _1, _2 ) ) );
    RawFirmata.attach( static_cast<uint8_t>( SYSEX_I2C_REPLY ), static_cast<sysexCallbackFunction>( std::bind( &UwpFirmata::sysexInvoke, this, _1, _2, _3 ) ) );
    RawFirmata.attach( static_cast<uint8_t>( STRING_DATA ), static_cast<stringCallbackFunction>( std::bind( &UwpFirmata::stringInvoke, this, _1 ) ) );
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
    return ::RawFirmata.available();
}

void
UwpFirmata::begin(
    Microsoft::Maker::Serial::IStream ^s_
    )
{
    if( s_ == nullptr ) return;

    _firmata_stream = s_;
    ::RawFirmata.begin( s_ );

    //lock the IStream object to guarantee its state won't change while we check if it is already connected.
    _firmata_stream->lock();

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
        ::RawFirmata.sendSysex( _sys_command, _sys_position, _data_buffer.get() );
        _firmata_stream->flush();
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
    _firmata_lock.lock();
    _firmata_stream->flush();
    stopThreads();

    _firmata_stream = nullptr;
    _sys_command = 0;
    _sys_position = 0;
    _data_buffer = nullptr;
    _firmata_lock.unlock();
    return ::RawFirmata.finish();
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
    ::RawFirmata.printVersion();
    _firmata_stream->flush();
    return;
}

void
UwpFirmata::printFirmwareVersion(
    void
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    ::RawFirmata.printFirmwareVersion();
    _firmata_stream->flush();
    return;
}

void
UwpFirmata::processInput(
    void
    )
{
    return ::RawFirmata.processInput();
}

void
UwpFirmata::setFirmwareNameAndVersion(
    String ^name_,
    uint8_t major_,
    uint8_t minor_
    )
{
    std::wstring nameW = name_->ToString()->Begin();
    std::string nameA( nameW.begin(), nameW.end() );
    return ::RawFirmata.setFirmwareNameAndVersion( nameA.c_str(), major_, minor_ );
}

void
UwpFirmata::sendAnalog(
    uint8_t pin_,
    uint16_t value_
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    ::RawFirmata.sendAnalog(pin_, value_);
    _firmata_stream->flush();
    return;
}


void
UwpFirmata::sendDigitalPort(
    uint8_t port_number_,
    uint8_t port_data_
    )
{
    std::lock_guard<std::mutex> lock(_firmutex);
    ::RawFirmata.sendDigitalPort(port_number_, port_data_);
    _firmata_stream->flush();
    return;
}


void
UwpFirmata::sendString(
    String ^string_
    )
{
    return sendString(STRING_DATA, string_);
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
    return ::RawFirmata.write( c_ );
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
            ::RawFirmata.processInput();
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
    Platform::String ^message
    )
{
    FirmataConnectionFailed( message );
}

void
UwpFirmata::onConnectionLost(
    Platform::String ^message
    )
{
    _connection_ready = false;
    FirmataConnectionLost( message );
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
