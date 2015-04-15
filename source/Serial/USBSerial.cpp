
#include "pch.h"
#include "UsbSerial.h"

#include <string>

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Microsoft::Maker::Serial;

UsbSerial::UsbSerial
(
    void
) :
	_baud(9600),
	_config(SerialConfig::SERIAL_8N1),
    _connection_ready(0),
    _usb_device(nullptr),
    _rx(nullptr),
    _tx(nullptr)
{
}

uint16_t
UsbSerial::available(
    void
) {
    // Check to see if connection is ready
    if ( !connectionReady() ) { return 0; }

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
    begin(true);
}

void
UsbSerial::begin(
    bool synchronous_mode_
) {
    _synchronous_mode = synchronous_mode_;

    // Ensure known good state
    end();

	try
	{
		// Identify all paired serial devices
		Concurrency::create_task(listAvailableDevicesAsync())
			.then([this](Windows::Devices::Enumeration::DeviceInformationCollection ^usb_devices_) {
			// Ensure at least one device satisfies query
			if (!usb_devices_->Size) { return; }

			for (unsigned int i = 0; i < usb_devices_->Size; ++i) {
				if (std::string::npos == std::wstring(usb_devices_->GetAt(i)->Name->Data()).find(L"VID_2341") && std::string::npos == std::wstring(usb_devices_->GetAt(i)->Id->Data()).find(L"VID_2341")) { continue; }

				Concurrency::create_task(Windows::Devices::SerialCommunication::SerialDevice::FromIdAsync(usb_devices_->GetAt(i)->Id))
					.then([this](Windows::Devices::SerialCommunication::SerialDevice ^usb_device_) {
					_usb_device = usb_device_;
					_usb_device->BaudRate = _baud;
					switch (_config) {
					case SerialConfig::SERIAL_5E1:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_5E2:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_5N1:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_5N2:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_5O1:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_5O2:
						_usb_device->DataBits = 5;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_6E1:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_6E2:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_6N1:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_6N2:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_6O1:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_6O2:
						_usb_device->DataBits = 6;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_7E1:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_7E2:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_7N1:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_7N2:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_7O1:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_7O2:
						_usb_device->DataBits = 7;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_8E1:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_8E2:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::Even;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_8N1:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_8N2:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::None;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					case SerialConfig::SERIAL_8O1:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::One;
						break;
					case SerialConfig::SERIAL_8O2:
						_usb_device->DataBits = 8;
						_usb_device->Parity = SerialParity::Odd;
						_usb_device->StopBits = SerialStopBitCount::Two;
						break;
					}

					_rx = ref new Windows::Storage::Streams::DataReader(usb_device_->InputStream);
					if (_synchronous_mode) {
						_rx->InputStreamOptions = Windows::Storage::Streams::InputStreamOptions::Partial;
						currentLoadOperation = _rx->LoadAsync(100);
						currentStoreOperation = nullptr;
					}

					_tx = ref new Windows::Storage::Streams::DataWriter(usb_device_->OutputStream);

					// Set connection ready flag
					InterlockedOr(&_connection_ready, true);
					ConnectionEstablished();
				});
			}
		});
	}
	catch (Platform::Exception ^e)
	{
		ConnectionFailed();
	}

    return;
}

void
UsbSerial::beginAsync(
    void
) {
    begin(false);
}

bool
UsbSerial::connectionReady(
void
) {
    return static_cast<bool>(InterlockedAnd(&_connection_ready, true));
}

/// \ref https://social.msdn.microsoft.com/Forums/windowsapps/en-US/961c9d61-99ad-4a1b-82dc-22b6bd81aa2e/error-c2039-close-is-not-a-member-of-windowsstoragestreamsdatawriter?forum=winappswithnativecode
void
UsbSerial::end(
    void
) {
    InterlockedAnd(&_connection_ready, false);
    delete(_rx); //_rx->Close();
    _rx = nullptr;
    delete(_tx); //_tx->Close();
    _tx = nullptr;
    delete(_usb_device);
    _usb_device = nullptr;
}

Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
UsbSerial::listAvailableDevicesAsync(
void
) {
    // Arduino Uno Vendor and Product Ids
    //const uint16_t vid = 0x2341;
    //const uint16_t pid = 0x0043;

    // Construct AQS String from service id of desired device
    //Platform::String ^device_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelectorFromUsbVidPid(vid, pid);  // Arduino Uno only
	Platform::String ^device_aqs = Windows::Devices::SerialCommunication::SerialDevice::GetDeviceSelector("ALL");

    // Identify all paired devices satisfying query
    return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(device_aqs);
}

Windows::Storage::Streams::DataReaderLoadOperation ^
UsbSerial::loadAsync(
    unsigned int count_
) {
    //TODO: Determine how to return an empty DataReaderLoadOperation when the connection is unavailable or synchronous mode is enabled
    return _rx->LoadAsync(count_);
}


uint16_t
UsbSerial::read(
	void
	) {
	uint16_t c = static_cast<uint16_t>(-1);

	if (available()) {
		c = _rx->ReadByte();
	}

	// Prefetch buffer
	if (connectionReady() &&
		_synchronous_mode &&
		!_rx->UnconsumedBufferLength &&
		currentLoadOperation->Status != Windows::Foundation::AsyncStatus::Started)
	{
		//attempt to detect disconnection
		if (currentLoadOperation->Status == Windows::Foundation::AsyncStatus::Error)
		{
			InterlockedAnd(&_connection_ready, false);
			ConnectionLost();
			return -1;
		}
		currentLoadOperation->Close();
		currentLoadOperation = _rx->LoadAsync(100);
	}

	return c;
}

uint32_t
UsbSerial::write(
	uint8_t c_
	) {
	// Check to see if connection is ready
	if (!connectionReady()) { return 0; }

	if ((currentStoreOperation != nullptr) && currentStoreOperation->Status == Windows::Foundation::AsyncStatus::Error)
	{
		InterlockedAnd(&_connection_ready, false);
		ConnectionLost();
		return 0;
	}

	_tx->WriteByte(c_);
	currentStoreOperation = _tx->StoreAsync();
	return 1;
}