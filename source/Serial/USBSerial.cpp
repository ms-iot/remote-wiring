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

using namespace Concurrency;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;

using namespace Microsoft::Maker::Serial;

//******************************************************************************
//* Constructors
//******************************************************************************

UsbSerial::UsbSerial(
    Platform::String ^vid_
    ) :
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _usb_lock(_usbutex, std::defer_lock),
    _baud(57600),
    _config(SerialConfig::SERIAL_8N1),
    _current_load_operation(nullptr),
    _device(nullptr),
    _device_collection(nullptr),
    _pid(nullptr),
    _rx(nullptr),
    _serial_device(nullptr),
    _tx(nullptr),
    _vid(vid_)
{
}

UsbSerial::UsbSerial(
    Platform::String ^vid_,
    Platform::String ^pid_
    ) :
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _usb_lock(_usbutex, std::defer_lock),
    _baud(57600),
    _config(SerialConfig::SERIAL_8N1),
    _current_load_operation(nullptr),
    _device(nullptr),
    _device_collection(nullptr),
    _pid(pid_),
    _rx(nullptr),
    _serial_device(nullptr),
    _tx(nullptr),
    _vid(vid_)
{
}

UsbSerial::UsbSerial(
    Windows::Devices::Enumeration::DeviceInformation ^device_
    ) :
    _connection_ready(ATOMIC_VAR_INIT(false)),
    _usb_lock(_usbutex, std::defer_lock),
    _baud(57600),
    _config(SerialConfig::SERIAL_8N1),
    _current_load_operation(nullptr),
    _device(device_),
    _device_collection(nullptr),
    _pid(nullptr),
    _rx(nullptr),
    _serial_device(nullptr),
    _tx(nullptr),
    _vid(nullptr)
{
}

//******************************************************************************
//* Destructors
//******************************************************************************

UsbSerial::~UsbSerial(
    void
    )
{
    //we will fire the ConnectionLost event in the case that this object is unexpectedly destructed while the connection is established.
    if( connectionReady() )
    {
        ConnectionLost( L"Your connection has been terminated. The Microsoft::Maker::Serial::UsbSerial destructor was called unexpectedly." );
    }
    end();
}

//******************************************************************************
//* Public Methods
//******************************************************************************

uint16_t
UsbSerial::available(
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

void
UsbSerial::begin(
    uint32_t baud_,
    SerialConfig config_
    )
{
    _baud = baud_;
    _config = config_;

    // Ensure known good state
    end();

    // Although this path is not optimal, the behavior (the calls must be made from the UI thread) follows BluetoothSerial. The algorithm is a compromise to provide the succint, maintainable code.
    Concurrency::create_task(listAvailableDevicesAsync())
        .then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^device_collection_)
    {
        // If a friendly name was specified, then identify the associated device
        if (_vid) {
            // Store parameter as a member to ensure the duration of object allocation
            _device_collection = device_collection_;
            if (!_device_collection->Size)
            {
                throw ref new Platform::Exception(E_UNEXPECTED, L"No USB devices found.");
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
            ConnectionFailed(L"UsbSerial::connectToDeviceAsync failed with a Platform::Exception type. (message: " + e->Message + L")");
        }
        catch (...)
        {
            ConnectionFailed(L"UsbSerial::connectToDeviceAsync failed with a non-Platform::Exception type. (vid: " + _vid + L" pid: " + _pid + L")");
        }
    });
}

bool
UsbSerial::connectionReady(
    void
    )
{
    return _connection_ready;
}

/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
UsbSerial::end(
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
    _serial_device = nullptr;
    _device_collection = nullptr;
}

void
UsbSerial::flush(
    void
    )
{
    auto async_operation_ = _tx->StoreAsync();
    create_task( async_operation_ )
        .then( [ this, async_operation_ ]( task<unsigned int> task_ )
    {
        try
        {
            task_.get();

            //detect disconnection
            if( async_operation_->Status == Windows::Foundation::AsyncStatus::Error )
            {
                throw ref new Platform::Exception( E_UNEXPECTED );
            }
        }
        catch( Platform::Exception ^ )
        {
            _connection_ready = false;
            ConnectionLost( L"A fatal error has occurred in UsbSerial::flush() and your connection has been lost." );
        }
    } );
}

