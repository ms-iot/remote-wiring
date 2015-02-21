//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

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

    protected:
        void Clicked_OffButton(Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
        void Clicked_OnButton(Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
        virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
        void OutputText(Platform::String^ textString);

    private:
        RemoteWiring ^_arduino_uno;
	};
}
