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
 * Protocol enum is used to recognize which protocol the initialization data represents.
 * Currently, the only option is Firmata, which is the only protocol Remote Arduino currently supports
 */
public enum class Protocol
{
    FIRMATA
};

/*
 * Pin capabilities are stored as bitmasks, the PinCapability enum represents the bit value of each capability.
 */
public enum class PinCapability
{
    INPUT = 0x01,
    INPUT_PULLUP = 0x02,
    OUTPUT = 0x04,
    ANALOG = 0x08,
    PWM = 0x10,
    SERVO = 0x20,
    I2C = 0x40
};

/*
 * This class represents a virtual piece of hardware. It can create a profile of pins and their capabilities, and can be
 * used to verify outgoing commands are valid and map incoming commands to specific pins.
 */
public ref class HardwareProfile sealed
{

public:

    //this is a required property which must always be accurate
    property int AnalogPinCount
    {
        int get()
        {
            return _analog_pin_count;
        }
    }

    //this is a required property which must always be accurate
    property int AnalogOffset
    {
        int get()
        {
            return _analog_offset;
        }
    }

    //this is a required property which must always be accurate
    property bool IsValid
    {
        bool get()
        {
            return _is_valid;
        }
    }

    //this is a required property which must always be accurate
    property int TotalPinCount
    {
        int get()
        {
            return _total_pin_count;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^AnalogPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = _analog_offset; pin < _total_pin_count; ++pin )
                {
                    if( isAnalogSupported( pin ) )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^DigitalPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = 0; pin < _total_pin_count; ++pin )
                {
                    if( isDigitalOutputSupported( pin ) )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^DisabledPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = 0; pin < _total_pin_count; ++pin )
                {
                    if( getPinCapabilitiesBitmask( pin ) == 0 )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^I2cPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = 0; pin < _total_pin_count; ++pin )
                {
                    if( isI2cSupported( pin ) )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^PwmPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = 0; pin < _total_pin_count; ++pin )
                {
                    if( isPwmSupported( pin ) )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }

    property Windows::Foundation::Collections::IVector<uint8_t> ^ServoPins
    {
        Windows::Foundation::Collections::IVector<uint8_t> ^ get()
        {
            auto vector = ref new Platform::Collections::Vector<uint8_t>();
            if( _is_valid )
            {
                for( size_t pin = 0; pin < _total_pin_count; ++pin )
                {
                    if( isServoSupported( pin ) )
                    {
                        vector->Append( pin );
                    }
                }
            }
            return vector;
        }
    }
    
    ///<summary>
    ///This default constructor accepts an IBuffer containing pin information which is assumed to be in the default Firmata protocol.
    ///<param name="buffer_">The input IBuffer object reference</param>
    ///</summary>
    inline
    HardwareProfile(
        Windows::Storage::Streams::IBuffer ^buffer_
        ) :
        HardwareProfile( buffer_, Protocol::FIRMATA )
    {
    }

    ///<summary>
    ///this constructor accepts an IBuffer containing pin information which will be decomposed using the given protocol to attempt to construct a valid profile
    ///<param name="buffer_">The input IBuffer object reference</param>
    ///<param name="protocol_">The protocol used to describe the device configuration</param>
    ///</summary>
    [Windows::Foundation::Metadata::DefaultOverloadAttribute]
    HardwareProfile(
        Windows::Storage::Streams::IBuffer ^buffer_,
        Protocol protocol_
        );

    ///<summary>
    ///this constructor accepts the absolute minimum required parameters for the class to function properly, but will not consider this instance 'valid'
    ///and will not contain any capability information for the hardware
    ///<param name="number_of_digital_pins_">The number of digital pins on the physical hardware device</param>
    ///<param name="number_of_analog_pins_">The number of analog pins on the physical hardware device</param>
    ///</summary>
    HardwareProfile(
        int total_number_of_pins,
        int number_of_analog_pins_
        );

    virtual ~HardwareProfile();

    ///<summary>
    ///returns the raw capabilities bitmask for the given pin, which represents all of the functionality of the pin
    ///an AND operation (&) can be performed with this bitmask and a PinCapability to determine if the given pin has the chosen capability.
    ///<param name="pin_">The requested pin</param>
    ///<returns>the bitmask for the requested pin or 0 if the pin and/or this hardware profile are not valid</returns>
    ///</summary>
    uint8_t
    getPinCapabilitiesBitmask(
        size_t pin_
        );

    ///<summary>
    ///returns true if the analog capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the analog (ADC) capability, false otherwise</returns>
    ///</summary>
    bool
    isAnalogSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the digital input capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the digital input capability, false otherwise</returns>
    ///</summary>
    bool
    isDigitalInputSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the digital input pullup capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the digital input pullup capability, false otherwise</returns>
    ///</summary>
    bool
    isDigitalInputPullupSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the digital output capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the digital output capability, false otherwise</returns>
    ///</summary>
    bool
    isDigitalOutputSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the I2C capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the I2C capability, false otherwise</returns>
    ///</summary>
    bool
    isI2cSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the PWM capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the PWM capability, false otherwise</returns>
    ///</summary>
    bool
    isPwmSupported(
        size_t pin_
        );

    ///<summary>
    ///returns true if the servo capability is supported by the given pin number
    ///<param name="pin_">The requested pin</param>
    ///<returns>true if the pin and this hardware profile are both valid and the pin supports the servo capability, false otherwise</returns>
    ///</summary>
    bool
    isServoSupported(
        size_t pin_
        );

private:
    std::atomic_bool _is_valid;

    //stateful members received from the device
    std::atomic_int _analog_offset;
    std::atomic_int _analog_pin_count;
    std::atomic_int _total_pin_count;
    std::vector<uint8_t> *_pinCapabilities;
    //for each of the following maps: K = pin number, V = resolution value in bits
    std::map<uint8_t, uint8_t> *_analogResolutions;
    std::map<uint8_t, uint8_t> *_pwmResolutions;
    std::map<uint8_t, uint8_t> *_servoResolutions;

    void
    initializeWithFirmata(
        Windows::Storage::Streams::IBuffer ^buffer_
        );
};

} // namespace Wiring
} // namespace Maker
} // namespace Microsoft
