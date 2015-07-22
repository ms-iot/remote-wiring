/*
  Firmata.cpp - Firmata library v2.4.1 - 2015-2-7
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
  */

  //******************************************************************************
  //* Includes
  //******************************************************************************

#include "pch.h"
#include "Firmata.h"

#include <string.h>
#include <stdlib.h>

//******************************************************************************
//* Support Functions
//******************************************************************************

void
FirmataClass::sendValueAsTwo7bitBytes(
    int value
    )
{
    FirmataStream->write( static_cast<byte>( value & 0x7F ) ); // LSB
    FirmataStream->write( static_cast<byte>( ( value >> 7 ) & 0x7F ) ); // MSB
}

void
FirmataClass::startSysex(
    void
    )
{
    FirmataStream->write( static_cast<byte>( START_SYSEX ) );
}

void
FirmataClass::endSysex(
    void
    )
{
    FirmataStream->write( static_cast<byte>( END_SYSEX ) );
}

//******************************************************************************
//* Constructors
//******************************************************************************


FirmataClass::FirmataClass(
    void
    )
{
    firmwareVersionCount = 0;
    firmwareVersionVector = nullptr;
    systemReset();
}

//******************************************************************************
//* Public Methods
//******************************************************************************


void
FirmataClass::begin(
    long speed
    )
{
    FirmataStream->begin( speed, Microsoft::Maker::Serial::SerialConfig::SERIAL_8N1 );
}


void
FirmataClass::begin(
    Microsoft::Maker::Serial::IStream ^s
    )
{
    FirmataStream = s;
}

void
FirmataClass::finish(
    void
    )
{
    return FirmataStream->end();
}


void
FirmataClass::printVersion(
    void
    )
{
    FirmataStream->write( static_cast<byte>( REPORT_VERSION ) );
    FirmataStream->write( static_cast<byte>( FIRMATA_MAJOR_VERSION ) );
    FirmataStream->write( static_cast<byte>( FIRMATA_MINOR_VERSION ) );
}


void
FirmataClass::printFirmwareVersion(
    void
    )
{
    byte i;

    if( firmwareVersionCount )
    { // make sure that the name has been set before reporting
        startSysex();
        FirmataStream->write( static_cast<byte>( REPORT_FIRMWARE ) );
        FirmataStream->write( firmwareVersionVector[ 0 ] ); // major version number
        FirmataStream->write( firmwareVersionVector[ 1 ] ); // minor version number
        for( i = 2; i < firmwareVersionCount; ++i )
        {
            sendValueAsTwo7bitBytes( firmwareVersionVector[ i ] );
        }
        endSysex();
    }
}


void
FirmataClass::setFirmwareNameAndVersion(
    const char *name,
    byte major,
    byte minor
    )
{
    const char *firmwareName;
    const char *extension;

    // parse out ".cpp" and "applet/" that comes from using __FILE__
    extension = strstr( name, ".cpp" );
    firmwareName = strrchr( name, '\\' );

    if( !firmwareName )
    {
        // user passed firmware name
        firmwareName = name;
    }
    else
    {
        ++firmwareName;
    }

    if( !extension )
    {
        firmwareVersionCount = static_cast<byte>( strlen( firmwareName ) + 2 );
    }
    else
    {
        firmwareVersionCount = static_cast<byte>( extension - firmwareName + 2 );
    }

    // in case anyone calls setFirmwareNameAndVersion more than once
    delete[]( firmwareVersionVector );

    firmwareVersionVector = new byte[ firmwareVersionCount + 1 ](); // create and initialize to zero
    firmwareVersionVector[ 0 ] = major;
    firmwareVersionVector[ 1 ] = minor;
    strncpy_s( (char *)firmwareVersionVector + 2, firmwareVersionCount - 2, firmwareName, firmwareVersionCount - 2 );
}


int
FirmataClass::available(
    void
    )
{
    return FirmataStream->available();
}


void
FirmataClass::processSysexMessage(
    void
    )
{
    switch( storedInputData[ 0 ] )
    { //first byte in buffer is command
    case REPORT_FIRMWARE:
        printFirmwareVersion();
        break;
    case STRING_DATA:
        if( currentStringCallback )
        {
            byte bufferLength = ( sysexBytesRead - 1 ) / 2;
            byte i = 1;
            byte j = 0;
            while( j < bufferLength )
            {
                // The string length will only be at most half the size of the
                // stored input buffer so we can decode the string within the buffer.
                storedInputData[ j ] = storedInputData[ i ];
                i++;
                storedInputData[ j ] += ( storedInputData[ i ] << 7 );
                i++;
                j++;
            }
            // Make sure string is null terminated. This may be the case for data
            // coming from client libraries in languages that don't null terminate
            // strings.
            if( storedInputData[ j - 1 ] != '\0' )
            {
                storedInputData[ j ] = '\0';
            }
            currentStringCallback( (uint8_t *)&storedInputData[ 0 ] );
        }
        break;
    default:
        if( currentSysexCallback )
            currentSysexCallback( storedInputData[ 0 ], sysexBytesRead - 1, storedInputData + 1 );
    }
}


void
FirmataClass::processInput(
    void
    )
{
    unsigned short inputData = FirmataStream->read();
    int command;

    if( inputData == 0xFFFF ) return; //TODO add delay for better performance

    if( parsingSysex )
    {
        if( inputData == END_SYSEX )
        {
            //stop sysex byte
            parsingSysex = false;
            //fire off handler function
            processSysexMessage();
        }
        else
        {
            //normal data byte - add to buffer
            storedInputData[ sysexBytesRead ] = static_cast<byte>( inputData );
            sysexBytesRead++;
        }
    }
    else if( ( waitForData > 0 ) && ( inputData < 128 ) )
    {
        waitForData--;
        storedInputData[ waitForData ] = static_cast<byte>( inputData );
        if( ( waitForData == 0 ) && executeMultiByteCommand )
        { // got the whole message
            switch( executeMultiByteCommand )
            {
            case ANALOG_MESSAGE:
                if( currentAnalogCallback )
                {
                    currentAnalogCallback( multiByteChannel,
                        ( storedInputData[ 0 ] << 7 )
                        + storedInputData[ 1 ] );
                }
                break;
            case DIGITAL_MESSAGE:
                if( currentDigitalCallback )
                {
                    currentDigitalCallback( multiByteChannel,
                        ( storedInputData[ 0 ] << 7 )
                        + storedInputData[ 1 ] );
                }
                break;
            case SET_PIN_MODE:
                if( currentPinModeCallback )
                    currentPinModeCallback( storedInputData[ 1 ], storedInputData[ 0 ] );
                break;
            case REPORT_ANALOG:
                if( currentReportAnalogCallback )
                    currentReportAnalogCallback( multiByteChannel, storedInputData[ 0 ] );
                break;
            case REPORT_DIGITAL:
                if( currentReportDigitalCallback )
                    currentReportDigitalCallback( multiByteChannel, storedInputData[ 0 ] );
                break;
            }
            executeMultiByteCommand = 0;
        }
    }
    else
    {
        // remove channel info from command byte if less than 0xF0
        if( inputData < 0xF0 )
        {
            command = inputData & 0xF0;
            multiByteChannel = inputData & 0x0F;
        }
        else
        {
            command = inputData;
            // commands in the 0xF* range don't use channel data
        }
        switch( command )
        {
        case ANALOG_MESSAGE:
        case DIGITAL_MESSAGE:
        case SET_PIN_MODE:
            waitForData = 2; // two data bytes needed
            executeMultiByteCommand = command;
            break;
        case REPORT_ANALOG:
        case REPORT_DIGITAL:
            waitForData = 1; // one data byte needed
            executeMultiByteCommand = command;
            break;
        case START_SYSEX:
            parsingSysex = true;
            sysexBytesRead = 0;
            break;
        case SYSTEM_RESET:
            systemReset();
            break;
        case REPORT_VERSION:
            //do nothing
            break;
        }
    }
}


