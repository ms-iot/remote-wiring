#pragma once

#include "ISerial.h"

#include <cstdint>

namespace Microsoft {
namespace Maker {
namespace Serial {

public delegate void ConnectionEstablishedCallback();
public delegate void ConnectionFailedCallback();
public delegate void ConnectionLostCallback();

public ref class BluetoothSerial sealed : public ISerial
{
public:
	event ConnectionEstablishedCallback^ ConnectionEstablished;
	event ConnectionFailedCallback^ ConnectionFailed;
	event ConnectionLostCallback^ ConnectionLost;

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
		SerialConfig config_
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

	void
    beginAsync (
        void
    );

	bool
    connectionReady(
        void
    );

private:
    Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^_device_service;
    Windows::Storage::Streams::DataReader ^_rx;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceId ^_service_id;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceProvider ^_service_provider;
    Windows::Networking::Sockets::StreamSocket ^_stream_socket;
	Windows::Storage::Streams::DataWriter ^_tx;
	Windows::Storage::Streams::DataReaderLoadOperation ^currentLoadOperation;
	Windows::Storage::Streams::DataReaderLoadOperation ^currentWriteOperation;

    LONG volatile _connection_ready;
    bool _synchronous_mode;

    void
    begin (
        bool synchronous_mode_
		);

	Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
		listAvailableDevicesAsync(
		void
		);

	Windows::Storage::Streams::DataReaderLoadOperation ^
		loadAsync(
		unsigned int count_
		);
};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft
