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
        BluetoothSerial _bt_serial;
        RemoteWiring _arduino_uno;

        public MainPage()
        {
            this.InitializeComponent();

            _bt_serial = new BluetoothSerial();
            _arduino_uno = new RemoteWiring(_bt_serial);
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            OutputText("Attempt to connect.");
            _bt_serial.begin(57600, 0);
            OutputText("Acquiring connection...");
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            base.OnNavigatedFrom(e);

            OutputText("Dropping connection...");
            _bt_serial.end();
            OutputText("Disconnected.");
        }
        
        private void Pin0_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(0, 1);
                    OutputText("LED 0 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(0, 0);
                    OutputText("LED 0 off");
                }
            }
        }

        private void Pin1_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(1, 1);
                    OutputText("LED 1 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(1, 0);
                    OutputText("LED 1 off");
                }
            }
        }

        private void Pin2_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(2, 1);
                    OutputText("LED 2 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(2, 0);
                    OutputText("LED 2 off");
                }
            }
        }
        private void Pin3_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(3, 1);
                    OutputText("LED 3 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(3, 0);
                    OutputText("LED 3 off");
                }
            }
        }
        private void Pin4_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(4, 1);
                    OutputText("LED 4 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(4, 0);
                    OutputText("LED 4 off");
                }
            }
        }

        private void Pin5_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(5, 1);
                    OutputText("LED 5 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(5, 0);
                    OutputText("LED 5 off");
                }
            }
        }

        private void Pin6_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(6, 1);
                    OutputText("LED 6 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(6, 0);
                    OutputText("LED 6 off");
                }
            }
        }

        private void Pin7_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(7, 1);
                    OutputText("LED 7 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(7, 0);
                    OutputText("LED 7 off");
                }
            }
        }

        private void Pin8_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(8, 1);
                    OutputText("LED 8 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(8, 0);
                    OutputText("LED 8 off");
                }
            }
        }

        private void Pin9_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(9, 1);
                    OutputText("LED 9 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(9, 0);
                    OutputText("LED 9 off");
                }
            }
        }

        private void Pin10_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(10, 1);
                    OutputText("LED 10 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(10, 0);
                    OutputText("LED 10 off");
                }
            }
        }

        private void Pin11_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(11, 1);
                    OutputText("LED 11 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(11, 0);
                    OutputText("LED 11 off");
                }
            }
        }

        private void Pin12_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(12, 1);
                    OutputText("LED 12 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(12, 0);
                    OutputText("LED 12 off");
                }
            }
        }

        private void Pin13_ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {
            ToggleSwitch toggleSwitch = sender as ToggleSwitch;
            if (toggleSwitch != null)
            {
                if (toggleSwitch.IsOn == true)
                {
                    _arduino_uno.digitalWrite(13, 1);
                    OutputText("LED 13 on");
                }
                else
                {
                    _arduino_uno.digitalWrite(13, 0);
                    OutputText("LED 13 off");
                }
            }
        }

        private void Pin3_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }

        private void Pin5_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }

        private void Pin6_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }

        private void Pin9_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }

        private void Pin10_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }
        private void Pin11_Slider_ValueChanged(object sender, RangeBaseValueChangedEventArgs e)
        {
        }

        // Remove code below this line
        private void TextBlock_SelectionChanged_1(object sender, RoutedEventArgs e)
        {

        }

        private void Clicked_OffButton_OLD(object sender, RoutedEventArgs e)
        {
            /*AN _arduino_uno.digitalWrite(13, 0);
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
            //_arduino_uno.digitalWrite(0, 0);  // RX*/
            OutputText("LED off.");
        }

        private void Clicked_OnButton_OLD(object sender, RoutedEventArgs e)
        {
            /*AN_arduino_uno.digitalWrite(13, 1);
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
            //_arduino_uno.digitalWrite(0, 1);  // RX */
            OutputText("LED on.");
        }

        private void OutputText(String textString)
        {
            OutputBlock.SelectAll();
            OutputBlock.Text = OutputBlock.SelectedText + "Status: " + textString;
        }

        private void TextBlock_SelectionChanged(object sender, RoutedEventArgs e)
        {

        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {

        }

        private void ToggleSwitch_Toggled(object sender, RoutedEventArgs e)
        {


        }
        private void OutputBlock_SelectionChanged(object sender, RoutedEventArgs e)
        {

        }

    }
}