/// \details An Advanced Query String is constructed based upon paired usb devices. Then a collection is returned of all devices matching the query.
/// \ref https://msdn.microsoft.com/en-us/library/aa965711(VS.85).aspx
Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
UsbSerial::listAvailableDevicesAsync(
    void
    )
{
    // Construct AQS String from service id of desired device
    Platform::String ^device_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector();

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(device_aqs);
}

void
UsbSerial::lock(
    void
    )
{
    _usb_lock.lock();
}

uint16_t
UsbSerial::read(
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
            ConnectionLost( L"A fatal error has occurred in UsbSerial::read() and your connection has been lost." );
            return -1;
        }

        _current_load_operation = _rx->LoadAsync( MAX_READ_SIZE );
    }

    return c;
}

void
UsbSerial::unlock(
    void
    )
{
    _usb_lock.unlock();
}

uint32_t
UsbSerial::write(
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
UsbSerial::connectToDeviceAsync(
    Windows::Devices::Enumeration::DeviceInformation ^device_
    )
{
    return Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(device_->Id))
        .then([this](Windows::Devices::SerialCommunication::SerialDevice ^serial_device_)
    {
        if( serial_device_ == nullptr )
        {
            throw ref new Platform::Exception( E_UNEXPECTED, ref new Platform::String( L"Unable to initialize the device. Did you forget your USB device capabilities in the manifest? SerialDevice::FromIdAsync returned null." ) );
        }

        // Store parameter as a member to ensure the duration of object allocation
        _serial_device = serial_device_;

        // Configure the device properties
        _serial_device->Handshake = SerialHandshake::None;
        _serial_device->BaudRate = _baud;

        switch (_config) {
        case SerialConfig::SERIAL_5E1:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_5E2:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_5N1:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_5N2:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_5O1:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_5O2:
            _serial_device->DataBits = 5;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_6E1:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_6E2:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_6N1:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_6N2:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_6O1:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_6O2:
            _serial_device->DataBits = 6;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_7E1:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_7E2:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_7N1:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_7N2:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_7O1:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_7O2:
            _serial_device->DataBits = 7;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_8E1:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_8E2:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::Even;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_8N1:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_8N2:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::None;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        case SerialConfig::SERIAL_8O1:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::One;
            break;
        case SerialConfig::SERIAL_8O2:
            _serial_device->DataBits = 8;
            _serial_device->Parity = SerialParity::Odd;
            _serial_device->StopBits = SerialStopBitCount::Two;
            break;
        }

        // Enable RX
        _rx = ref new Windows::Storage::Streams::DataReader(_serial_device->InputStream);
        _rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;  // Partial mode will allow for better async reads
        _current_load_operation = _rx->LoadAsync( MAX_READ_SIZE );

        // Enable TX
        _tx = ref new Windows::Storage::Streams::DataWriter(_serial_device->OutputStream);

        // Set connection ready flag
        _connection_ready = true;
        ConnectionEstablished();
    });
}

Windows::Devices::Enumeration::DeviceInformation ^
UsbSerial::identifyDeviceFromCollection(
    Windows::Devices::Enumeration::DeviceInformationCollection ^devices_
    )
{
    for (auto &&device : devices_)
    {
        // If the vid doesn't match, move to the next device.
        if (std::string::npos == std::wstring(device->Id->Data()).find(_vid->Data()))
        {
            continue;
        }

        // If the pid doesn't match, move to the next device.
        if (_pid && std::string::npos == std::wstring(device->Id->Data()).find(_pid->Data()))
        {
            continue;
        }

        // If the supplied values match, we've identified the device!
        return device;
    }

    // If we searched and found nothing that matches the identifier, we've failed to connect and cannot recover.
    throw ref new Platform::Exception(E_INVALIDARG, L"No USB devices found matching the specified identifier.");
}
