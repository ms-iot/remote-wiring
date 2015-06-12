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
#include "UsbSerial.h"

#include <string>

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Microsoft::Maker::Serial;

//******************************************************************************
//* Constructors
//******************************************************************************

UsbSerial::UsbSerial(
	Windows::Devices::Enumeration::DeviceInformation ^deviceInfo_
	) :
	_baud( 9600 ),
	_config( SerialConfig::SERIAL_8N1 ),
	_connection_ready( 0 ),
	_device( deviceInfo_ ),
	_vid( nullptr ),
	_pid( nullptr ),
	_rx( nullptr ),
	_tx( nullptr )
{
}

UsbSerial::UsbSerial(
	Platform::String ^vid_
	) :
	_baud( 9600 ),
	_config( SerialConfig::SERIAL_8N1 ),
	_connection_ready( 0 ),
	_device( nullptr ),
	_vid( vid_ ),
	_pid( nullptr ),
	_rx( nullptr ),
	_tx( nullptr )
{
}

UsbSerial::UsbSerial(
	Platform::String ^vid_,
	Platform::String ^pid_
	) :
	_baud( 9600 ),
	_config( SerialConfig::SERIAL_8N1 ),
	_connection_ready( 0 ),
	_device( nullptr ),
	_vid( vid_ ),
	_pid( pid_ ),
	_rx( nullptr ),
	_tx( nullptr )
{
}

//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
UsbSerial::available(
	void
	) {
	// Check to see if connection is ready
	if( !connectionReady() ) { return 0; }

	return _rx->UnconsumedBufferLength;
}

/// \details Immediately discards the incoming parameters, because they are used for standard serial connections and will have no bearing on a bluetooth connection. An Advanced Query String is constructed based upon the service id of the desired device. Then build a collection of all paired devices matching the query.
/// \ref https://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx
void
UsbSerial::begin(
	uint32_t baud_,
	SerialConfig config_
	) {
	_baud = baud_;
	_config = config_;
	begin( true );
}

void
UsbSerial::begin(
	bool synchronous_mode_
	) {
	_synchronous_mode = synchronous_mode_;

	// Ensure known good state
	end();

	if( _device != nullptr )
	{
		connectAsync( _device )
			.then( [ this ]( Concurrency::task<void> t )
		{
			try
			{
				t.get();
			}
			catch( Platform::Exception ^e )
			{
				ConnectionFailed( ref new Platform::String( L"UsbSerial::connectAsync failed with a Platform::Exception type. Message: " ) + e->Message );
			}
			catch( ... )
			{
				ConnectionFailed( ref new Platform::String( L"UsbSerial::connectAsync failed with a non-Platform::Exception type." ) );
			}
		} );
	}
	else
	{
		//otherwise, we first need to get a list of all possible devices
		Concurrency::create_task( listAvailableDevicesAsync() )
			.then( [ this ]( Windows::Devices::Enumeration::DeviceInformationCollection ^devices )
		{
			if( !devices->Size )
			{
				//no devices found
				throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"No USB devices found." ) );
			}

			//if at least a VID was specified, we will attempt to match one of the devices in the collection
			if( _vid != nullptr )
			{
				Windows::Devices::Enumeration::DeviceInformation ^device = identifyDevice( devices );
				if( device != nullptr )
				{
					return connectAsync( device );
				}
				else
				{
					//Requested device not found
					throw ref new Platform::Exception( E_INVALIDARG, ref new Platform::String( L"No USB devices found matching the specified identifier." ) );
				}
			}

			//if no device or device identifier is specified, we try brute-force to connectAsync to each device
			// start with a "failed" device. This will never be passed on, since we guarantee above that there is at least one device.
			auto t = Concurrency::task_from_exception<void>( ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"ERROR! Hacking too much time!" ) ) );
			for each( auto device in devices )
			{
				t = t.then( [ this, device ]( Concurrency::task<void> t ) {
					try
					{
						t.get();
						return Concurrency::task_from_result();
					}
					catch( ... )
					{
						return connectAsync( device );
					}
				} );
			}
			return t;
		} )
			.then( [ this ]( Concurrency::task<void> t )
		{
			try
			{
				//if anything in our task chain threw an exception, get() will receive it.
				return t.get();
			}
			catch( Platform::Exception ^e )
			{
				ConnectionFailed( ref new Platform::String( L"UsbSerial::connectAsync failed with a Platform::Exception type. Message: " ) + e->Message );
			}
			catch( ... )
			{
				ConnectionFailed( ref new Platform::String( L"UsbSerial::connectAsync failed with a non-Platform::Exception type." ) );
			}
		} );
	}
}


