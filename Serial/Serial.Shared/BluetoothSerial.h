#pragma once

#include "ISerial.h"

#include <cstdint>

namespace Wiring {
namespace Serial {

public ref class BluetoothSerial sealed : public ISerial
{
public:
    BluetoothSerial();

    virtual
    uint16_t
    available (
        void
    );

    virtual
    void
    begin (
        uint32_t baud_,
        uint8_t config_
    );

    virtual
    void
    end(
        void
    );

    virtual
    uint16_t
    read (
        void
    );

    virtual
    uint32_t
    write (
        uint8_t c_
    );

    bool
    connectionReady(
        void
    );

    Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
    listAvailableDevicesAsync(
        void
    );

    Windows::Storage::Streams::DataReaderLoadOperation ^
    loadAsync(
        unsigned int count_
    );

private:
    Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^_device_service;
    Windows::Storage::Streams::DataReader ^_rx;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceId ^_service_id;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceProvider ^_service_provider;
    Windows::Networking::Sockets::StreamSocket ^_stream_socket;
    Windows::Storage::Streams::DataWriter ^_tx;

    LONG volatile _connection_ready;
};

}
}
