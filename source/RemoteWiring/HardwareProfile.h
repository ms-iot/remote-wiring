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

namespace Microsoft {
namespace Maker {
namespace RemoteWiring {

/*
 * This class represents a virtual piece of hardware. It can create a profile of pins and their capabilities, and can be
 * used to verify outgoing commands are valid and map incoming commands to specific pins.
 */
public ref class HardwareProfile sealed
{

public:

    enum class Protocol
    {
        FIRMATA
    };

    [Windows::Foundation::Metadata::DefaultOverload]
    /*inline
    HardwareProfile(
        Windows::Storage::Streams::IBuffer ^buffer_
        )
    {
        this( buffer_, Protocol::FIRMATA );
    }*/

    HardwareProfile(
        Windows::Storage::Streams::IBuffer ^buffer_,
        Protocol protocol_ = Protocol::FIRMATA
        );

private:
    std::atomic_bool _is_valid;

    //stateful members received from the device
    std::atomic_int _analog_offset;
    std::atomic_int _num_analog_pins;
    std::atomic_int _total_pins;

    void
    initializeWithFirmata(
        Windows::Storage::Streams::IBuffer ^buffer_
        )
    {

    }
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
