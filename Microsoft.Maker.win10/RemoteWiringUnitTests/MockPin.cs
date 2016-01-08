using Microsoft.Maker.RemoteWiring;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RemoteWiringUnitTests
{
    public class MockPin
    {
        private ushort currentValue;

        public MockPin(uint pinNumber)
        {
            this.SupportedModes = new List<KeyValuePair<PinMode, ushort>>();
            this.CurrentMode = PinMode.IGNORED;
            this.currentValue = 0;
            this.Number = pinNumber;
        }

        public List<KeyValuePair<PinMode, UInt16>> SupportedModes;
        public PinMode CurrentMode;
        public uint Number;

        public event EventHandler CurrentValueChanged;

        public ushort CurrentValue
        {
            get
            {
                return this.currentValue;
            }
            set
            {
                this.currentValue = value;
                CurrentValueChanged(this, new EventArgs());
            }
        }
    }
}
