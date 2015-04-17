﻿#pragma once

#include "IArduinoStream.h"

namespace Microsoft {
namespace Maker {
namespace Serial {

public ref class BluetoothSerial sealed : public IArduinoStream
{
public:
	event RemoteWiringConnectionCallback^ ConnectionEstablished;
	event RemoteWiringConnectionCallback^ ConnectionFailed;
	event RemoteWiringConnectionCallback^ ConnectionLost;

	BluetoothSerial();
	BluetoothSerial( Platform::String ^deviceIdentifier_ );
	BluetoothSerial( Windows::Devices::Enumeration::DeviceInformation ^device_ );

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

	static
	Windows::Foundation::IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection ^> ^
	listAvailableDevicesAsync(
		void
	);

private:
    Windows::Devices::Bluetooth::Rfcomm::RfcommDeviceService ^_device_service;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceId ^_service_id;
    Windows::Devices::Bluetooth::Rfcomm::RfcommServiceProvider ^_service_provider;
    Windows::Networking::Sockets::StreamSocket ^_stream_socket;
	Windows::Storage::Streams::DataReader ^_rx;
	Windows::Storage::Streams::DataWriter ^_tx;
	Windows::Storage::Streams::DataReaderLoadOperation ^currentLoadOperation;
	Windows::Storage::Streams::DataWriterStoreOperation ^currentStoreOperation;

	//optional device-specifiers
	Platform::String ^_deviceIdentifier;
	Windows::Devices::Enumeration::DeviceInformation ^_device;

    LONG volatile _connection_ready;
    bool _synchronous_mode;

    void
    begin (
        bool synchronous_mode_
	);

	void
	connect(
		Windows::Devices::Enumeration::DeviceInformation ^device_
	);

	Windows::Storage::Streams::DataReaderLoadOperation ^
	loadAsync(
		unsigned int count_
	);
};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft
