
#include "pch.h"
#include "UapFirmata.h"

#include "Firmata.h"
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


UapFirmata::UapFirmata(
	void
	)
{
	_sysCommand = 0;
	_dataBuffer = (uint8_t*)malloc( 31 );
	_sysPosition = 0;
	_blobStarted = false;
	_blobPosition = 0;

	RawFirmata.attach( static_cast<uint8_t>( DIGITAL_MESSAGE ), static_cast<callbackFunction>( std::bind( &UapFirmata::digitalInvoke, this, _1, _2 ) ) );
	RawFirmata.attach( static_cast<uint8_t>( ANALOG_MESSAGE ), static_cast<callbackFunction>( std::bind( &UapFirmata::analogInvoke, this, _1, _2 ) ) );
	RawFirmata.attach( static_cast<uint8_t>( SYSEX_I2C_REPLY ), static_cast<sysexCallbackFunction>( std::bind( &UapFirmata::sysexInvoke, this, _1, _2, _3 ) ) );
}


//******************************************************************************
//* Public Methods
//******************************************************************************


void
UapFirmata::begin(
	Microsoft::Maker::Serial::IStream ^s_
	)
{
	::RawFirmata.begin( s_ );
}

void
UapFirmata::startListening(
	void
	)
{
	//initialize the input thread
	create_async( [ this ]() -> void { inputThread(); } );
}


void
UapFirmata::finish(
	void
	)
{
	stopThreads();
	while( !inputThreadExited );
	return ::RawFirmata.finish();
}


void
UapFirmata::printVersion(
	void
	)
{
	return ::RawFirmata.printVersion();
}


void
UapFirmata::printFirmwareVersion(
	void
	)
{
	return ::RawFirmata.printFirmwareVersion();
}


void
UapFirmata::setFirmwareNameAndVersion(
	String ^name_,
	uint8_t major_,
	uint8_t minor_
	)
{
	std::wstring nameW = name_->ToString()->Begin();
	std::string nameA( nameW.begin(), nameW.end() );
	return ::RawFirmata.setFirmwareNameAndVersion( nameA.c_str(), major_, minor_ );
}


int
UapFirmata::available(
	void
	)
{
	return ::RawFirmata.available();
}


void
UapFirmata::processInput(
	void
	)
{
	return ::RawFirmata.processInput();
}


void
UapFirmata::sendAnalog(
	uint8_t pin_,
	int value_
	)
{
	return ::RawFirmata.sendAnalog( pin_, value_ );
}


void
UapFirmata::sendDigitalPort(
	uint8_t portNumber_,
	int portData_
	)
{
	return ::RawFirmata.sendDigitalPort( portNumber_, portData_ );
}


void
UapFirmata::setDigitalReadEnabled(
	uint8_t portNumber,
	int portData
	)
{
	write( static_cast<uint8_t>( Command::REPORT_DIGITAL_PIN ) | ( portNumber & 0x0F ) );
	write( static_cast<uint8_t>( portData ) );
}


void
UapFirmata::sendString(
	String ^string_
	)
{
	std::wstring stringW = string_->ToString()->Begin();
	std::string stringA( stringW.begin(), stringW.end() );
	return ::RawFirmata.sendString( stringA.c_str() );
}


void
UapFirmata::sendString(
	uint8_t command_,
	String ^string_
	)
{
	std::wstring stringW = string_->ToString()->Begin();
	std::string stringA( stringW.begin(), stringW.end() );
	return ::RawFirmata.sendString( command_, stringA.c_str() );
}


bool
UapFirmata::beginSysex(
	uint8_t command_
	)
{
	if( _blobStarted ) return false;
	_sysCommand = command_;
	_sysPosition = 0;
	return true;
}


bool
UapFirmata::appendSysex(
	uint8_t byte_
	)
{
	if( _sysCommand && ( _sysPosition < MAX_SYSEX_LEN ) && !_blobStarted )
	{
		_dataBuffer[ _sysPosition ] = byte_;
		++_sysPosition;
		return true;
	}
	return false;
}


