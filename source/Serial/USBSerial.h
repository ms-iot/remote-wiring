#pragma once

#include "IStream.h"

namespace Microsoft {
namespace Maker {
namespace Serial {

public ref class UsbSerial sealed : public IStream
{
public:
	event RemoteWiringConnectionCallback^ ConnectionEstablished;
	event RemoteWiringConnectionCallback^ ConnectionFailed;
	event RemoteWiringConnectionCallback^ ConnectionLost;

    virtual
    uint16_t
    available (
        void
    );
	UsbSerial(Windows::Devices::Enumeration::DeviceInformation ^deviceInfo_);
	UsbSerial(Platform::String ^vid_);
	UsbSerial(Platform::String ^vid_, Platform::String ^pid_);

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

    Windows::Storage::Streams::DataReaderLoadOperation ^
    loadAsync(
        unsigned int count_
    );

private:
    Windows::Devices::SerialCommunication::SerialDevice ^_device;
	Windows::Devices::Enumeration::DeviceInformation ^_deviceInfo;
    Windows::Storage::Streams::DataReader ^_rx;
    Windows::Storage::Streams::DataWriter ^_tx;
	Windows::Storage::Streams::DataReaderLoadOperation ^currentLoadOperation;
	Windows::Storage::Streams::DataWriterStoreOperation ^currentStoreOperation;
	Windows::Devices::Enumeration::DeviceInformationCollection ^_devices;
	Platform::String ^_deviceIdentifier;
	Platform::String ^_vid;
	Platform::String ^_pid;
	
	uint32_t _baud;
	SerialConfig _config;
	LONG volatile _connection_ready;
    bool _synchronous_mode;


    void
    begin (
        bool synchronous_mode_
    );

};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft
