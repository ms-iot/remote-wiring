
#include "pch.h"
#include "BluetoothSerial.h"
#include <pplawait.h>
#include <chrono>
#include <thread>

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth::Rfcomm;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Microsoft::Maker::Serial;

namespace {
    // Connection code taken from:
    // https://msdn.microsoft.com/en-us/library/windows/apps/xaml/dn264586.aspx

    // This App requires a connection that is encrypted but does not care about
    // whether its authenticated.
    bool SupportsProtection(RfcommDeviceService^ service)
    {
        switch (service->ProtectionLevel)
        {
        case SocketProtectionLevel::PlainSocket:
            if ((SocketProtectionLevel::BluetoothEncryptionWithAuthentication == service->MaxProtectionLevel)
                || (service->MaxProtectionLevel == SocketProtectionLevel::BluetoothEncryptionAllowNullAuthentication))
            {
                // The connection can be upgraded when opening the socket so the
                // App may offer UI here to notify the user that Windows may
                // prompt for a PIN exchange.
                return true;
            }
            else
            {
                // The connection cannot be upgraded so an App may offer UI here
                // to explain why a connection won’t be made.
                return false;
            }
        case SocketProtectionLevel::BluetoothEncryptionWithAuthentication:
            return true;
        case SocketProtectionLevel::BluetoothEncryptionAllowNullAuthentication:
            return true;
        }
        return false;
    }

    // This App relies on CRC32 checking available in version 2.0 of the service.
    bool IsCompatibleVersion(RfcommDeviceService^ service)
    {
        const uint32_t SERVICE_VERSION_ATTRIBUTE_ID = 0x0300;
        const byte SERVICE_VERSION_ATTRIBUTE_TYPE = 0x0A;   // UINT32
        const uint32_t MINIMUM_SERVICE_VERSION = 0x0200;

        create_task(service->GetSdpRawAttributesAsync(
            Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached))
        .then([&](Windows::Foundation::Collections::IMapView<uint32_t, Windows::Storage::Streams::IBuffer ^> ^attributes_){
            Windows::Storage::Streams::DataReader ^reader = DataReader::FromBuffer(attributes_->Lookup(SERVICE_VERSION_ATTRIBUTE_ID));

            // The first byte contains the attribute's type
            byte attributeType = reader->ReadByte();
            if ( attributeType == SERVICE_VERSION_ATTRIBUTE_TYPE )
            {
                // The remainder is the data
                uint32_t version = reader->ReadUInt32();
                return version >= MINIMUM_SERVICE_VERSION;
            }

            return true;
        });

        return true; // bug, returns erroneous result
    }
}  // namespace



//******************************************************************************
//* Constructors
//******************************************************************************

BluetoothSerial::BluetoothSerial(
    void
) :
    _connection_ready(0),
    _device_service(nullptr),
    _rx(nullptr),
    _service_id(nullptr),
    _service_provider(nullptr),
    _stream_socket(nullptr),
    _tx(nullptr),
	_deviceIdentifier(nullptr),
	_device(nullptr)
{
}

BluetoothSerial::BluetoothSerial(
	Platform::String ^deviceIdentifier_
	) :
	_connection_ready(0),
	_device_service(nullptr),
	_rx(nullptr),
	_service_id(nullptr),
	_service_provider(nullptr),
	_stream_socket(nullptr),
	_tx(nullptr),
	_deviceIdentifier(deviceIdentifier_),
	_device(nullptr)
{
}

BluetoothSerial::BluetoothSerial(
	DeviceInformation ^device_
	) :
	_connection_ready(0),
	_device_service(nullptr),
	_rx(nullptr),
	_service_id(nullptr),
	_service_provider(nullptr),
	_stream_socket(nullptr),
	_tx(nullptr),
	_deviceIdentifier(nullptr),
	_device(device_)
{
}

//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
BluetoothSerial::available(
    void
) {
    // Check to see if connection is ready
    if ( !connectionReady() ) { return 0; }

    return _rx->UnconsumedBufferLength;
}

/// \details Immediately discards the incoming parameters, because they are used for standard serial connections and will have no bearing on a bluetooth connection. An Advanced Query String is constructed based upon the service id of the desired device. Then build a collection of all paired devices matching the query.
/// \ref https://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx
void
BluetoothSerial::begin(
    uint32_t baud_,
	SerialConfig config_
) {
    // Discard incoming parameters inherited from IArduinoStream interface.
    UNREFERENCED_PARAMETER(baud_);
    UNREFERENCED_PARAMETER(config_);
    begin(true);
}


void
BluetoothSerial::beginAsync(
    void
	)
{
    begin(false);
}


bool
BluetoothSerial::connectionReady(
void
) {
    return static_cast<bool>(InterlockedAnd(&_connection_ready, true));
}


/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
BluetoothSerial::end(
    void
) {
    InterlockedAnd(&_connection_ready, false);
    delete(_rx); //_rx->Close();
    _rx = nullptr;
    delete(_tx); //_tx->Close();
    _tx = nullptr;
    delete(_stream_socket); //_socket->Close();
    _stream_socket = nullptr;
    _device_service = nullptr;
    _service_id = nullptr;
    _service_provider = nullptr;
}


uint16_t
BluetoothSerial::read(
    void
) {
    uint16_t c = static_cast<uint16_t>(-1);

    if ( available() ) {
         c = _rx->ReadByte();
    }

    // Prefetch buffer
	if ( connectionReady() &&
		_synchronous_mode &&
		!_rx->UnconsumedBufferLength &&
		currentLoadOperation->Status != Windows::Foundation::AsyncStatus::Started )
	{
		//attempt to detect disconnection
		if( currentLoadOperation->Status == Windows::Foundation::AsyncStatus::Error )
		{
			InterlockedAnd( &_connection_ready, false );
			ConnectionLost();
			return -1;
		}
		currentLoadOperation->Close();
		currentLoadOperation = _rx->LoadAsync( 100 );
	}

    return c;
}

uint32_t
BluetoothSerial::write(
    uint8_t c_
) {
    // Check to see if connection is ready
    if ( !connectionReady() ) { return 0; }

	if( ( currentStoreOperation != nullptr ) && currentStoreOperation->Status == Windows::Foundation::AsyncStatus::Error )
	{
		InterlockedAnd( &_connection_ready, false );
		ConnectionLost();
		return 0;
	}

    _tx->WriteByte(c_);
	currentStoreOperation = _tx->StoreAsync();
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

	/*
	 * there are several scenarios here which may involve blocking operations for proper error handling and etc. we need to absolutely guarantee that this can never
	 * execute on the main thread.
	 */ 
	std::thread thr( [ this ] {
		//if a device was specified, attempt to connectAsync to that device and bounce
		if( _device != nullptr )
		{
			if( !connectAsync( _device ).get() )
			{
				ConnectionFailed();
			}
			return;
		}

		//otherwise, we first need to get a list of all possible devices
		Concurrency::create_task( listAvailableDevicesAsync() )
			.then( [ this ]( Windows::Devices::Enumeration::DeviceInformationCollection ^devices )
		{
			if( !devices->Size )
			{
				//no devices found
				ConnectionFailed();
				return;
			}
			_devices = devices;

			//if a device identifier is specified, we will attempt to match one of the devices in the collection to the identifier.
			if( _deviceIdentifier != nullptr )
			{
				for each( auto device in _devices )
				{
					if( device->Id->Equals( _deviceIdentifier ) || device->Name->Equals( _deviceIdentifier ) )
					{
						if( !connectAsync( device ).get() )
						{
							ConnectionFailed();
						}
						return;
					}
				}

				//if we've exhausted the list and found nothing that matches the identifier, we've failed to connectAsync.
				ConnectionFailed();
				return;
			}

			//if no device or device identifier is specified, we try brute-force to connectAsync to each device
			for each( auto device in _devices )
			{
				if( connectAsync( device ).get() )
				{
					return;
				}
			}

			ConnectionFailed();
		} );
	} );
	thr.detach();
}



Concurrency::task<bool>
BluetoothSerial::connectAsync(
Windows::Devices::Enumeration::DeviceInformation ^device_
)
{
	return Concurrency::create_task( Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService::FromIdAsync( device_->Id ) )

		.then( [ this ]( Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^device_service_ )
	{
		_device_service = device_service_;
		_stream_socket = ref new Windows::Networking::Sockets::StreamSocket();

		// Connect the socket
		auto task = _stream_socket->ConnectAsync(
			_device_service->ConnectionHostName,
			_device_service->ConnectionServiceName,
			Windows::Networking::Sockets::SocketProtectionLevel::BluetoothEncryptionAllowNullAuthentication );

		//we need to wait for the above IAsyncAction to complete, creating a task and calling .wait() throws an uncatchable exception on failure.
		while( task->Status == Windows::Foundation::AsyncStatus::Started )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
		}
		if( task->Status == Windows::Foundation::AsyncStatus::Error )
		{
			return false;
		}

		_rx = ref new Windows::Storage::Streams::DataReader( _stream_socket->InputStream );
		if( _synchronous_mode )
		{
			//partial mode will allow for better async reads
			_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;
			currentLoadOperation = _rx->LoadAsync( 100 );
			currentStoreOperation = nullptr;
		}

		_tx = ref new Windows::Storage::Streams::DataWriter( _stream_socket->OutputStream );

		// Set connection ready flag
		InterlockedOr( &_connection_ready, true );

		ConnectionEstablished();
		return true;
	} )

		.then( [ this ]( Concurrency::task<bool> t )
	{
		try
		{
			//if anything in our task chain threw an exception, get() will receive it.
			return t.get();
		}
		catch( ... )
		{
			return false;
		}
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