bool
UapFirmata::endSysex(
	void
	)
{
	if( _sysCommand && !_blobStarted )
	{
		::RawFirmata.sendSysex( _sysCommand, _sysPosition, _dataBuffer );
		_sysCommand = 0;
		_sysPosition = 0;
		return true;
	}
	return false;
}


bool
UapFirmata::beginBlob(
	void
	)
{
	if( _sysCommand ) return false;

	_blobStarted = true;
	_blobPosition = 0;
	return true;
}


bool
UapFirmata::appendBlob(
	uint8_t byte_
	)
{
	if( !_blobStarted || _sysCommand ) return false;

	if( _blobPosition >= MAX_BLOB_LEN )
	{
		endBlob();
		beginBlob();
	}

	_dataBuffer[ _blobPosition ] = byte_ & 0x7F;
	++_blobPosition;
	return true;
}


bool
UapFirmata::endBlob(
	void
	)
{
	if( !_blobStarted || _sysCommand ) return false;

	::RawFirmata.write( static_cast<byte>( START_SYSEX ) );
	::RawFirmata.write( static_cast<byte>( SYSEX_BLOB_COMMAND ) );
	for( uint8_t i = 0; i < _blobPosition; ++i )
	{
		::RawFirmata.write( _dataBuffer[ i ] );
	}
	::RawFirmata.write( static_cast<byte>( END_SYSEX ) );
	_blobPosition = 0;
	_blobStarted = false;
	return true;
}


void
UapFirmata::write(
	uint8_t c_
	)
{
	return ::RawFirmata.write( c_ );
}


void
UapFirmata::enableI2c(
	uint16_t i2cReadDelayMicros_
	)
{
	::RawFirmata.startSysex();
	::RawFirmata.write( static_cast<uint8_t>( I2C_CONFIG ) );
	::RawFirmata.sendValueAsTwo7bitBytes( i2cReadDelayMicros_ );
	::RawFirmata.endSysex();
}


void
UapFirmata::writeI2c(
	uint8_t address_,
	Platform::String ^message_
	)
{
	std::wstring wstr( message_->Begin() );
	std::string str( wstr.begin(), wstr.end() );
	const size_t len = message_->Length();

	sendI2cSysex( address_, 0, 0xFF, len, str.c_str() );
}


void
UapFirmata::readI2c(
	uint8_t address_,
	size_t numBytes_,
	uint8_t reg_,
	bool continuous_
	)
{
	//if you want to do continuous reads, you must provide a register to prompt for new data
	if( continuous_ && ( reg_ == 0xFF ) ) return;
	sendI2cSysex( address_, ( continuous_ ? 0x10 : 0x08 ), reg_, 1, (const char*)&numBytes_ );
}


void
UapFirmata::stopI2c(
	uint8_t address_
	)
{
	sendI2cSysex( address_, 0x18, 0xFF, 0, nullptr );
}


//******************************************************************************
//* Private Methods
//******************************************************************************

void
UapFirmata::sendI2cSysex(
	const uint8_t address_,
	const uint8_t rw_mask_,
	const uint8_t reg_,
	const size_t len,
	const char * data
	)
{
	::RawFirmata.startSysex();
	::RawFirmata.write( static_cast<uint8_t>( I2C_REQUEST ) );
	::RawFirmata.write( address_ );
	::RawFirmata.write( rw_mask_ );

	if( reg_ != 0xFF )
	{
		::RawFirmata.sendValueAsTwo7bitBytes( reg_ );
	}

	for( size_t i = 0; i < len; i++ )
	{
		::RawFirmata.sendValueAsTwo7bitBytes( data[ i ] );
	}
	::RawFirmata.endSysex();
}


void
UapFirmata::inputThread(
	void
	)
{
	if( inputThreadRunning ) return;

	//set state-tracking member variables and begin processing input
	inputThreadRunning = true;
	inputThreadExited = false;
	while( inputThreadRunning )
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
	inputThreadExited = true;
}


void
UapFirmata::stopThreads(
	void
	)
{
	inputThreadRunning = false;
}