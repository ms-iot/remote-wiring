#Windows Remote Arduino

Windows Remote Arduino is an open-source [Windows Runtime Component](https://msdn.microsoft.com/en-us/magazine/jj651567.aspx?utm_source=rss&utm_medium=rss&utm_campaign=windows-runtime-reimagining-app-development-with-the-windows-runtime) library which allows Makers to control an Arduino through a Bluetooth or USB connection! It is intended for [Windows Runtime (WinRT)](https://msdn.microsoft.com/en-us/library/windows/apps/br211369.aspx) Developers who want to harness the power of Arduino hardware using the Windows Runtime Languages. Developers who include this component in their projects will automatically have access to its features in any of the WinRT languages (C++/CX, C# and JavaScript). The library is built on top of the [Firmata protocol](https://github.com/firmata/protocol), which is implemented in a pre-packaged [Firmata library](http://arduino.cc/en/reference/firmata) included in the Arduino software obtained from [http://arduino.cc](http://arduino.cc).

[View the open-source license](license.txt).

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


###The Microcontroller
Arduinos are a series of open source microcontrollers built on an established platform. An individual Arduino device is a simple programmable device that is capable of tasks such as GPIO, but also more advanced communication protocols such as I2C. Makers have the ability of quickly hooking up sensors and other devices to an Arduino and using basic Arduino “sketches” to give instructions to the hardware.

However, devices like an Arduino are fundamentally limited by the single-threaded nature of a microcontroller. Remote Arduino brings together all of the physical control of an Arduino and the supercharged software abilities of Windows devices such as multi-threading, native file systems, simple internet access, and more.

###Windows 10
Windows 10 introduces the concept of a [Universal Application Platform (UAP)](https://dev.windows.com/en-us/develop/building-universal-windows-apps). This means that developers can produce one solution that will work on any device running Windows 10, including Windows Phone 10 and Raspberry Pi 2. By including the **Windows Remote Arduino** library in your solution, you can turn any Windows device into a remote programming surface for your Arduino! Now it is possible to use the power of the Windows 10 operating system to open up a whole new set of advanced capabilities (and maker projects!) through the use of multi-threading, data storage, internet access, and natural user experiences.

#Structure
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
The implementation of Firmata is taken directly from the [Firmata repository](https://github.com/firmata/arduino), with absolute minimal changes (i.e. removing arduino/hardware dependencies), and is wrapped by a transparent [Windows Runtime Component](https://msdn.microsoft.com/en-us/library/hh441572.aspx) library class named UapFirmata.
The wrapper does not change or add functionality, it only provides parameter smoothing (i.e. `char *` -> `String`) and paradigm translation (i.e. `callbacks` -> `events`). This layer is completely independent from the RemoteWiring layer above it, so it can be used as a fully-functional Firmata implementation!

###Serial
Serial is the transport layer, which provides the physical communication between applications and the Arduino device. IStream is the interface which defines the requirements of a communication stream between the Arduino and the application itself. Currently, this is implemented in the default library with the `BluetoothSerial` class as well as `USBSerial` for wired connections on Windows 10. There are five functions which need to be implemented should you choose to extend the capabilities of the library with other communication methods. These functions MUST be guaranteed to be synchronous operations in order to be consumed by the Firmata layer.

- `begin(int, char)` -> `void` -- initializes the stream
- `end(void)` -> `void` -- finalizes the stream
- `available(void)` -> `int` -- gets the number of bytes in the stream
- `read(void)` -> `short` -- reads a single character from the incoming stream
- `write(char)` -> `void` -- writes a single character to the outgoing stream

##RemoteDevice

A user-friendly wrapper for Firmata, providing an Arduino feel for GPIO and I2C. After adding this package to your solution either manually or through NuGet, you may construct a RemoteDevice object directly.

#Setup
This section explains how to set up your Arduino and how to add the Windows Remote Arduino library to your Windows projects!

##Arduino Setup

Windows Remote Arduino uses the [Firmata protocol](https://github.com/firmata/protocol), which has implementations in many languages including Arduino! The Arduino implementation is called [StandardFirmata](https://github.com/firmata/arduino/blob/master/examples/StandardFirmata/StandardFirmata.ino) and comes pre-packaged with the Arduino software when you install it! Follow the steps below to upload the StandardFirmata sketch to your Arduino.

1. Download and install the Arduino software from [http://arduino.cc](http://arduino.cc).
2. Connect your Arduino device to the computer using USB. 
3. Launch the Arduino application.
4. Verify that you have the correct Arduino board selected under *Tools > Board*
5. Verify that you have the correct COM Port selected under *Tools > Port*
6. In the Arduino IDE, navigate to *File > Examples > Firmata > StandardFirmata*
7. Press “Upload” to deploy the StandardFirmata sketch to the Arduino device.

That’s it! Your Arduino will now run the StandardFirmata sketch forever unless reprogrammed with a different sketch. You can now optionally disconnect your Arduino from the computer and power it in any way you choose. If you wish to use the recommended Bluetooth pairing between your devices, you will need to hook up a Bluetooth device to the Arduino. We recommend the [SparkFun Bluetooth Mate Silver](https://www.sparkfun.com/products/12576).

####Note:

StandardFirmata uses the Serial lines to talk to a Bluetooth device or over USB. By default, it uses a baud rate of 57,600 bps. Depending on the configuration of your Bluetooth device, you may need to modify that rate. It can be found in the `setup` method and looks like this:

`Firmata.begin(57600);`

Simply change the `begin` parameter to match the configuration of your Bluetooth device. The most common configurations are 1152000, 57600, and 9600. The recommended SparkFun Bluetooth Mate devices use 115200 by default. If you are not sure of the default baud rate of your Bluetooth device, check the device documentation.

##Project Setup

Typically, you will want to add the Windows Remote Arduino library into your own Maker projects. The easiest way to do this is by installing the NuGet package into your projects! 

Simply open the [NuGet Package Manager Console](https://docs.nuget.org/consume/package-manager-console) and type the following command:

`Install-Package Windows.Remote.Arduino`

The necessary projects and any dependencies will automatically be downloaded and installed into your project!

[What is NuGet?](http://blogs.msdn.com/b/davidebb/archive/2010/10/05/introducing-nupack-the-smart-way-to-bring-bits-into-your-projects.aspx)

[How do I use the NuGet Package Manager?](https://docs.nuget.org/consume/package-manager-console)

However, if you'd prefer to manually download the source code and compile it yourself, either as a separate project or included in your solution, refer to the [manual installation instructions](installation.md)


#Usage
This section explains the basic usage of Windows Remote Arduino. This is an excellent place to start if you are new to this library or Arduino Wiring itself. For advanced behaviors, see the [advanced readme](advanced.md).

####C# Example:


```c#
        using namespace Microsoft.Maker.RemoteWiring;
        using namespace Microsoft.Maker.Serial;

		//create a bluetooth connection and pass it to the RemoteDevice
		IStream bt = new BluetoothSerial();
        RemoteDevice arduino = new RemoteDevice( bt );
		
		//always begin your IStream
		bt.begin( 115200, 0 );

        arduino->pinMode( 7, PinMode.OUTPUT );
        arduino->pinMode( 9, PinMode.INPUT );

        arduino->digitalWrite( 7, PinState.HIGH );
        UInt16 val = arduino->analogRead( 9 );
```

#####C++ Example:


```c++
        using namespace Microsoft::Maker::RemoteWiring;
        using namespace Microsoft::Maker::Serial;
		
		//create a bluetooth connection and pass it to the RemoteDevice
		IStream ^bt = ref new BluetoothSerial;
        RemoteDevice ^arduino = ref new RemoteDevice( bt );
		
		//always begin your IStream
		bt->begin( 115200, 0 );

        arduino->pinMode( 7, PinMode::OUTPUT );
        arduino->pinMode( 9, PinMode::INPUT );

        arduino->digitalWrite( 7, PinState::HIGH );
        uint16_t val = arduino->analogRead( 9 );
```

###Events

As previously mentioned, the RemoteWiring layer allows interactions with the RemoteDevice class to feel like interacting with the device directly through the Wiring API. However, Windows `events` give us the power to respond immediately to changes reported by the Arduino. [Click here for more information about events.](https://msdn.microsoft.com/en-us/library/hh758286.aspx)

For example, whenever you set an analog or digital pin to INPUT, the library will be notified whenever a pin value changes for digital pins, and every few milliseconds for analog pins. Windows Remote Arduino can pass these notifications on to you in the form of `events`. Simply subscribe to the event with a delegate function, and that function will automatically be called whenever it is appropriate!

####Note:

Events are often called on background threads. You may need to consider basic threading behaviors if you are storing data into an array or object created/used on your main thread, or if you are working with a user interface. When you use digital and analog read, the threading issues are taken care of by the library and are of no concern to you.

```c#
IStream bt;
RemoteDevice arduino;

public MyObject()
{
	bt = new Bluetooth Serial( "RNBT_BTLE" ); //Directly providing a device name to connect to
	arduino = new RemoteDevice( bt );

	//subscribe to the DigitalPinUpdateEvent with the name of the function to be called.
	arduino.DigitalPinUpdateEvent += MyDigitalPinUpdateCallback;
	
	//subscribe to the AnalogPinUpdateEvent with the name of the function to be called.
	arduino.AnalogPinUpdateEvent += MyAnalogPinUpdateCallback;

	//subscribe to the ConnectionEstablished event with the name of the function to be called.
	bt.ConnectionEstablished += MyConnectionEstablishedCallback;
	
	//always begin your IStream object
	bt.begin( 115200, 0 );
}

//this function will automatically be called when the bluetooth connection is established
public void MyConnectionEstablishedCallback()
{
	//set pin 7 to input mode to automatically receive callbacks when it changes
	arduino.pinMode( 7, PinMode.INPUT );
	
	arduino.pinMode( 8, OUTPUT );
	
	//you will also get a pin event when you change the value yourself.
	arduino.digitalWrite( 8, PinState.HIGH );
}

//this function will automatically be called whenever a digital pin value changes
public void MyDigitalPinUpdateCallback( byte pin, PinState state )
{
	Debug.WriteLine( "Pin D" + pin + " is now " + ( state == PinState.HIGH ? "HIGH" + "LOW" ) );
}

//this function will automatically be called whenever the analog values are reported. This is usually every few ms.
public void MyAnalogPinUpdateCallback( byte pin, UInt16 value )
{
	Debug.WriteLine( "Pin A" + pin + " is now " + value );
}

```

##Enabling Bluetooth Capabilities
In order to invoke the bluetooth capabilities of a WinRT application, you will need to open the package.appxmanifest file of your project by right-clicking and selecting the "View Code" option. Then find the <Capabilities> tag and paste the following as a child node.

###Windows 10
```xml
<DeviceCapability Name="bluetooth.rfcomm">
  <Device Id="any">
    <Function Type="name:serialPort"/>
  </Device>
</DeviceCapability>
```

##Advanced Usage
Please refer to the [Advanced Usage](advanced.md) documentation.

##Notes
For more details, visit [MSDN: How to specify device capabilities in a package manifest (Windows Runtime apps)](https://msdn.microsoft.com/en-us/library/windows/apps/dn263092.aspx).