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

void MainPage::Clicked_OffButton(Object ^sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
    _arduino_uno->digitalWrite(13, 0);
    OutputText("LED off.");
}

void MainPage::Clicked_OnButton(Object ^sender, Windows::UI::Xaml::RoutedEventArgs^ e) {
    _arduino_uno->digitalWrite(13, 1);
    OutputText("LED on.");
}

void MainPage::OutputText(Platform::String^ textString)
{
    OutputBlock->SelectAll();
    OutputBlock->Text = OutputBlock->SelectedText + textString;
}
