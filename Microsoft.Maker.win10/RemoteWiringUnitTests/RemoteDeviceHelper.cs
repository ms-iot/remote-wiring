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
    /// <summary>
    /// Helper class for gathering device information or connecting to devices
    /// </summary>
    public static class RemoteDeviceHelper
    {
        /// <summary>
        /// Get arduino devices connected.
        /// </summary>
        /// <remarks>Requires arduino drivers installed for name resolution</remarks>
        /// <returns>last arduino device found</returns>
        public static DeviceInformation GetDevice()
        {
            DeviceInformation returnDevice = null;
            Task<DeviceInformationCollection> task = GetDeviceListAsync();
            var result = task.Result;
            foreach (DeviceInformation device in result)
            {
                if (device.Name.Contains("Arduino"))
                    returnDevice = device;
            }

            return returnDevice;
        }

        /// <summary>
        /// Gather list of all devices (including non-arduino) attached via USB
        /// </summary>
        /// <returns>DeviceInformationCollection of devices</returns>
        public static Task<DeviceInformationCollection> GetDeviceListAsync()
        {
            CancellationTokenSource cancelTokenSource = new CancellationTokenSource(30000);
            Task<DeviceInformationCollection> task = UsbSerial.listAvailableDevicesAsync().AsTask<DeviceInformationCollection>(cancelTokenSource.Token);
            return task;
        }
    }

    internal static class ExpectedDeviceHardware
    {

    }
}
