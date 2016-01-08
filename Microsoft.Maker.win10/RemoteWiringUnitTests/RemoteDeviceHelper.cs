using Microsoft.Maker.RemoteWiring;
using Microsoft.Maker.Serial;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;

namespace RemoteWiringUnitTests
{
    public enum DeviceState
    {
        Empty,
        Ready,
        Error
    }

    /// <summary>
    /// Helper class for gathering device information or connecting to devices
    /// </summary>
    public class RemoteDeviceHelper
    {

        public DeviceState DeviceState;

        private MockStream mockFirmataStream;

        public RemoteDevice CreateDeviceUnderTestAndConnect(MockBoard board)
        {
            // setup and start connection events
            RemoteDevice device = null;

            mockFirmataStream = new MockStream(board);
            device = new RemoteDevice(mockFirmataStream);
            device.DeviceReady += OnDeviceReady;
            device.DeviceConnectionFailed += OnConnectionFailed;
            mockFirmataStream.begin(0, SerialConfig.SERIAL_8N1);

            // Wait for connection
            DateTime timeout = DateTime.UtcNow.AddMilliseconds(10000);
            System.Threading.SpinWait.SpinUntil(() => (
                timeout <= DateTime.UtcNow ||
                mockFirmataStream.connectionReady() ||
                 this.DeviceState == DeviceState.Ready
                ));

            foreach(var pin in board.Pins)
            {
                pin.CurrentValueChanged += Pin_CurrentValueChanged;
            }

            return device;
        }

        private void Pin_CurrentValueChanged(object sender, EventArgs e)
        {
            var pin = sender as MockPin;
            this.mockFirmataStream.SendDigitalUpdateMessage((byte)pin.Number, (PinState)pin.CurrentValue);
        }

        private void OnConnectionFailed(string message)
        {
            this.DeviceState = DeviceState.Error;
        }

        private void OnDeviceReady()
        {
            this.DeviceState = DeviceState.Ready;
        }
    }
}
