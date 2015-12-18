using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RemoteWiringUnitTests
{
    public class MockBoard
    {
        public IReadOnlyList<MockPin> Pins;
        public string FirmwareName;

        public MockBoard(List<MockPin> pins)
        {
            this.FirmwareName = "Mock_Firmata";
            Pins = pins;
        }
    }
}
