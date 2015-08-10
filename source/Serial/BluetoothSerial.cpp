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

using namespace Concurrency;
using namespace Windows::Devices::Bluetooth::Rfcomm;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;

using namespace Microsoft::Maker::Serial;

//******************************************************************************
//* Constructors
//******************************************************************************

BluetoothSerial::BluetoothSerial(
    Platform::String ^device_name_
    ) :
    _connection_ready( ATOMIC_VAR_INIT(false)),
    _bluetooth_lock(_blutex, std::defer_lock),
    _current_load_operation(nullptr),
    _device(nullptr),
    _device_collection(nullptr),
    _device_name(device_name_),
    _rfcomm_service(nullptr),
    _rx(nullptr),
    _stream_socket(nullptr),
    _tx(nullptr)
{
}

BluetoothSerial::BluetoothSerial(
    DeviceInformation ^device_
    ) :
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _bluetooth_lock(_blutex, std::defer_lock),
    _current_load_operation(nullptr),
    _device(device_),
    _device_collection(nullptr),
    _device_name(nullptr),
    _rfcomm_service(nullptr),
    _rx(nullptr),
    _stream_socket(nullptr),
    _tx(nullptr)
{
}

//******************************************************************************
//* Destructors
//******************************************************************************

BluetoothSerial::~BluetoothSerial(
    void
    )
{
    //we will fire the ConnectionLost event in the case that this object is unexpectedly destructed while the connection is established.
    if( connectionReady() )
    {
        ConnectionLost( L"Your connection has been terminated. The Microsoft::Maker::Serial::BluetoothSerial destructor was called unexpectedly." );
    }
    end();
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
    if (!connectionReady()) {
        return 0;
    }
    else {
        return _rx->UnconsumedBufferLength;
    }
}

/// \details Immediately discards the incoming parameters, because they are used for standard serial connections and will have no bearing on a bluetooth connection.
/// \warning Must be called from the UI thread
void
BluetoothSerial::begin(
    uint32_t baud_,
    SerialConfig config_
    )
{
    // Discard incoming parameters inherited from IStream interface.
    UNREFERENCED_PARAMETER(baud_);
    UNREFERENCED_PARAMETER(config_);

    // Ensure known good state
    end();

    // Although this path is not optimal, the behavior (the calls must be made from the UI thread) is mandated by the bluetooth API. The algorithm is a compromise to provide the succint, maintainable code.
    Concurrency::create_task(listAvailableDevicesAsync())
        .then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^device_collection_)
    {
        if( device_collection_ == nullptr )
        {
            throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"Unable to enumerate available devices. Did you forget your Bluetooth device capabilities in the manifest? DeviceInformation::FindAllAsync returned null." ) );
        }

        // If a friendly name was specified, then identify the associated device
        if (_device_name) {
            // Store parameter as a member to ensure the duration of object allocation
            _device_collection = device_collection_;
            if (!_device_collection->Size)
            {
                throw ref new Platform::Exception(E_UNEXPECTED, L"No Bluetooth devices found or Bluetooth is disabled.");
            }

            _device = identifyDeviceFromCollection(_device_collection);
        }

        if (!_device) {
            throw ref new Platform::Exception(E_UNEXPECTED, L"ERROR! Hacking too much time!");
        }

        return connectToDeviceAsync(_device);
    }).then([this](Concurrency::task<void> t)
    {
        try
        {
            t.get();
        }
        catch (Platform::Exception ^e)
        {
            ConnectionFailed(L"BluetoothSerial::connectToDeviceAsync failed with a Platform::Exception type. (message: " + e->Message + L")");
        }
        catch (...)
        {
            ConnectionFailed(L"BluetoothSerial::connectToDeviceAsync failed with a non-Platform::Exception type. (name: " + _device_name + L")");
        }
    });
}

bool
BluetoothSerial::connectionReady(
    void
    )
{
    return _connection_ready;
}

/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
BluetoothSerial::end(
    void
    )
{
    _connection_ready = false;
    _current_load_operation = nullptr;

    // Reset with respect to dependencies
    delete(_rx); //_rx->Close();
    _rx = nullptr;
    delete(_tx); //_tx->Close();
    _tx = nullptr;
    delete(_stream_socket); //_socket->Close();
    _stream_socket = nullptr;
    _rfcomm_service = nullptr;
    _device_collection = nullptr;
}

