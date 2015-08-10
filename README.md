#Windows Remote Arduino

Windows Remote Arduino is an open-source [Windows Runtime Component](https://msdn.microsoft.com/en-us/magazine/jj651567.aspx?utm_source=rss&utm_medium=rss&utm_campaign=windows-runtime-reimagining-app-development-with-the-windows-runtime) library which allows Makers to control an Arduino through a Bluetooth, USB, WiFi, or Ethernet connection! It is intended for [Windows Runtime (WinRT)](https://msdn.microsoft.com/en-us/library/windows/apps/br211369.aspx) Developers who want to harness the power of Arduino hardware using the Windows Runtime Languages. Developers who include this component in their projects will automatically have access to its features in any of the WinRT languages (C++/CX, C# and JavaScript). The library is built on top of the [Firmata protocol](https://github.com/firmata/protocol), which is implemented in a pre-packaged [Firmata library](http://arduino.cc/en/reference/firmata) included in the Arduino software obtained from [http://arduino.cc](http://arduino.cc).

[View the open-source license](license.txt).

**tl;dr:** [Quick start!](#5-steps-to-remote-arduino)

##Overview
Windows Remote Arduino bridges the gap between the software world and the physical world. By leveraging the power of Windows 10 we are able to expose new possibilities of Windows devices such as a Windows Phone, Surface Pro 3, Windows desktop, and even Raspberry Pi 2. Pair your device with an Arduino and gain access to a whole new set of realities with the world at your fingertips.

##Functionality
Windows Remote Arduino enables the following functionality, right "out of the box".

1. GPIO – Analog and Digital I/O
  * digitalWrite - Digital Write
  * digitalRead - Digital Read
  * analogWrite - Analog Write
  * analogRead - Analog Read
  * pinMode - Set the mode for any pins.
  * Eventing - receive events when values change / are reported.
2. I2C/TwoWire
  * Send/Receive data to devices over I2C.
  * Subscribe to a device, repeated queries and automatic reporting.
3. Custom protocols via Firmata SysEx command

##5 Steps to Remote Arduino
You can always [clone the samples repository](http://github.com/ms-iot/windows-remote-arduino-samples) for a quick dive into 

1. [Upload StandardFirmata to your Arduino](#arduino-setup)
2. [Create a project](installation.md/#installation) or use a [sample project](http://github.com/ms-iot/windows-remote-arduino-samples)
3. Choose your connection method. Serial Commuinication like USB and Bluetooth have [a couple settings to verify], while there is also an [entire guide on hooking up your Bluetooth device](bluetooth.md#hooking-up-your-bluetooth-device). You can also use [Ethernet or WiFi, but must have the appropriate hardware](#notes-on-wifi-and-ethernet).
4. Verify your `package.appxmanifest` file in your Windows solution contains the necessary [device capabilities](installation.md#device-capabilities).
5. Review the [usage](#usage) to get started writing your Remote Arduino code!


###The Microcontroller
Arduinos are a series of open source microcontrollers built on an established platform. An individual Arduino device is a simple programmable device that is capable of tasks such as GPIO, but also more advanced communication protocols such as I2C. Makers have the ability of quickly hooking up sensors and other devices to an Arduino and using basic Arduino “sketches” to give instructions to the hardware.

However, devices like an Arduino are fundamentally limited by the single-threaded nature of a microcontroller. Remote Arduino brings together all of the physical control of an Arduino and the supercharged software abilities of Windows devices such as multi-threading, native file systems, simple internet access, and more.

###Windows 10
Windows 10 introduces the concept of a [Universal Windows Platform (UWP)](https://dev.windows.com/en-us/develop/building-universal-windows-apps). This means that developers can produce one solution that will work on any device running Windows 10, including Windows Phone 10 and Raspberry Pi 2. By including the **Windows Remote Arduino** library in your solution, you can turn any Windows device into a remote programming surface for your Arduino! Now it is possible to use the power of the Windows 10 operating system to open up a whole new set of advanced capabilities (and maker projects!) through the use of multi-threading, data storage, internet access, and natural user experiences.

#Supported Boards

This section lists boards which have been *well-tested* using all connection methods offered by this library. If a board or manufacturer is not listed, it does *not* mean that the board will not work with Windows Remote Arduino, it just means it has not been explicitly tested and verified. In fact, it is the case that most boards *will* work with this library as long as they can run StandardFirmata or an equivalent sketch that understands the Firmata protocol. 

##Arduino

1. Uno
2. Leonardo
3. Mega

####Note for USB:
There is a known issue with USB connections to Arduino Uno. The device will very often ignore up to 50 bytes of information being sent to the device only after a fresh connection is established. This issue does not exist with other connection methods. This can cause increased delays while waiting for the RemoteDevice class to correctly handshake with the device.

##DFRobot

1. Bluno
2. Bluno Beetle

####Note for USB:
There is a known issue with USB connections to DFRobot boards. Input (from the Arduino to the Windows device) does not work when connected via USB. This issue is not present under and other connection methods, and there is no known workaround at this time.


#Software Architecture
The implementation is a three layer cake, where each layer provides an entry-point exposed as a Windows Runtime Component. A Maker can choose to use the topmost layer (RemoteWiring) which exposes an interface nearly identical to Arduino Wiring for all basic instructions like GPIO and communication to other devices through I2C. The vast majority of makers will likely never need more. However, a Maker can also choose to interface with the Firmata layer directly for creating [advanced behaviors](advanced.md) for all of those crazy creative ideas that you are bound to come up with. 

##Layers
There are three layers in the architecture of Windows Remote Arduino; they are interface, protocol, and transport. Each layer is a consumer of all layers below it, and therefore dependent on them. However, a layer is never dependent on anything above it. The layered construction allows individual layers to be swapped or utilized directly at any time, which provides the ability to supply alternate implementations at each layer. The layers, in their respective ordering, are given below.

- RemoteWiring (interface)
- Firmata (protocol)
- Serial (transport)

For example, the Firmata layer translates the requests of the RemoteWiring layer into the Firmata protocol and then passes them on to the Serial layer for transport to the Arduino (and vise-versa). The Firmata layer has no knowledge of what the Serial implementation looks like, or what method of transport is actually being used. However, the Firmata layer absolutely depends on this layer in order to work. In sharp contrast, the Firmata layer is not aware of the RemoteWiring layer's existence, and therefore could be interacted with directly.

###RemoteWiring
The main interface class of the RemoteWiring layer is RemoteDevice, and it is the main API entry point that should be used in most cases. It offers an interface that is nearly identical to what you will find in the Arduino Wiring API, and should therefore be familiar if you have written Arduino sketches yourself. However, It is safe to say that all calls through this layer are directed to the Firmata layer below it, so it is only necessary to bypass or extend this layer when very advanced behaviors are desired for a project!

###Firmata
The implementation of Firmata is taken directly from the [Firmata repository](https://github.com/firmata/arduino), with absolute minimal changes (i.e. removing arduino/hardware dependencies), and is wrapped by a transparent [Windows Runtime Component](https://msdn.microsoft.com/en-us/library/hh441572.aspx) library class named UwpFirmata.
The wrapper does not change or add functionality, it only provides parameter smoothing (i.e. `char *` -> `String`) and paradigm translation (i.e. `callbacks` -> `events`). This layer is completely independent from the RemoteWiring layer above it, so it can be used as a fully-functional Firmata implementation!

One advantage UwpFirmata provides through the Windows operating system is the ability to read inputs on a background thread, rather than being constricted to a single thread of execution. In most cases, this thread is created and managed completely by the library. However, if you choose to construct the Firmata layer (UwpFirmata object) yourself, you can choose to enable this feature or not. If your project does not require reading inputs from the Arduino (or if you want to handle them yourself), you may be interested in heading to the [advanced usage](advanced.md) section for more information!

###Serial
Serial is the transport layer, which provides the physical communication between applications and the Arduino device. IStream is the interface which defines the requirements of a communication stream between the Arduino and the application itself. Currently, this is implemented in the default library with the `BluetoothSerial` class as well as `USBSerial` for wired connections on Windows 10 (USB is not supported on Windows 8.1). There are five functions which need to be implemented should you choose to extend the capabilities of the library with other communication methods. These functions MUST be guaranteed to be synchronous operations in order to be consumed by the Firmata layer.

- `begin(int, SerialConfig)` -> `void` -- initializes the stream, the SerialConfig is important when using USB. Default for Arduino is SERIAL_8N1
- `end(void)` -> `void` -- finalizes the stream
- `available(void)` -> `int` -- gets the number of bytes in the stream
- `read(void)` -> `short` -- reads a single character from the incoming stream
- `write(char)` -> `void` -- writes a single character to the outgoing stream

##RemoteDevice

A user-friendly wrapper for Firmata, providing an Arduino feel for GPIO and I2C. After adding this package to your solution either manually or through NuGet, you may construct a RemoteDevice object directly.

#Setup
This section explains how to set up your Arduino and how to add the Windows Remote Arduino library to your Windows projects!

##Arduino Setup

Windows Remote Arduino uses the [Firmata protocol](https://github.com/firmata/protocol), which has implementations in many languages including Arduino! The Arduino implementation is called [StandardFirmata](https://github.com/firmata/arduino/blob/master/examples/StandardFirmata/StandardFirmata.ino) and comes pre-packaged with the Arduino software when you install it! The raw StandardFirmata sketch works for USB and Bluetooth, while modified versions are available for WiFi and Ethernet (see below). Follow the steps below to upload the StandardFirmata sketch to your Arduino.

1. Download and install the Arduino software from [http://arduino.cc](http://arduino.cc).
2. Connect your Arduino device to the computer using USB. 
3. Launch the Arduino application.
4. Verify that you have the correct Arduino board selected under *Tools > Board*
5. Verify that you have the correct COM Port selected under *Tools > Port*
6. In the Arduino IDE, navigate to *File > Examples > Firmata > StandardFirmata*
7. Verify that StandardFirmata will use the correct baud rate for your connection (see Notes on Serial Commuinication below)
8. Press “Upload” to deploy the StandardFirmata sketch to the Arduino device.

That’s it! Your Arduino will now run the StandardFirmata sketch forever unless reprogrammed with a different sketch. You can now optionally disconnect your Arduino from the computer and power it in any way you choose. If you wish to use the recommended Bluetooth pairing between your devices, you will need to [hook up a Bluetooth device to the Arduino](bluetooth.md). We recommend the [SparkFun Bluetooth Mate Silver](https://www.sparkfun.com/products/12576).

####Notes on Serial Communication

Some hardware setups may require additional considerations when it comes to setting up your Bluetooth device over the serial pins 0 and 1.

1. *Baud Rate*: StandardFirmata uses the Serial lines to talk to a Bluetooth device or over USB. By default, it uses a baud rate of 57,600 bps. Depending on the configuration of your Bluetooth device, you may need to modify that rate. It can be found in the `setup` method and looks like this:

 `Firmata.begin(57600);`

 Simply change the `begin` parameter to match the configuration of your Bluetooth device. The most common configurations are 1152000, 57600, and 9600. The recommended SparkFun Bluetooth Mate devices use 115200 by default. USB connections should also be set to 115200. If you are not sure of the default baud rate of your Bluetooth device, check the device documentation.
 
2. *Serial vs Serial1*: Many Arduino devices, such as the Leonardo and the Yun, use `Serial1` (Rather than just `Serial`) for serial communications over pins 0 and 1. If you are using one of these devices, you will need to change the serial initialization procedure. You will want to remove the line `Firmata.begin(57600);` and replace it with the code below:

 ```
  Serial1.begin( 57600 );	//or your baud rate here, it will be 115200 if using the Bluetooth Mate Silver or Gold
  while( !Serial1 );
  Firmata.begin( Serial1 );
 ```
 
####Notes on WiFi and Ethernet

First, you must own an [Arduino WiFi shield](https://www.arduino.cc/en/Main/ArduinoWiFiShield) *or* an [Arduino Ethernet shield](https://www.arduino.cc/en/Main/ArduinoEthernetShield).

The Arduino IDE includes WiFi and Ethernet libraries written for their respective shield linked above. Unfortunately, raw StandardFirmata itself doesn't understand these libraries right 'out of the box'. However, custom libraries and Firmata sketches have been written which will allow you to easily configure Windows Remote Arduino to use these shields!

There is an additional repository which contains these libraries and sketches. It is called [standard-firmata-networking](https://github.com/turkycat/standard-firmata-networking) and is also available as open-source.

1. Clone the [standard-firmata-networking](https://github.com/turkycat/standard-firmata-networking) repository.
2. Copy the EthernetStream and/or WiFiStream folders located in \lib\ to your Arduino libraries directory (usually %HOMEPATH%\Documents\Arduino\libraries)
3. Open one of the standard-firmata-ethernet.ino or standard-firmata-wifi.ino sketch files.
4. (WiFi only) customize the *WiFi Settings* section near the top of the sketch file. This is necessary to connect to your wireless network.
5. Verify that the correct shield is attached to your Arduino.
6. Press "Upload" to deploy the Firmata sketch to the Arduino device.

##Project Setup

Typically, you will want to add the Windows Remote Arduino library into your own Maker projects. The easiest way to do this is by installing the NuGet package into your projects. NuGet is a quick and easy way to automatically install the packages and setup dependencies. Unfortunately, we do not yet have support for NuGet in Windows 10.

For now, please refer to the [manual installation instructions](installation.md)


#Usage
This section explains the basic usage of Windows Remote Arduino. This is an excellent place to start if you are new to this library or Arduino Wiring itself. For advanced behaviors, see the [advanced readme](advanced.md).

##Remote Arduino in 4 Lines
This sample shows how to construct the two necessary objects, an `IStream` implementation (in this case: `BluetoothSerial`), and the `RemoteDevice`. We then add a delegate function to the `DeviceReady` event on the `RemoteDevice` which will be called as soon as the device is ready. Last, we call `begin()` on the IStream object to start the connection process.

####C# Example:


```c#
        using namespace Microsoft.Maker.RemoteWiring;
        using namespace Microsoft.Maker.Serial;
		
		IStream connection;
		RemoteDevice arduino;

		public void SetupRemoteArduino()
		{
			//create a bluetooth connection and pass it to the RemoteDevice
			//I am using a constructor that accepts a device name or ID.
			connection = new BluetoothSerial( "MyBluetoothDevice" );
			arduino = new RemoteDevice( connection );
			
			//add a callback method (delegate) to be invoked when the device is ready, refer to the Events section for more info
			arduino.DeviceReady += Setup;
			
			//always begin your IStream
			bt.begin( 115200, SerialConfig.SERIAL_8N1 );
		}
		
		//treat this function like "setup()" in an Arduino sketch.
		public void Setup()
		{
			//set digital pin 13 to OUTPUT
			arduino.pinMode( 13, PinMode.OUTPUT );
			
			//set analog pin A0 to ANALOG INPUT
			arduino.pinMode( "A0", PinMode.ANALOG );
		}
		
		//This function will read a value from our ANALOG INPUT pin A0 (range: 0 - 1023) and set pin 13 HIGH if the value is >= 512.
		public void ReadAndReport()
		{
			UInt16 val = arduino.analogRead( "A0" );
			if( val >= 512 )
			{
				arduino.digitalWrite( 13, PinState.HIGH );
			}
			else
			{
				arduino.digitalWrite( 13, PinState.LOW );
			}
		}
```

#####C++ Example:


```c++
        using namespace Microsoft::Maker::RemoteWiring;
        using namespace Microsoft::Maker::Serial;
		
		IStream ^connection;
		RemoteDevice ^arduino;
		
		public void setupRemoteArduino()
		{
			//create a bluetooth connection and pass it to the RemoteDevice
			connection = ref new BluetoothSerial( "MyBluetoothDevice" );
			arduino = ref new RemoteDevice( connection );
			
			//add a callback method (delegate) to be invoked when the device is ready, refer to the Events section for more info
			arduino->DeviceReady += ref new Microsoft::Maker::RemoteWiring::RemoteDeviceConnectionCallback( this, &MyNamespace::MyClass::setup );
		
			//always begin your IStream
			connection->begin( 115200, SerialConfig::SERIAL_8N1 );
		}

		//treat this function like "setup()" in an Arduino sketch.
		public void setup()
		{
			//set digital pin 13 to OUTPUT
			arduino->pinMode( 13, PinMode::OUTPUT );
			
			//set analog pin A0 to ANALOG INPUT
			arduino->pinMode( "A0", PinMode::ANALOG );
		}
		
		//This function will read a value from our ANALOG INPUT pin A0 (range: 0 - 1023) and set pin 13 HIGH if the value is >= 512.
		public void readAndReport()
		{
			uint16_t val = arduino->analogRead( "A0" );
			if( val >= 512 )
			{
				arduino->digitalWrite( 13, PinState::HIGH );
			}
			else
			{
				arduino->digitalWrite( 13, PinState::LOW );
			}
		}
```

###Working with Analog

Analog input pins (A0, A1, A2, etc) have some unique properties which can cause some confusion working with them for the first time.

These pins support analogRead, but also digital functions like digitalRead and digitalWrite. Therefore, you need to consider the pinMode a little differently than you would in an Arduino IDE.

`arduino.pinMode( "A0", PinMode.INPUT );    //sets mode of pin A0 to DIGITAL input.`
`arduino.pinMode( "A0", PinMode.OUTPUT );   //sets mode of pin A0 to DIGITAL output.`
`arduino.pinMode( "A0", PinMode.ANALOG );   //sets mode of pin A0 to ANALOG input.`

Therefore, if you want to read analog values from these pins, you must set the pinmode to ANALOG, not simply INPUT or OUTPUT.

Last, when referring to analog pins, make sure to always use strings with the format "A#" (where # is the analog pin number) for the functions pinMode, analogRead, and getPinMode.

`arduino.pinMode( "A3", PinMode.ANALOG );   //will correctly set pin A3 to ANALOG INPUT mode.`
`arduino.pinMode( 3, PinMode.ANALOG );      //will do nothing, since pin 3 refers to a digital pin and does not support ANALOG INPUT.`

###Events

As previously mentioned, the RemoteWiring layer allows interactions with the RemoteDevice class to feel like interacting with the device directly through the Wiring API. However, Windows `events` give us the power to respond immediately to changes reported by the Arduino. [Click here for more information about events.](https://msdn.microsoft.com/en-us/library/hh758286.aspx)

For example, whenever you set an analog or digital pin to INPUT, the library will be notified whenever a pin value changes for digital pins, and every few milliseconds for analog pins. Windows Remote Arduino can pass these notifications on to you in the form of `events`. Simply subscribe to the event with a delegate function, and that function will automatically be called whenever it is appropriate!

####Note:

Events are often called on background threads. You may need to consider basic threading behaviors if you are storing data into an array or object created/used on your main thread, or if you are working with a user interface. When you use digital and analog read, the threading issues are taken care of by the library and are of no concern to you.

```c#
IStream connection;
RemoteDevice arduino;

public MyObject()
{
	connection = new BluetoothSerial( "MyBluetoothDevice" ); //Directly providing my device name to connect to
	arduino = new RemoteDevice( connection );

	//subscribe to the ConnectionEstablished event with the name of the function to be called.
	arduino.DeviceReady += MyDeviceReadyCallback;

	//subscribe to the DigitalPinUpdateEvent with the name of the function to be called.
	arduino.DigitalPinUpdated += MyDigitalPinUpdateCallback;
	
	//subscribe to the AnalogPinUpdateEvent with the name of the function to be called.
	arduino.AnalogPinUpdated += MyAnalogPinUpdateCallback;
	
	//always begin your IStream object
	connection.begin( 115200, 0 );
}

//this function will automatically be called when a device connection is established and the device is ready
//you may think of this like setup() in an Arduino sketch. It is the best place to prepare your
//Arduino for the logic that the rest of your program will execute
public void MyDeviceReadyCallback()
{
	//set pin 7 to INPUT mode to automatically receive callbacks when it changes
	arduino.pinMode( 7, PinMode.INPUT );
	
	//set pin 7 to ANALOG mode to automatically receive callbacks when it changes
	arduino.pinMode( "A2", PinMode.ANALOG );
}

//this function will automatically be called whenever a digital pin value changes
public void MyDigitalPinUpdateCallback( byte pin, PinState state )
{
	Debug.WriteLine( "Digital pin " + pin + " is now " + ( state == PinState.HIGH ? "HIGH" + "LOW" ) );
}

//this function will automatically be called whenever the analog values are reported. This is usually every few ms.
public void MyAnalogPinUpdateCallback( byte pin, UInt16 value )
{
	Debug.WriteLine( "Analog pin A" + pin + " is now " + value );
}

```


##Advanced Usage
Please refer to the [Advanced Usage](advanced.md) documentation.

##Notes
For more details, visit [MSDN: How to specify device capabilities in a package manifest (Windows Runtime apps)](https://msdn.microsoft.com/en-us/library/windows/apps/dn263092.aspx).