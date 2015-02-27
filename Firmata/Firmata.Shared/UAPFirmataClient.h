#pragma once

#include <cstdint>

#ifdef IGNORE
#undef IGNORE
#endif

using namespace Windows::Storage::Streams;

namespace Wiring {
namespace Firmata {

ref class UAPFirmataClient;

public ref class CallbackEventArgs sealed {
  public:
    CallbackEventArgs(
        byte status_,
        uint16_t value_
    ) :
        _status(status_),
        _value(value_)
    {}

    inline byte getStatus(void) { return _status; }
    
    inline uint16_t getValue(void) { return _value; }

  private:
    byte _status;
    uint16_t _value;
};

public ref class StringCallbackEventArgs sealed {
  public:
    StringCallbackEventArgs(
        IBuffer ^string_
    ) :
      _string(string_)
    {}

    inline IBuffer ^ getString(void) { return _string; }

  private:
    IBuffer ^_string;
};

public ref class SysexCallbackEventArgs sealed {
  public:
    SysexCallbackEventArgs(
        byte command_,
        IBuffer ^sysex_string_
    ) :
    _command(command_),
    _sysex_string(sysex_string_)
    {}

    inline byte getCommand(void) { return _command; }

    inline IBuffer ^ getSysexString(void) { return _sysex_string; }

  private:
    byte _command;
    IBuffer ^_sysex_string;
};

public ref class SystemResetCallbackEventArgs sealed {
  public:
      SystemResetCallbackEventArgs() {}
  private:
};

public enum class FirmataCommand {
    ANALOG_MESSAGE = 0xE0,
    DIGITAL_MESSAGE = 0x90,
    REPORT_ANALOG_PIN = 0xC0,
    REPORT_DIGITAL_PIN = 0xD0,
    SET_PIN_MODE = 0xF4,
    START_SYSEX = 0xF0,
    END_SYSEX = 0xF7,
    PROTOCOL_VERSION = 0xF9,
    SYSTEM_RESET = 0xFF,
};

public enum class FirmataSysexCommand {
    ENCODER_DATA = 0x61,
    SERVO_CONFIG = 0x70,
    STRING_DATA = 0x71,
    STEPPER_DATA = 0x72,
    ONEWIRE_DATA = 0x73,
    SHIFT_DATA = 0x75,
    I2C_REQUEST = 0x76,
    I2C_REPLY = 0x77,
    I2C_CONFIG = 0x78,
    EXTENDED_ANALOG = 0x6F,
    PIN_STATE_QUERY = 0x6D,
    PIN_STATE_RESPONSE = 0x6E,
    CAPABILITY_QUERY = 0x6B,
    CAPABILITY_RESPONSE = 0x6C,
    ANALOG_MAPPING_QUERY = 0x69,
    ANALOG_MAPPING_RESPONSE = 0x6A,
    REPORT_FIRMWARE = 0x79,
    SAMPLING_INTERVAL = 0x7A,
    SCHEDULER_DATA = 0x7B,
    SYSEX_NON_REALTIME = 0x7E,
    SYSEX_REALTIME = 0x7F,
};

public enum class FirmataPinMode {
    INPUT = 0x00,
    OUTPUT = 0x01,
    ANALOG = 0x02,
    PWM = 0x03,
    SERVO = 0x04,
    SHIFT = 0x05,
    I2C = 0x06,
    ONEWIRE = 0x07,
    STEPPER = 0x08,
    ENCODER = 0x09,
    IGNORE = 0x7F,
    TOTAL_PIN_MODES = 0x0B,
};

public delegate void CallbackFunction(UAPFirmataClient ^caller, CallbackEventArgs ^argv);
public delegate void StringCallbackFunction(UAPFirmataClient ^caller, StringCallbackEventArgs ^argv);
public delegate void SysexCallbackFunction(UAPFirmataClient ^caller, SysexCallbackEventArgs ^argv);
public delegate void SystemResetCallbackFunction(UAPFirmataClient ^caller, SystemResetCallbackEventArgs ^argv);

public ref class UAPFirmataClient sealed
{
  public:
    event CallbackFunction^ CallbackEvent;
    event StringCallbackFunction^ StringCallbackEvent;
    event SysexCallbackFunction^ SysexCallbackEvent;
    event SystemResetCallbackFunction^ SystemResetCallbackEvent;

    UAPFirmataClient();

    void
    begin(
        uint32_t speed
    );

    [Windows::Foundation::Metadata::DefaultOverload]
    void
    begin(
        Serial::ISerial ^s
    );

    void
    printVersion(
        void
    );

    void
    printFirmwareVersion(
        void
    );

    void
    setFirmwareNameAndVersion(
        IBuffer ^name,
        byte major,
        byte minor
    );

    int
    available(
        void
    );

    void
    processInput(
        void
    );

    void
    sendAnalog(
        byte pin,
        int value
    );

    void
    sendDigitalPort(
        byte portNumber,
        int portData
    );

    void
    sendString(
        IBuffer ^string
    );

    void
    sendString(
        byte command,
        IBuffer ^string
    );

    void
    sendSysex(
        byte command,
        byte bytec,
        IBuffer ^bytev
    );

    void
    write(
        byte c
    );

  private:
    CallbackFunction^ _callback_function;
    StringCallbackFunction^ _string_callback_function;
    SysexCallbackFunction^ _sysex_callback_function;
    SystemResetCallbackFunction^ _system_reset_callback_function;
};

}
}
