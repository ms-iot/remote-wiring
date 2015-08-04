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

#include <cstdint>

namespace Microsoft {
namespace Maker {
namespace Serial {

public enum class SerialConfig {
    SERIAL_5E1,
    SERIAL_5E2,
    SERIAL_5N1,
    SERIAL_5N2,
    SERIAL_5O1,
    SERIAL_5O2,
    SERIAL_6E1,
    SERIAL_6E2,
    SERIAL_6N1,
    SERIAL_6N2,
    SERIAL_6O1,
    SERIAL_6O2,
    SERIAL_7E1,
    SERIAL_7E2,
    SERIAL_7N1,
    SERIAL_7N2,
    SERIAL_7O1,
    SERIAL_7O2,
    SERIAL_8E1,
    SERIAL_8E2,
    SERIAL_8N1,
    SERIAL_8N2,
    SERIAL_8O1,
    SERIAL_8O2,
    SERIAL_CONFIG_COUNT,
};

public delegate void IStreamConnectionCallback();
public delegate void IStreamConnectionCallbackWithMessage(Platform::String ^message);

public interface struct IStream
{
    virtual event IStreamConnectionCallback ^ConnectionEstablished;
    virtual event IStreamConnectionCallbackWithMessage ^ConnectionLost;
    virtual event IStreamConnectionCallbackWithMessage ^ConnectionFailed;

    ///<summary>
    ///Returns the number of bytes available to be read
    ///</summary>
    virtual
    uint16_t
    available(
        void
        ) = 0;

    ///<summary>
    ///Initializes the connection process, which will attempt to establish a connection to the device.
    ///<para>This connection process will complete asyncronously. You must subscribe to one of the connection events
    ///in order to be informed of a successful or failed connection attempt.</para>
    ///</summary>
    virtual
    void
    begin(
        uint32_t baud_,
        SerialConfig config_
        ) = 0;

    ///<summary>
    ///Returns true if the connection is currently established
    ///</summary>
    virtual
    bool
    connectionReady(
        void
        ) = 0;

    ///<summary>
    ///Closes the active connection
    ///</summary>
    virtual
    void
    end(
        void
        ) = 0;

    ///<summary>
    ///Flushes any awaiting data from the outbound queue. This function must be called before any data
    ///is actually sent to the other end of the connection.
    ///</summary>
    virtual
    void
    flush(
        void
        ) = 0;

    ///<summary>
    ///Locks this instance of the object, enabling thread safety
    ///<para>when explicitly invoking this method, unlock() must be called when the lock is no longer needed.</para>
    ///</summary>
    void
    lock(
        void
        ) = 0;

    ///<summary>
    ///Attempts to read one byte
    ///</summary>
    virtual
    uint16_t
    read(
        void
        ) = 0;

    ///<summary>
    ///Places one byte into the outbound queue. flush() must be called on this object before the data will be sent.
    ///</summary>
    virtual
    uint32_t
    write(
        uint8_t c_
        ) = 0;

    ///<summary>
    ///Unlocks this instance of the object, allowing other threads or actions to use it.
    ///<para>This function must be explicitly invoked after each invocation of the lock() method, when the lock is no longer needed.</para>
    ///</summary>
    virtual
    void
    unlock(
        void
        ) = 0;
};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft
