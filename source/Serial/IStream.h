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
