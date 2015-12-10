using System;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using Microsoft.Maker.RemoteWiring;
using Microsoft.Maker.Serial;
using Windows.Devices.Enumeration;
using System.Threading.Tasks;
using System.Threading;

namespace RemoteWiringUnitTests
{
    [TestClass]
    public class UnitTest1
    {
        private uint baudRate = 9600;
        private string connectionStatus;

        [TestInitialize]
        public void InitTest()
        {
            this.connectionStatus = string.Empty;
        }
        
        [TestMethod]
        public void ConnectionTest_USB()
        {
            // Get List of devices and select Arduino
            DeviceInformation device = null;
            device = GetDevice();
            Assert.IsNotNull(device);

            // setup and start connection events
            IStream connection = new UsbSerial(device);
            RemoteDevice arduinoDevice = new RemoteDevice(connection);
            arduinoDevice.DeviceReady += OnDeviceReady;
            arduinoDevice.DeviceConnectionFailed += OnConnectionFailed;
            connection.begin(baudRate, SerialConfig.SERIAL_8N1);

            // Wait for connection
            DateTime timeout = DateTime.UtcNow.AddSeconds(30);
            System.Threading.SpinWait.SpinUntil(() => (
                timeout <= DateTime.UtcNow ||
                connection.connectionReady() ||
                !String.IsNullOrEmpty(this.connectionStatus)
                ));

            // Test if connected properly
            Assert.IsTrue(connection.connectionReady(), this.connectionStatus);

            timeout = DateTime.UtcNow.AddSeconds(30);
            System.Threading.SpinWait.SpinUntil(() => (
                timeout <= DateTime.UtcNow ||
                !String.IsNullOrEmpty(this.connectionStatus)
                ));

            Assert.AreEqual(this.connectionStatus, "Ready", "OnDeviceReady event was not fired");
        }

        private static DeviceInformation GetDevice()
        {
            DeviceInformation returnDevice = null;
            Task<DeviceInformationCollection> task = GetDeviceAsync();
            var result = task.Result;
            foreach (DeviceInformation device in result)
            {
                if (device.Name.Contains("Arduino"))
                    returnDevice = device;
            }

            return returnDevice;
        }

        private static Task<DeviceInformationCollection> GetDeviceAsync()
        {
            CancellationTokenSource cancelTokenSource = new CancellationTokenSource(30000);
            Task<DeviceInformationCollection> task = UsbSerial.listAvailableDevicesAsync().AsTask<DeviceInformationCollection>(cancelTokenSource.Token);
            return task;
        }

        private void OnConnectionFailed(string message)
        {
            this.connectionStatus = "Failed: " + message;
        }

        private void OnDeviceReady()
        {
            this.connectionStatus = "Ready";
        }
    }
}
