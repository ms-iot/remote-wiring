
#include "pch.h"
#include "UAPFirmataClient.h"

#include "Firmata.h"
#include <cstdlib>

using namespace Wiring::Serial;
using namespace Wiring::Firmata;
using namespace std::placeholders;

namespace {
    FirmataClass RawFirmata;
}


//******************************************************************************
//* Constructors
//******************************************************************************


UAPFirmataClient::UAPFirmataClient(
	void
	)
{
	RawFirmata.attach( static_cast<uint8_t>( DIGITAL_MESSAGE ), static_cast<callbackFunction>( std::bind( &UAPFirmataClient::digitalInvoke, this, _1, _2 ) ) );
	RawFirmata.attach( static_cast<uint8_t>( ANALOG_MESSAGE ), static_cast<callbackFunction>( std::bind( &UAPFirmataClient::analogInvoke, this, _1, _2 ) ) );
}


//******************************************************************************
//* Public Methods
//******************************************************************************

void
UAPFirmataClient::begin(
    ISerial ^s_
	)
{
    return ::RawFirmata.begin(s_);
}


void
UAPFirmataClient::finish(
	void
	)
{
	return ::RawFirmata.finish();
}


void
UAPFirmataClient::printVersion(
    void
	)
{
    return ::RawFirmata.printVersion();
}


void
UAPFirmataClient::printFirmwareVersion(
    void
	)
{
    return ::RawFirmata.printFirmwareVersion();
}


void
UAPFirmataClient::setFirmwareNameAndVersion(
	String ^name_,
	uint8_t major_,
	uint8_t minor_
	)
{
    std::wstring nameW = name_->ToString()->Begin();
    std::string nameA(nameW.begin(), nameW.end());
    return ::RawFirmata.setFirmwareNameAndVersion(nameA.c_str(), major_, minor_);
}


int
UAPFirmataClient::available(
    void
	)
{
    return ::RawFirmata.available();
}


void
UAPFirmataClient::processInput(
    void
	)
{
    return ::RawFirmata.processInput();
}


void
UAPFirmataClient::sendAnalog(
	uint8_t pin_,
	int value_
	)
{
    return ::RawFirmata.sendAnalog(pin_, value_);
}


void
UAPFirmataClient::sendDigitalPort(
	uint8_t portNumber_,
	int portData_
	)
{
    return ::RawFirmata.sendDigitalPort(portNumber_, portData_);
}


void
UAPFirmataClient::setDigitalReadEnabled(
	uint8_t portNumber,
	int portData
	)
{
	write(static_cast<uint8_t>(Wiring::Firmata::Command::REPORT_DIGITAL_PIN) | (portNumber & 0x0F));
	write(static_cast<uint8_t>(portData));
}


void
UAPFirmataClient::sendString(
    String ^string_
	)
{
    std::wstring stringW = string_->ToString()->Begin();
    std::string stringA(stringW.begin(), stringW.end());
    return ::RawFirmata.sendString(stringA.c_str());
}


void
UAPFirmataClient::sendString(
	uint8_t command_,
    String ^string_
	)
{
    std::wstring stringW = string_->ToString()->Begin();
    std::string stringA(stringW.begin(), stringW.end());
    return ::RawFirmata.sendString(command_, stringA.c_str());
}


void
UAPFirmataClient::sendSysex(
	uint8_t command_,
	uint8_t bytec_,
    String ^bytev_
	)
{
    std::wstring bytevW = bytev_->ToString()->Begin();
    std::string bytevA(bytevW.begin(), bytevW.end());
    uint8_t *bytev = reinterpret_cast<uint8_t *>(const_cast<char *>(bytevA.c_str()));
    return ::RawFirmata.sendSysex(command_, bytec_, bytev);
}


void
UAPFirmataClient::write(
	uint8_t c_
	)
{
    return ::RawFirmata.write(c_);
}
