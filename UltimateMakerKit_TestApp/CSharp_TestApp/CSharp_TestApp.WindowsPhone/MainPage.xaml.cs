﻿using System;
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
using Wiring.Firmata;
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

            this.NavigationCacheMode = NavigationCacheMode.Required;
            _bt_serial = new BluetoothSerial();
            _arduino_uno = new RemoteWiring(_bt_serial);
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.
        /// This parameter is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // TODO: Prepare page for display here.

            // TODO: If your application contains multiple pages, ensure that you are
            // handling the hardware Back button by registering for the
            // Windows.Phone.UI.Input.HardwareButtons.BackPressed event.
            // If you are using the NavigationHelper provided by some templates,
            // this event is handled for you.
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