void
BluetoothSerial::flush(
    void
    )
{
    auto async_operation_ = _tx->StoreAsync();
    create_task( _tx->StoreAsync() )
        .then( [ this, async_operation_ ]( unsigned int value_ )
    {
        UNREFERENCED_PARAMETER( value_ );

        //detect disconnection
        if( async_operation_->Status == Windows::Foundation::AsyncStatus::Error )
        {
            throw ref new Platform::Exception( E_UNEXPECTED );
        }

        return create_task( _tx->FlushAsync() );
    } )
        .then( [ this ]( task<bool> task_ )
    {
        try
        {
            task_.wait();
        }
        catch( Platform::Exception ^e )
        {
            _connection_ready = false;
            ConnectionLost( L"A fatal error occurred in BluetoothSerial::flush(). Your connection has been lost. Error: " + e->Message );
        }
    } );
}

/// \details An Advanced Query String is constructed based upon paired bluetooth devices. Then a collection is returned of all devices matching the query.
/// \ref https://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx
/// \warning Must be called from UI thread
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
BluetoothSerial::listAvailableDevicesAsync(
    void
    )
{
    // Construct AQS String from service id of desired device
    Platform::String ^device_aqs = Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService::GetDeviceSelector(Windows::Devices::Bluetooth::Rfcomm::RfcommServiceId::SerialPort);

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(device_aqs);
}

void
BluetoothSerial::lock(
    void
    )
{
    _bluetooth_lock.lock();
}

uint16_t
BluetoothSerial::read(
    void
    )
{
    uint16_t c = static_cast<uint16_t>(-1);

    if ( !connectionReady() ) {
        return c;
    }

    if ( available() ) {
        c = _rx->ReadByte();
    }
    else if ( _current_load_operation->Status != Windows::Foundation::AsyncStatus::Started ) {
        // Attempt to detect disconnection
        if (_current_load_operation->Status == Windows::Foundation::AsyncStatus::Error)
        {
            _connection_ready = false;
            ConnectionLost( L"A fatal error has occurred in BluetoothSerial::read() and your connection has been lost." );
            return -1;
        }

        _current_load_operation = _rx->LoadAsync( MAX_READ_SIZE );
    }

    return c;
}

void
BluetoothSerial::unlock(
    void
    )
{
    _bluetooth_lock.unlock();
}

uint32_t
BluetoothSerial::write(
    uint8_t c_
    )
{
    // Check to see if connection is ready
    if ( !connectionReady() ) { return 0; }

    _tx->WriteByte(c_);
    return 1;
}

//******************************************************************************
//* Private Methods
//******************************************************************************

Concurrency::task<void>
BluetoothSerial::connectToDeviceAsync(
    Windows::Devices::Enumeration::DeviceInformation ^device_
    )
{
    _device_name = device_->Name;  // Update name in case device was specified directly
    _stream_socket = ref new StreamSocket();
    return Concurrency::create_task(Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService::FromIdAsync(device_->Id))
        .then([this](Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^rfcomm_service_)
    {
        if( rfcomm_service_ == nullptr )
        {
            throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"Unable to initialize the device. RfcommDeviceService::FromIdAsync returned null." ) );
        }

        // Store parameter as a member to ensure the duration of object allocation
        _rfcomm_service = rfcomm_service_;

        // Connect the socket
        return Concurrency::create_task(_stream_socket->ConnectAsync(
            _rfcomm_service->ConnectionHostName,
            _rfcomm_service->ConnectionServiceName,
            Windows::Networking::Sockets::SocketProtectionLevel::BluetoothEncryptionAllowNullAuthentication))
            .then([this]()
        {
            // Enable RX
            _rx = ref new Windows::Storage::Streams::DataReader(_stream_socket->InputStream);
            _rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;  // Partial mode will allow for better async reads
            _current_load_operation = _rx->LoadAsync( MAX_READ_SIZE );

            // Enable TX
            _tx = ref new Windows::Storage::Streams::DataWriter(_stream_socket->OutputStream);

            // Set connection ready flag
            _connection_ready = true;
            ConnectionEstablished();
        });
    });
}

Windows::Devices::Enumeration::DeviceInformation ^
BluetoothSerial::identifyDeviceFromCollection(
    Windows::Devices::Enumeration::DeviceInformationCollection ^devices_
    )
{
    for (auto &&device : devices_)
    {
        if (device->Id->Equals(_device_name) || device->Name->Equals(_device_name))
        {
            return device;
        }
    }

    // If we searched and found nothing that matches the identifier, we've failed to connect and cannot recover.
    throw ref new Platform::Exception(E_INVALIDARG, L"No Bluetooth devices found matching the specified identifier.");
}
