using Microsoft.Maker.RemoteWiring;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System.Collections.Generic;
using System.Threading;

namespace RemoteWiringUnitTests
{
    [TestClass]
    public class AnalogPinTests
    {
        [TestMethod]
        public void TestAnalogPinSetModeSuccess()
        {
            // Arrange
            RemoteDevice deviceUnderTest = null;
            RemoteDeviceHelper deviceHelper = new RemoteDeviceHelper();
            var expectedPinMode = PinMode.ANALOG;

            var pin = new MockPin(0);

            pin.SupportedModes.Add(new KeyValuePair<PinMode, ushort>(PinMode.ANALOG, 1));

            var board = new MockBoard(new List<MockPin>() { pin });

            // Act
            deviceUnderTest = deviceHelper.CreateDeviceUnderTestAndConnect(board);

            // Wait until the mock board is ready
            SpinWait.SpinUntil(() => { return deviceHelper.DeviceState == DeviceState.Ready; }, 100000);

            deviceUnderTest.pinMode("A0", PinMode.ANALOG);

            // Assert
            Assert.AreEqual(expectedPinMode, deviceUnderTest.getPinMode("A0"), "Pin mode was not set properly");
        }

        [TestMethod]
        public void TestAnalogPinReadValueSuccess()
        {
            Assert.Fail("Test Not Implemented");
        }
    }
}