void
FirmataClass::sendAnalog(
    byte pin,
    int value
    )
{
    // pin can only be 0-15, so chop higher bits
    FirmataStream->write( static_cast<byte>( ANALOG_MESSAGE | ( pin & 0xF ) ) );
    sendValueAsTwo7bitBytes( value );
}


void
FirmataClass::sendDigitalPort(
    byte portNumber,
    int portData
    )
{
    FirmataStream->write( static_cast<byte>( DIGITAL_MESSAGE | ( portNumber & 0xF ) ) );
    sendValueAsTwo7bitBytes( portData );
}


void
FirmataClass::sendSysex(
    byte command,
    byte bytec,
    byte *bytev
    )
{
    byte i;
    startSysex();
    FirmataStream->write( command );
    for( i = 0; i < bytec; i++ )
    {
        sendValueAsTwo7bitBytes( bytev[ i ] );
    }
    endSysex();
}


void
FirmataClass::sendString(
    byte command,
    const char *string
    )
{
    sendSysex( command, static_cast<byte>( strlen( string ) ), reinterpret_cast<byte *>( const_cast<char *>( string ) ) );
}


void
FirmataClass::sendString(
    const char *string
    )
{
    sendString( STRING_DATA, string );
}


void
FirmataClass::write(
    byte c
    )
{
    FirmataStream->write( c );
}


void
FirmataClass::attach(
    byte command,
    callbackFunction newFunction
    )
{
    switch( command )
    {
    case ANALOG_MESSAGE: currentAnalogCallback = newFunction; break;
    case DIGITAL_MESSAGE: currentDigitalCallback = newFunction; break;
    case REPORT_ANALOG: currentReportAnalogCallback = newFunction; break;
    case REPORT_DIGITAL: currentReportDigitalCallback = newFunction; break;
    case SET_PIN_MODE: currentPinModeCallback = newFunction; break;
    }
}


void
FirmataClass::attach(
    byte command,
    systemResetCallbackFunction newFunction
    )
{
    switch( command )
    {
    case SYSTEM_RESET: currentSystemResetCallback = newFunction; break;
    }
}


void
FirmataClass::attach(
    byte command,
    stringCallbackFunction newFunction
    )
{
    switch( command )
    {
    case STRING_DATA: currentStringCallback = newFunction; break;
    }
}


void
FirmataClass::attach(
    byte command,
    sysexCallbackFunction newFunction
    )
{
    currentSysexCallback = newFunction;
}


void
FirmataClass::detach(
    byte command
    )
{
    switch( command )
    {
    case SYSTEM_RESET:
        currentSystemResetCallback = static_cast<systemResetCallbackFunction>( nullptr );
        break;
    case STRING_DATA:
        currentStringCallback = static_cast<stringCallbackFunction>( nullptr );
        break;
    case START_SYSEX:
        currentSysexCallback = static_cast<sysexCallbackFunction>( nullptr );
        break;
    default:
        attach( command, static_cast<callbackFunction>( nullptr ) );
    }
}

//******************************************************************************
//* Private Methods
//******************************************************************************


void
FirmataClass::systemReset(
    void
    )
{
    byte i;

    waitForData = 0; // this flag says the next serial input will be data
    executeMultiByteCommand = 0; // execute this after getting multi-byte data
    multiByteChannel = 0; // channel data for multiByteCommands

    for( i = 0; i < MAX_DATA_BYTES; i++ )
    {
        storedInputData[ i ] = 0;
    }

    parsingSysex = false;
    sysexBytesRead = 0;

    if( currentSystemResetCallback )
        currentSystemResetCallback();
}