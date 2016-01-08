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
    public class DigitalPinTests
    {
        [TestMethod]
        public void TestDigitalPinSetModeSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            var expectedPinMode = PinMode.INPUT;
            byte pinUnderTest = 0;

            var pin = new MockPin(0);

            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.INPUT, 1));
            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.PULLUP, 1));
            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.OUTPUT, 1));

            var board = new MockBoard(new List<MockPin>() { pin });

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            deviceUnderTest.pinMode(pinUnderTest, expectedPinMode);
            var actualPinMode = deviceUnderTest.getPinMode(pinUnderTest);

            // Assert
            Assert.AreEqual(expectedPinMode, actualPinMode, "Pin mode was not set properly in cache");
            Assert.AreEqual(expectedPinMode, board.Pins[pinUnderTest].CurrentMode, "Pin mode was communicated to board properly");
        }

        [TestMethod]
        public async Task TestDigitalPinReadValueSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            var pinMode = PinMode.INPUT;
            byte pinUnderTest = 0;
            PinState expectedPinState = PinState.HIGH;

            var pin = new MockPin(pinUnderTest);

            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.INPUT, 1));
            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.PULLUP, 1));
            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.OUTPUT, 1));

            var board = new MockBoard(new List<MockPin>() { pin });

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            deviceUnderTest.pinMode(pinUnderTest, pinMode);

            board.Pins[pinUnderTest].CurrentValue = (ushort)expectedPinState;

            // Wait for the mock board to report the state change
            await Task.Delay(100);

            var actualPinState = deviceUnderTest.digitalRead(pinUnderTest);

            // Assert
            Assert.AreEqual(expectedPinState, actualPinState, "Pin state was incorrect");
        }

        [TestMethod]
        public async Task TestDigitalPinWriteValueSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            var pinMode = PinMode.OUTPUT;
            byte pinUnderTest = 0;
            PinState expectedPinState = PinState.HIGH;

            var pin = new MockPin(pinUnderTest);

            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.INPUT, 1));
            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.OUTPUT, 1));

            var board = new MockBoard(new List<MockPin>() { pin });

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            deviceUnderTest.pinMode(pinUnderTest, pinMode);

            deviceUnderTest.digitalWrite(pinUnderTest, expectedPinState);

            // Wait for the mock board to recieve the state change
            await Task.Delay(100);

            var actualPinState = (PinState)board.Pins[pinUnderTest].CurrentValue;

            // Assert
            Assert.AreEqual(expectedPinState, actualPinState, "Pin state was incorrect");
        }
    }
}
