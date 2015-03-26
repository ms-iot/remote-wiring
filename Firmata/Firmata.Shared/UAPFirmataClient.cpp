
#include "pch.h"
#include "UAPFirmataClient.h"

#include "Firmata.h"

using namespace Wiring::Serial;
using namespace Wiring::Firmata;

namespace {
    FirmataClass RawFirmata;
}

UAPFirmataClient::UAPFirmataClient() {}

void
UAPFirmataClient::begin(
    uint32_t speed_
) {
    return ::RawFirmata.begin(speed_);
}

void
UAPFirmataClient::begin(
    ISerial ^s_
) {
    return ::RawFirmata.begin(s_);
}

void
UAPFirmataClient::printVersion(
    void
) {
    return ::RawFirmata.printVersion();
}

void
UAPFirmataClient::printFirmwareVersion(
    void
) {
    return ::RawFirmata.printFirmwareVersion();
}

void
UAPFirmataClient::setFirmwareNameAndVersion(
    String ^name_,
    byte major_,
    byte minor_
) {
    std::wstring nameW = name_->ToString()->Begin();
    std::string nameA(nameW.begin(), nameW.end());
    return ::RawFirmata.setFirmwareNameAndVersion(nameA.c_str(), major_, minor_);
}

int
UAPFirmataClient::available(
    void
) {
    return ::RawFirmata.available();
}

void
UAPFirmataClient::processInput(
    void
) {
    return ::RawFirmata.processInput();
}

void
UAPFirmataClient::sendAnalog(
    byte pin_,
    int value_
) {
    return ::RawFirmata.sendAnalog(pin_, value_);
}

void
UAPFirmataClient::sendDigitalPort(
    byte portNumber_,
    int portData_
) {
    return ::RawFirmata.sendDigitalPort(portNumber_, portData_);
}

void
UAPFirmataClient::sendString(
    String ^string_
) {
    std::wstring stringW = string_->ToString()->Begin();
    std::string stringA(stringW.begin(), stringW.end());
    return ::RawFirmata.sendString(stringA.c_str());
}

void
UAPFirmataClient::sendString(
    byte command_,
    String ^string_
) {
    std::wstring stringW = string_->ToString()->Begin();
    std::string stringA(stringW.begin(), stringW.end());
    return ::RawFirmata.sendString(command_, stringA.c_str());
}

void
UAPFirmataClient::sendSysex(
    byte command_,
    byte bytec_,
    String ^bytev_
) {
    std::wstring bytevW = bytev_->ToString()->Begin();
    std::string bytevA(bytevW.begin(), bytevW.end());
    byte *bytev = reinterpret_cast<byte *>(const_cast<char *>(bytevA.c_str()));
    return ::RawFirmata.sendSysex(command_, bytec_, bytev);
}

void
UAPFirmataClient::write(
    byte c_
) {
    return ::RawFirmata.write(c_);
}
