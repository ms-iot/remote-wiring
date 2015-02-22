//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace CPlusPlus_TestApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
	InitializeComponent();

    OutputText("Attempt to connect.");
    _arduino_uno = ref new RemoteWiring(ref new Serial::BluetoothSerial, 57600);
    OutputText("Acquiring connection...");
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	(void) e;	// Unused parameter

	// TODO: Prepare page for display here.

	// TODO: If your application contains multiple pages, ensure that you are
	// handling the hardware Back button by registering for the
	// Windows::Phone::UI::Input::HardwareButtons.BackPressed event.
	// If you are using the NavigationHelper provided by some templates,
	// this event is handled for you.
}

void MainPage::Clicked_OffButton(Object ^sender, RoutedEventArgs^ e)
{
    _arduino_uno->digitalWrite(13, 0);
    _arduino_uno->digitalWrite(12, 0);
    _arduino_uno->digitalWrite(11, 0);
    _arduino_uno->digitalWrite(10, 0);
    _arduino_uno->digitalWrite(9, 0);
    _arduino_uno->digitalWrite(8, 0);
    _arduino_uno->digitalWrite(7, 0);
    _arduino_uno->digitalWrite(6, 0);
    _arduino_uno->digitalWrite(5, 0);
    _arduino_uno->digitalWrite(4, 0);
    _arduino_uno->digitalWrite(3, 0);
    _arduino_uno->digitalWrite(2, 0);
    //_arduino_uno->digitalWrite(1, 0);  // TX
    //_arduino_uno->digitalWrite(0, 0);  // RX
    OutputText("LED off.");
}

void MainPage::Clicked_OnButton(Object ^sender, RoutedEventArgs^ e)
{
    _arduino_uno->digitalWrite(13, 1);
    _arduino_uno->digitalWrite(12, 1);
    _arduino_uno->digitalWrite(11, 1);
    _arduino_uno->digitalWrite(10, 1);
    _arduino_uno->digitalWrite(9, 1);
    _arduino_uno->digitalWrite(8, 1);
    _arduino_uno->digitalWrite(7, 1);
    _arduino_uno->digitalWrite(6, 1);
    _arduino_uno->digitalWrite(5, 1);
    _arduino_uno->digitalWrite(4, 1);
    _arduino_uno->digitalWrite(3, 1);
    _arduino_uno->digitalWrite(2, 1);
    //_arduino_uno->digitalWrite(1, 1);  // TX
    //_arduino_uno->digitalWrite(0, 1);  // RX
    OutputText("LED on.");
}

void MainPage::OutputText(Platform::String^ textString)
{
    OutputBlock->SelectAll();
    OutputBlock->Text = OutputBlock->SelectedText + textString;
}
