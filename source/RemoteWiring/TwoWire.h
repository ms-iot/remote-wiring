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
namespace RemoteWiring {

ref class RemoteDevice;

namespace I2c {

public delegate void I2cReplyCallback( uint8_t address_, uint8_t reg_, Windows::Storage::Streams::DataReader ^response );

public ref class TwoWire sealed
{
public:
    friend ref class RemoteDevice;

    event I2cReplyCallback ^ I2cReplyEvent;

    ///<summary>
    ///Enables I2C with no delay time for requesting a response from the secondary device
    ///</summary>
    void
    inline
    enable(
        void
        )
    {
        enable( 0 );
    }


    ///<summary>
    ///Enables I2C with a given delay time for requesting a response from the secondary device
    ///</summary>
    void
    enable(
        uint16_t i2cReadDelayMicros_
    );

    
    ///<summary>
    ///Begins an I2C transmission to the given device address.
    ///</summary>
    void
    beginTransmission(
        uint8_t address_
    );


    ///<summary>
    ///Writes raw byte data to the device
    ///<para>You must have an active device transmission open. Begin a new transmission with beginTransmission( uint8_t )</para>
    ///</summary>
    void
    write(
        uint8_t data_
    );


    ///<summary>
    ///ends an I2C transmission
    ///</summary>
    void
    endTransmission(
        void
    );


    ///<summary>
    ///A one-time read which will request the given number of bytes from the device.
    ///<para>The device's response will be provided in the form of an I2cReplyEvent. You must subscribe
    ///to this event with a delegate function in order to be alerted of your reply.</para>
    ///</summary>
    inline
    void
    requestFrom(
        uint8_t address_,
        uint8_t numBytes_
        ) 
    {
        sendI2cSysex( address_, 0x08, 1, &numBytes_ );
    }

private:
    //since 16 bit values are sent as two 7 bit bytes, you can't send a value larger than this across the wire
    const uint16_t MAX_READ_DELAY_MICROS = 0x3FFF;
    const size_t MAX_MESSAGE_LEN = 15;

    //singleton pattern w/ friend class to instantiate
    TwoWire(
        Firmata::UwpFirmata ^ firmata_
        ) :
        _data_buffer( new uint8_t[ MAX_MESSAGE_LEN ] ),
        _firmata( firmata_ )
    {
        _firmata->I2cReplyReceived += ref new Firmata::I2cReplyCallbackFunction( [this]( Firmata::UwpFirmata ^caller, Firmata::I2cCallbackEventArgs^ args ) -> void { onI2cReply( args ); } );
    }
    
    //a reference to the UAP firmata interface
    Firmata::UwpFirmata ^_firmata;

    //transmission-building variables
    uint8_t _address;
    uint8_t _position;
    std::unique_ptr<uint8_t> _data_buffer;

    void
    sendI2cSysex(
        const uint8_t address_,
        const uint8_t rw_mask_,
        const uint8_t len_,
        uint8_t *data_
    );

    void
    onI2cReply(
        Firmata::I2cCallbackEventArgs ^argv
    );
};

} // namespace I2c
} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
