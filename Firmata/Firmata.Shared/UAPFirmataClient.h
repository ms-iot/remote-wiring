#pragma once

#include "Firmata.h"

#include <cstdint>

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
        Serial::ISerial ^s,
        uint32_t speed
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
