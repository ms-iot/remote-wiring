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
#include "BluetoothSerial.h"

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth::Rfcomm;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Microsoft::Maker::Serial;

//******************************************************************************
//* Constructors
//******************************************************************************

BluetoothSerial::BluetoothSerial(
	void
	) :
	_connection_ready( 0 ),
	_rx( nullptr ),
	_current_load_operation( nullptr ),
	_current_store_operation( nullptr ),
	_tx( nullptr ),
	_deviceIdentifier( nullptr ),
	_device( nullptr )
{
}

BluetoothSerial::BluetoothSerial(
	Platform::String ^deviceIdentifier_
	) :
	_connection_ready( 0 ),
	_rx( nullptr ),
	_current_load_operation( nullptr ),
	_current_store_operation( nullptr ),
	_tx( nullptr ),
	_deviceIdentifier( deviceIdentifier_ ),
	_device( nullptr )
{
}

BluetoothSerial::BluetoothSerial(
	DeviceInformation ^device_
	) :
	_connection_ready( 0 ),
	_rx( nullptr ),
	_current_load_operation( nullptr ),
	_current_store_operation( nullptr ),
	_tx( nullptr ),
	_deviceIdentifier( nullptr ),
	_device( device_ )
{
}

//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
BluetoothSerial::available(
	void
	)
{
	// Check to see if connection is ready
	if( !connectionReady() ) { return 0; }

	return _rx->UnconsumedBufferLength;
}

/// \details Immediately discards the incoming parameters, because they are used for standard serial connections and will have no bearing on a bluetooth connection. An Advanced Query String is constructed based upon the service id of the desired device. Then build a collection of all paired devices matching the query.
/// \ref https://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx
void
BluetoothSerial::begin(
	uint32_t baud_,
	SerialConfig config_
	)
{
	// Discard incoming parameters inherited from IStream interface.
	UNREFERENCED_PARAMETER( baud_ );
	UNREFERENCED_PARAMETER( config_ );
	begin( true );
}


void
BluetoothSerial::beginAsync(
	void
	)
{
	begin( false );
}


bool
BluetoothSerial::connectionReady(
	void
	)
{
	return static_cast<bool>( InterlockedAnd( &_connection_ready, true ) );
}


/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
BluetoothSerial::end(
	void
	)
{
	InterlockedAnd( &_connection_ready, false );
	delete( _rx ); //_rx->Close();
	_rx = nullptr;
	delete( _tx ); //_tx->Close();
	_tx = nullptr;
	_current_load_operation = nullptr;
	_current_store_operation = nullptr;
}


uint16_t
BluetoothSerial::read(
	void
	)
{
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
BluetoothSerial::write(
	uint8_t c_
	)
{
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


//******************************************************************************
//* Private Methods
//******************************************************************************


void
BluetoothSerial::begin(
	bool synchronous_mode_
	)
{
	_synchronous_mode = synchronous_mode_;

	// Ensure known good state
	end();

	if ( _device != nullptr )
	{
		connectAsync( _device )
		.then([this]( Concurrency::task<void> t )
		{
			try
			{
				t.get();
			}
			catch( Platform::Exception ^e )
			{
				ConnectionFailed( ref new Platform::String( L"BluetoothSerial::connectAsync failed with a Platform::Exception type. Message: " ) + e->Message );
			}
			catch( ... )
			{
				ConnectionFailed( ref new Platform::String( L"BluetoothSerial::connectAsync failed with a non-Platform::Exception type." ) );
			}
		});
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
				throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"No bluetooth devices found." ) );
			}

			//if a device identifier is specified, we will attempt to match one of the devices in the collection to the identifier.
			if( _deviceIdentifier != nullptr )
			{
				Windows::Devices::Enumeration::DeviceInformation ^device = identifyDevice( devices );
				if( device != nullptr )
				{
					return connectAsync( device );
				}
				else
				{
					//if we searched and found nothing that matches the identifier, we've failed to connect.
					throw ref new Platform::Exception( E_INVALIDARG, ref new Platform::String( L"No bluetooth devices found matching the specified identifier." ) );
				}
			}

			//if no device or device identifier is specified, we try brute-force to connectAsync to each device
			// start with a "failed" device. This will never be passed on, since we guarantee above that there is at least one device.
			auto t = Concurrency::task_from_exception<void>( ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"ERROR! Hacking too much time!" ) ) );
			for each( auto device in devices )
			{
				t = t.then([this,device] (Concurrency::task<void> t) {
					try
					{
						t.get();
						return Concurrency::task_from_result();
					}
					catch (...)
					{
						return connectAsync( device );
					}
				});
			}
			return t;
		}).then( [this] ( Concurrency::task<void> t )
		{
			try
			{
				t.get();
			}
			catch( Platform::Exception ^e )
			{
				ConnectionFailed( ref new Platform::String( L"BluetoothSerial::connectAsync failed with a Platform::Exception type. Message: " ) + e->Message );
			}
			catch( ... )
			{
				ConnectionFailed( ref new Platform::String( L"BluetoothSerial::connectAsync failed with a non-Platform::Exception type." ) );
			}
		});
	}
}



Concurrency::task<void>
BluetoothSerial::connectAsync(
	Windows::Devices::Enumeration::DeviceInformation ^device_
	)
{
	return Concurrency::create_task( Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService::FromIdAsync( device_->Id ) )
	.then( [ this ]( Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^device_service_ )
	{
		_stream_socket = ref new Windows::Networking::Sockets::StreamSocket();

		// Connect the socket
		return Concurrency::create_task( _stream_socket->ConnectAsync(
			device_service_->ConnectionHostName,
			device_service_->ConnectionServiceName,
			Windows::Networking::Sockets::SocketProtectionLevel::BluetoothEncryptionAllowNullAuthentication ) );
	} ).then( [ this ]( )
	{
		_rx = ref new Windows::Storage::Streams::DataReader( _stream_socket->InputStream );
		if( _synchronous_mode )
		{
			//partial mode will allow for better async reads
			_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;
			_current_load_operation = _rx->LoadAsync( 100 );
			_current_store_operation = nullptr;
		}

		_tx = ref new Windows::Storage::Streams::DataWriter( _stream_socket->OutputStream );

		// Set connection ready flag
		InterlockedOr( &_connection_ready, true );
		ConnectionEstablished();
	} );
}


Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
BluetoothSerial::listAvailableDevicesAsync(
	void
	)
{
	// Construct AQS String from service id of desired device
	Platform::String ^device_aqs = Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService::GetDeviceSelector( Windows::Devices::Bluetooth::Rfcomm::RfcommServiceId::SerialPort );

	// Identify all paired devices satisfying query
	return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync( device_aqs );
}


Windows::Storage::Streams::DataReaderLoadOperation ^
BluetoothSerial::loadAsync(
	unsigned int count_
	)
{
	//TODO: Determine how to return an empty DataReaderLoadOperation when the connection is unavailable or synchronous mode is enabled
	return _rx->LoadAsync( count_ );
}



Windows::Devices::Enumeration::DeviceInformation ^
BluetoothSerial::identifyDevice(
	Windows::Devices::Enumeration::DeviceInformationCollection ^devices_
	)
{
	for each( const auto device in devices_ )
	{
		if( device->Id->Equals( _deviceIdentifier->ToString() ) || device->Name->Equals( _deviceIdentifier ) )
		{
			return device;
		}
	}
	return nullptr;
}