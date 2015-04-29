#pragma once

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

public delegate void RemoteWiringConnectionCallback();

public interface struct IStream
{
    virtual
    uint16_t
    available(
        void
    ) = 0;

    virtual
    void
    begin(
        uint32_t baud_,
		SerialConfig config_
    ) = 0;

    virtual
    void
    end(
        void
    ) = 0;

    virtual
    uint16_t
    read(
        void
    ) = 0;

    virtual
    uint32_t
    write(
        uint8_t c_
    ) = 0;
};

} // namespace Serial
} // namespace Maker
} // namespace Microsoft
