using Microsoft.Maker.RemoteWiring;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RemoteWiringUnitTests
{
    [TestClass]
    public class HardwareProfileTests
    {
        [TestMethod]
        public void TestAnalogPinCountSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int totalExpectedPins = 10;

            var pins = new List<MockPin>();
            for (uint i = 0; i < totalExpectedPins; i++)
            {
                var pin = new MockPin(i);
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.ANALOG, 1));
                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(totalExpectedPins, deviceUnderTest.DeviceHardwareProfile.AnalogPinCount, "Total analog pin count on device was not communicated properly");
        }

        [TestMethod]
        public void TestAnalogOffsetSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int expectedOffset = 6;

            var pins = new List<MockPin>();
            // We setup the board to have 10 total pins
            // on the 6th pin we start inserting analog pins
            for (uint i = 0; i < 10; i++)
            {
                var pin = new MockPin(i);

                if (i >= expectedOffset)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.ANALOG, 1));

                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(expectedOffset, deviceUnderTest.DeviceHardwareProfile.AnalogOffset, "Total analog pin count on device was not communicated properly");
        }

        [TestMethod]
        public void TestTotalPinCountSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int totalExpectedPins = 10;

            var pins = new List<MockPin>();
            for (uint i = 0; i < totalExpectedPins; i++)
            {
                var pin = new MockPin(i);

                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(totalExpectedPins, deviceUnderTest.DeviceHardwareProfile.TotalPinCount, "Total pin count on device was not communicated properly");
        }

        [TestMethod]
        public void TestPinArraysProperlyPopulatedSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int totalExpectedPins = 10;
            int totalExpectedAnalogPins = 1;
            int totalExpectedDigitalPins = 2;
            int totalExpectedI2cPins = 3;
            int totalExpectedPwmPins = 4;
            int totalExpectedServoPins = 5;

            // Since we only have 5 pins with capabilities the left over will be disabled
            int totalExpectedDisabledPins = totalExpectedPins - totalExpectedServoPins;

            var pins = new List<MockPin>();
            for (uint i = 0; i < totalExpectedPins; i++)
            {
                var pin = new MockPin(i);

                if (i < totalExpectedAnalogPins)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.ANALOG, 1));

                if (i < totalExpectedDigitalPins)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.OUTPUT, 1));

                if (i < totalExpectedI2cPins)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.I2C, 1));

                if (i < totalExpectedPwmPins)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.PWM, 1));

                if (i < totalExpectedServoPins)
                    pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.SERVO, 1));

                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(totalExpectedPins, deviceUnderTest.DeviceHardwareProfile.TotalPinCount, "Total pin count on device was not communicated properly");

            Assert.AreEqual(totalExpectedAnalogPins, deviceUnderTest.DeviceHardwareProfile.AnalogPins.Count(), "Analog pins were not correctly enumerated");
            Assert.AreEqual(totalExpectedDigitalPins, deviceUnderTest.DeviceHardwareProfile.DigitalPins.Count(), "Digital pins were not correctly enumerated");
            Assert.AreEqual(totalExpectedDisabledPins, deviceUnderTest.DeviceHardwareProfile.DisabledPins.Count(), "Disabled pins were not correctly enumerated");
            Assert.AreEqual(totalExpectedI2cPins, deviceUnderTest.DeviceHardwareProfile.I2cPins.Count(), "I2c pins were not correctly enumerated");
            Assert.AreEqual(totalExpectedPwmPins, deviceUnderTest.DeviceHardwareProfile.PwmPins.Count(), "Pwm pins were not correctly enumerated");
            Assert.AreEqual(totalExpectedServoPins, deviceUnderTest.DeviceHardwareProfile.ServoPins.Count(), "Servo pins were not correctly enumerated");
        }

        [TestMethod]
        public void TestPinCapabilityAllSupportedSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int totalExpectedPins = 1;

            var pins = new List<MockPin>();
            for (uint i = 0; i < totalExpectedPins; i++)
            {
                var pin = new MockPin(i);

                // isAnalogSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.ANALOG, 1));

                // isDigitalInputSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.INPUT, 1));

                // isDigitalInputPullupSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.PULLUP, 1));

                // isDigitalOutputSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.OUTPUT, 1));

                // isI2cSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.I2C, 1));

                // isPwmSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.PWM, 1));

                // isServoSupported
                pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.SERVO, 1));

                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(totalExpectedPins, deviceUnderTest.DeviceHardwareProfile.TotalPinCount, "Total pin count on device was not communicated properly");

            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isAnalogSupported(0), "isAnalogSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isDigitalInputSupported(0), "isDigitalInputSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isDigitalInputPullupSupported(0), "isDigitalInputPullupSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isDigitalOutputSupported(0), "isDigitalOutputSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isI2cSupported(0), "isI2cSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isPwmSupported(0), "isPwmSupported did not get set properly");
            Assert.IsTrue(deviceUnderTest.DeviceHardwareProfile.isServoSupported(0), "isServoSupported did not get set properly");
        }

        [TestMethod]
        public void TestPinCapabilityNoneSupportedSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            int totalExpectedPins = 1;

            var pins = new List<MockPin>();
            for (uint i = 0; i < totalExpectedPins; i++)
            {
                var pin = new MockPin(i);

                pins.Add(pin);
            }
            var board = new MockBoard(pins);

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            // Assert
            Assert.AreEqual(totalExpectedPins, deviceUnderTest.DeviceHardwareProfile.TotalPinCount, "Total pin count on device was not communicated properly");

            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isAnalogSupported(0), "isAnalogSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isDigitalInputSupported(0), "isDigitalInputSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isDigitalInputPullupSupported(0), "isDigitalInputPullupSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isDigitalOutputSupported(0), "isDigitalOutputSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isI2cSupported(0), "isI2cSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isPwmSupported(0), "isPwmSupported did not get set properly");
            Assert.IsFalse(deviceUnderTest.DeviceHardwareProfile.isServoSupported(0), "isServoSupported did not get set properly");
        }
    }
}
