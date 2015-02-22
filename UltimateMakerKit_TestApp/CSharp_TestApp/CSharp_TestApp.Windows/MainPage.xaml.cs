using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Wiring;
using Wiring.Serial;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace CSharp_TestApp
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        RemoteWiring _arduino_uno;

        public MainPage()
        {
            this.InitializeComponent();

            OutputText("Attempt to connect.");
            _arduino_uno = new RemoteWiring(new BluetoothSerial(), 57600);
            OutputText("Acquiring connection...");
        }

        private void Clicked_OffButton(object sender, RoutedEventArgs e)
        {
            _arduino_uno.digitalWrite(13, 0);
            _arduino_uno.digitalWrite(12, 0);
            _arduino_uno.digitalWrite(11, 0);
            _arduino_uno.digitalWrite(10, 0);
            _arduino_uno.digitalWrite(9, 0);
            _arduino_uno.digitalWrite(8, 0);
            _arduino_uno.digitalWrite(7, 0);
            _arduino_uno.digitalWrite(6, 0);
            _arduino_uno.digitalWrite(5, 0);
            _arduino_uno.digitalWrite(4, 0);
            _arduino_uno.digitalWrite(3, 0);
            _arduino_uno.digitalWrite(2, 0);
            //_arduino_uno.digitalWrite(1, 0);  // TX
            //_arduino_uno.digitalWrite(0, 0);  // RX
            OutputText("LED off.");
        }

        private void Clicked_OnButton(object sender, RoutedEventArgs e)
        {
            _arduino_uno.digitalWrite(13, 1);
            _arduino_uno.digitalWrite(12, 1);
            _arduino_uno.digitalWrite(11, 1);
            _arduino_uno.digitalWrite(10, 1);
            _arduino_uno.digitalWrite(9, 1);
            _arduino_uno.digitalWrite(8, 1);
            _arduino_uno.digitalWrite(7, 1);
            _arduino_uno.digitalWrite(6, 1);
            _arduino_uno.digitalWrite(5, 1);
            _arduino_uno.digitalWrite(4, 1);
            _arduino_uno.digitalWrite(3, 1);
            _arduino_uno.digitalWrite(2, 1);
            //_arduino_uno.digitalWrite(1, 1);  // TX
            //_arduino_uno.digitalWrite(0, 1);  // RX
            OutputText("LED on.");
        }

        private void OutputText(String textString) {
            OutputBlock.SelectAll();
            OutputBlock.Text = OutputBlock.SelectedText + textString;
        }
    }
}
