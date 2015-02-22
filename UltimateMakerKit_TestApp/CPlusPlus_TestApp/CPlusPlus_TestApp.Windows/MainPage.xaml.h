//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

using namespace Platform;
using namespace Windows::UI::Xaml;
using namespace Wiring;

namespace CPlusPlus_TestApp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();
    private:
        RemoteWiring ^_arduino_uno;

        void Clicked_OffButton(Object ^sender, RoutedEventArgs^ e);
        void Clicked_OnButton(Object ^sender, RoutedEventArgs^ e);
        void OutputText(Platform::String^ textString);
    };
}