Concurrency::task<void>
UsbSerial::connectAsync(
	Windows::Devices::Enumeration::DeviceInformation ^device_
	)
{
	return Concurrency::create_task( Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync( device_->Id ) )
		.then( [ this ]( Windows::Devices::SerialCommunication::SerialDevice ^serial_device_ )
	{
		if( serial_device_ == nullptr )
		{
			throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"Unable to initialize the SerialDevice from the specified identifiers." ) );
		}

		serial_device_->Handshake = SerialHandshake::None;
		serial_device_->BaudRate = _baud;
		switch( _config ) {
		case SerialConfig::SERIAL_5E1:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_5E2:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_5N1:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_5N2:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_5O1:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_5O2:
			serial_device_->DataBits = 5;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_6E1:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_6E2:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_6N1:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_6N2:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_6O1:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_6O2:
			serial_device_->DataBits = 6;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_7E1:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_7E2:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_7N1:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_7N2:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_7O1:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_7O2:
			serial_device_->DataBits = 7;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_8E1:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_8E2:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::Even;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_8N1:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_8N2:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::None;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		case SerialConfig::SERIAL_8O1:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::One;
			break;
		case SerialConfig::SERIAL_8O2:
			serial_device_->DataBits = 8;
			serial_device_->Parity = SerialParity::Odd;
			serial_device_->StopBits = SerialStopBitCount::Two;
			break;
		}

		_rx = ref new Windows::Storage::Streams::DataReader( serial_device_->InputStream );
		if( _synchronous_mode ) {
			_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;
			_current_load_operation = _rx->LoadAsync( 100 );
			_current_store_operation = nullptr;
		}

		_tx = ref new Windows::Storage::Streams::DataWriter( serial_device_->OutputStream );

		//if no exception was thrown, connection was successful. set connection ready flag
		InterlockedOr( &_connection_ready, true );
		ConnectionEstablished();
	} );
}


void
UsbSerial::beginAsync(
	void
	) {
	begin( false );
}

bool
UsbSerial::connectionReady(
	void
	) {
	return static_cast<bool>( InterlockedAnd( &_connection_ready, true ) );
}

/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
UsbSerial::end(
	void
	) {
	InterlockedAnd( &_connection_ready, false );
	delete( _rx ); //_rx->Close();
	_rx = nullptr;
	delete( _tx ); //_tx->Close();
	_tx = nullptr;
	_current_load_operation = nullptr;
	_current_store_operation = nullptr;
	_device = nullptr;
}


Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
UsbSerial::listAvailableDevicesAsync(
	void
	) {

	// Construct AQS String from service id of desired device
	Platform::String ^device_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();

	// Identify all paired devices satisfying query
	return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync( device_aqs );
}

Windows::Storage::Streams::DataReaderLoadOperation ^
UsbSerial::loadAsync(
	unsigned int count_
	) {
	//TODO: Determine how to return an empty DataReaderLoadOperation when the connection is unavailable or synchronous mode is enabled
	return _rx->LoadAsync( count_ );
}


uint16_t
UsbSerial::read(
	void
	) {
	uint16_t c = static_cast<uint16_t>( -1 );

	if( available() ) {
		c = _rx->ReadByte();
	}

	// Prefetch buffer
	if( connectionReady() &&
		_synchronous_mode &&
		!_rx->UnconsumedBufferLength &&
		_current_load_operation->Status != Windows::Foundation::AsyncStatus::Started )
	{
		//attempt to detect disconnection
		if( _current_load_operation->Status == Windows::Foundation::AsyncStatus::Error )
		{
			InterlockedAnd( &_connection_ready, false );
			ConnectionLost();
			return -1;
		}
		_current_load_operation->Close();
		_current_load_operation = _rx->LoadAsync( 100 );
	}

	return c;
}

uint32_t
UsbSerial::write(
	uint8_t c_
	) {
	// Check to see if connection is ready
	if( !connectionReady() ) { return 0; }

	if( ( _current_store_operation != nullptr ) && _current_store_operation->Status == Windows::Foundation::AsyncStatus::Error )
	{
		InterlockedAnd( &_connection_ready, false );
		ConnectionLost();
		return 0;
	}

	_tx->WriteByte( c_ );
	_current_store_operation = _tx->StoreAsync();
	return 1;
}


Windows::Devices::Enumeration::DeviceInformation ^
UsbSerial::identifyDevice(
	Windows::Devices::Enumeration::DeviceInformationCollection ^devices_
	)
{
	for( unsigned int i = 0; i < devices_->Size; ++i )
	{
		//if the vid doesn't match, move to the next device
		if( std::string::npos == std::wstring( devices_->GetAt( i )->Id->Data() ).find( _vid->Data() ) )
		{
			continue;
		}
		
		//if the pid doesn't match, move to the next device
		if( std::string::npos == std::wstring( devices_->GetAt( i )->Id->Data() ).find( _pid->Data() ) )
		{
			continue;
		}

		//if both match, we've identified the device;
		return devices_->GetAt( i );
	}
	return nullptr;
}