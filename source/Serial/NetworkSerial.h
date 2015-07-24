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

#pragma once
#include "IStream.h"

namespace Microsoft {
namespace Maker {
namespace Serial {

public ref class NetworkSerial sealed : public IStream
{
public:
    virtual event RemoteWiringConnectionCallback ^ConnectionEstablished;
    virtual event RemoteWiringConnectionCallbackWithMessage ^ConnectionLost;
    virtual event RemoteWiringConnectionCallbackWithMessage ^ConnectionFailed;

    [Windows::Foundation::Metadata::DefaultOverload]
    ///<summary>
    ///A constructor which accepts a device HostName (web address or IP) and port to connect to.
    ///</summary>
    NetworkSerial(
        Windows::Networking::HostName ^host_,
        uint16_t port_
        );

    virtual
    ~NetworkSerial(
        void
        );

    virtual
    uint16_t
    available(
        void
        );

    virtual
    void
    begin(
        uint32_t baud_,
        SerialConfig config_
        );

    virtual
    void
    end(
        void
        );

    virtual
    void
    flush(
        void
        );

    virtual
    uint16_t
    read(
        void
        );

    virtual
    uint32_t
    write(
        uint8_t c_
        );

    ///<summary>
    ///Returns true if the connection is currently established
    ///</summary>
    bool
    connectionReady(
        void
        );

private:
    //maximum amount of data that may be read at a time, allows efficient reads
    static const uint8_t MAX_READ_SIZE = 100;

    // Device specific members (set during instantation)
    Windows::Networking::HostName ^_host;
    uint16_t _port;

    std::atomic_bool _connection_ready;
    Windows::Storage::Streams::DataReaderLoadOperation ^_current_load_operation;
    Windows::Networking::Sockets::StreamSocket ^_stream_socket;
    Windows::Storage::Streams::DataReader ^_rx;
    Windows::Storage::Streams::DataWriter ^_tx;

    Concurrency::task<void>
        connectToHostAsync(
            Windows::Networking::HostName ^host_,
            uint16_t port_
            );
};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft