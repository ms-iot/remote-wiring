using Microsoft.Maker.RemoteWiring;
using Microsoft.Maker.Serial;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System;
using System.Collections.Generic;
using System.Threading;
using Windows.Devices.Enumeration;

namespace RemoteWiringUnitTests
{
    [TestClass]
    public class UnitTest1
    {
        private const int TIMEOUT = 30000;
        private uint baudRate = 9600;
        private string connectionStatus;
        private RemoteDevice arduinoDevice;
        private static IStream connection;
        private Dictionary<byte, PinState> pinStates = new Dictionary<byte, PinState>();
        private Dictionary<string, ushort> analogPinValues = new Dictionary<string, ushort>();

        [TestInitialize]
        public void InitTest()
        {
            connectionStatus = string.Empty;
            if (null != connection && connection.connectionReady())
            {
                connection.end();
                connectionStatus = null;
            }
        }

        #region Tests
        [TestMethod]
        public void Test_Connection_USB()
        {
            // asserts are in helper function
            ConnectDevice();
        }

        [TestMethod]
        public void Test_Connection_statusEvent()
        {
            ConnectDevice();

            DateTime timeout = DateTime.UtcNow.AddSeconds(30);
            System.Threading.SpinWait.SpinUntil(() => (
                timeout <= DateTime.UtcNow ||
                !String.IsNullOrEmpty(connectionStatus)
                ));

            Assert.AreEqual("Ready", connectionStatus, "OnDeviceReady event was not fired");
        }

        [TestMethod]
        public void Test_TotalPinCount()
        {
            // todo: create classes to dynamically set expected values based on board type.
            int expectedPinCount = 30;
            HardwareProfile hardware = GetHardwareProfile();
            Assert.AreEqual(expectedPinCount, hardware.TotalPinCount, "Pin Count on device incorrect");
        }

        [TestMethod]
        public void Test_AnalogPinCount()
        {
            // todo: create classes to dynamically set expected values based on board type.
            int expectedPinCount = 12;
            HardwareProfile hardware = GetHardwareProfile();
            Assert.AreEqual(expectedPinCount, hardware.AnalogPinCount, "Pin Count on device incorrect");
        }

        [TestMethod]
        public void Test_GPIOPinCount()
        {
            Assert.Fail("GPIOPinCount Not supported yet");
            //// todo: create classes to dynamically set expected values based on board type.
            //int expectedPinCount = 12;
            //HardwareProfile hardware = GetHardwareProfile();
            //Assert.AreEqual(expectedPinCount, hardware.AnalogPinCount, "Pin Count on device incorrect");
        }

        /// <summary>
        /// Tests output and input of digital pins
        /// </summary>
        /// <remarks>Requires a jumper between pins 3 & 4 </remarks>
        [TestMethod]
        public void Test_GPIOPinModeSetGet()
        {
            byte pinOut = 3;
            byte pinIn = 4;
            PinMode pinInMode = PinMode.INPUT;
            PinMode pinOutMode = PinMode.OUTPUT;
            HardwareProfile hardware = GetHardwareProfile();

            PinMode mode = arduinoDevice.getPinMode(pinIn);
            if (mode != pinInMode)
            {
                arduinoDevice.pinMode(pinIn, pinInMode);
            }
            mode = arduinoDevice.getPinMode(pinOut);
            if (mode != pinOutMode)
            {
                arduinoDevice.pinMode(pinOut, pinOutMode);
            }

            arduinoDevice.digitalWrite(pinOut, PinState.HIGH);
            arduinoDevice.DigitalPinUpdated += ArduinoDevice_DigitalPinUpdated;

            SpinWait.SpinUntil(() => (pinStates.ContainsKey(pinIn)), TIMEOUT);

            PinState state = arduinoDevice.digitalRead(pinIn);
            Assert.AreEqual(PinState.HIGH, state);
        }

        /// <summary>
        /// Tests output and input of digital pins
        /// </summary>
        /// <remarks>Requires a jumper between pins 5 & 14 (A0) </remarks>
        [TestMethod]
        public void Test_AnalogPinModeSetGet()
        {
            HardwareProfile hardware = GetHardwareProfile();

            byte pinOut = 5;
            string pinInAnalog = "A0";
            PinMode pinInMode = PinMode.ANALOG;
            PinMode pinOutMode = PinMode.OUTPUT;
            ushort expectedValue = 1023;

            PinMode mode = arduinoDevice.getPinMode(pinInAnalog);
            if (mode != pinInMode)
            {
                arduinoDevice.pinMode(pinInAnalog, pinInMode);
            }
            mode = arduinoDevice.getPinMode(pinOut);
            if (mode != pinOutMode)
            {
                arduinoDevice.pinMode(pinOut, pinOutMode);
            }

            arduinoDevice.digitalWrite(pinOut, PinState.HIGH);
            arduinoDevice.AnalogPinUpdated += ArduinoDevice_AnalogPinUpdated;

            SpinWait.SpinUntil(() => (analogPinValues.ContainsKey(pinInAnalog)), TIMEOUT);

            ushort state = arduinoDevice.analogRead(pinInAnalog);
            Assert.IsTrue( expectedValue - state <= 100, "Analog Value {0} is not within 100 of expected value {1}", state, expectedValue);
        }
        #endregion tests

        #region helpers
        private void ConnectDevice()
        {
            // Get List of devices and select Arduino
            DeviceInformation device = null;
            device = RemoteDeviceHelper.GetDevice();
            Assert.IsNotNull(device);

            // setup and start connection events
            connection = new UsbSerial(device);
            arduinoDevice = new RemoteDevice(connection);
            arduinoDevice.DeviceReady += OnDeviceReady;
            arduinoDevice.DeviceConnectionFailed += OnConnectionFailed;
            connection.begin(baudRate, SerialConfig.SERIAL_8N1);

            // Wait for connection
            DateTime timeout = DateTime.UtcNow.AddMilliseconds(TIMEOUT);
            System.Threading.SpinWait.SpinUntil(() => (
                timeout <= DateTime.UtcNow ||
                connection.connectionReady() ||
                !String.IsNullOrEmpty(connectionStatus)
                ));

            // Test if connected properly
            Assert.IsTrue(connection.connectionReady(), connectionStatus);
        }

        private void ConnectDeviceWaitOnEvent()
        {
            ConnectDevice();
            SpinWait.SpinUntil(() => { return !string.IsNullOrEmpty(connectionStatus); }, TIMEOUT);
        }

        private HardwareProfile GetHardwareProfile()
        {
            ConnectDeviceWaitOnEvent();
            return arduinoDevice.DeviceHardwareProfile;
        }
        #endregion helpers

        #region connection event handlers
        private void OnConnectionFailed(string message)
        {
            connectionStatus = "Failed: " + message;
        }

        private void OnDeviceReady()
        {
            connectionStatus = "Ready";
        }

        private void ArduinoDevice_DigitalPinUpdated(byte pin, PinState state)
        {
            this.pinStates.Remove(pin);
            this.pinStates.Add(pin, state);
        }

        private void ArduinoDevice_AnalogPinUpdated(string pin, ushort value)
        {
            this.analogPinValues.Remove(pin);
            this.analogPinValues.Add(pin, value);
        }
        #endregion event handlers
    }
}
