
#include "pch.h"
#include "BluetoothSerial.h"

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth::Rfcomm;
using namespace Windows::Networking::Sockets;
using namespace Windows::Storage::Streams;
using namespace Concurrency;
using namespace Wiring::Serial;

namespace {
    // Connection code taken from:
    // https://msdn.microsoft.com/en-us/library/windows/apps/xaml/dn264586.aspx

    // This App relies on CRC32 checking available in version 2.0 of the service.
    const uint32_t SERVICE_VERSION_ATTRIBUTE_ID = 0x0300;
    const byte SERVICE_VERSION_ATTRIBUTE_TYPE = 0x0A;   // UINT32
    const uint32_t MINIMUM_SERVICE_VERSION = 200;

    Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService^ _service;
    Windows::Networking::Sockets::StreamSocket^ _socket;
    Windows::Storage::Streams::DataReader ^_read_pipe;
    Windows::Storage::Streams::DataWriter ^_write_pipe;

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

    bool IsCompatibleVersion(RfcommDeviceService^ service)
    {
        auto attributes = create_task(service->GetSdpRawAttributesAsync(
            Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached)).get();
        auto reader = DataReader::FromBuffer(attributes->Lookup(SERVICE_VERSION_ATTRIBUTE_ID));

        // The first byte contains the attribute's type
        byte attributeType = reader->ReadByte();
        if (attributeType == SERVICE_VERSION_ATTRIBUTE_TYPE)
        {
            // The remainder is the data
            uint32_t version = reader->ReadUInt32();
            return version >= MINIMUM_SERVICE_VERSION;
        }

        return true;
    }

    void Initialize()
    {
        // Enumerate devices with the object push service
        create_task(
            Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
            RfcommDeviceService::GetDeviceSelector(
            RfcommServiceId::SerialPort)))
        .then([](DeviceInformationCollection^ services)
        {
            if (services->Size > 0)
            {
                // Initialize the target Bluetooth BR device
                create_task(RfcommDeviceService::FromIdAsync(services->GetAt(0)->Id))
                .then([](RfcommDeviceService^ service)
                {
                    // Check that the service meets this App’s minimum
                    // requirement
                    if (SupportsProtection(service)
                        /* && IsCompatibleVersion(service) */)
                    {
                        _service = service;

                        // Create a socket and connect to the target
                        _socket = ref new StreamSocket();
                        create_task(_socket->ConnectAsync(
                            _service->ConnectionHostName,
                            _service->ConnectionServiceName,
                            SocketProtectionLevel
                            ::BluetoothEncryptionAllowNullAuthentication))
                        .then([](void)
                        {
                            // The socket is connected. At this point the App can
                            // wait for the user to take some action, e.g. click
                            // a button to send a file to the device, which could
                            // invoke the Picker and then send the picked file.
                            // The transfer itself would use the Sockets API and
                            // not the Rfcomm API, and so is omitted here for
                            //brevity.
                            _read_pipe = ref new DataReader(_socket->InputStream);
                            _write_pipe = ref new DataWriter(_socket->OutputStream);
                        });
                    }
                });
            }
        });
    }

}  // namespace

BluetoothSerial::BluetoothSerial()
{
}

uint16_t
BluetoothSerial::available(
    void
) {
    return _read_pipe->UnconsumedBufferLength;
}

void
BluetoothSerial::begin(
    uint32_t baud_,
    uint8_t config_
) {
    UNREFERENCED_PARAMETER(baud_);
    UNREFERENCED_PARAMETER(config_);

    return Initialize();
}

uint16_t
BluetoothSerial::read(
    void
) {
    uint16_t c = static_cast<uint16_t>(-1);

    if ( available() ) {
         c = _read_pipe->ReadByte();
    }

    return c;
}

uint32_t
BluetoothSerial::write(
    uint8_t c_
) {
    _write_pipe->WriteByte(c_);
    _write_pipe->StoreAsync();
    return 1;
}
