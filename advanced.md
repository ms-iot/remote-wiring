#Advanced Usage

While the RemoteDevice class is intended to be the main interface between your apps and the Arduino device, you may find it necessary to break outside of the basic functionality and define your own advanced behaviors. This might occur when you need to clock large amounts of data to another device (like an LED strip), or if you want to execute multiple commands on the Arduino by sending only a single command over Bluetooth. Fortunately, this is possible within the Firmata protocol by defining a Sysex command. A Sysex command is a special command type that is very extensible, allowing you to fully customize the behavior of the Arduino without modifying the Firmata or Remote Arduino libraries. However, you will need to add some code to the StandardFirmata sketch to handle your new custom command.

As an example, let's turn on (set `HIGH`) or off (set `LOW`) all fourteen of the Arduino's digital pins with a Sysex command. While this is possible using the RemoteDevice class, it requires fourteen `digitalWrite` commands to be sent and processed. We can instead do this with Sysex, and will serve as a basic example of how to simplify complex behaviors into a single command.


#Advanced Topic #1: Sysex

##Firmata Callbacks
As a short primer, it will help to gain a small understanding of how StandardFirmata processes the incoming commands that it receives. 

The Firmata library operates by mapping incoming data to certain command types. It allows the maker consuming the library to attach methods (callbacks) to each command type. These callbacks will then be executed when the appropriate command type is fully received. These types include `DIGITAL_MESSAGE`, `SET_PIN_MODE`, and `START_SYSEX`. This design allows the Firmata consumer to not be concerned with the library's behavior in the background, but simply to attach methods to the relevant command types they expect to receive and execute their logic for the command inside the given method.

StandardFirmata attaches callbacks to each of the command types, meaning that a callback method is already implemented for sysex commands. Fortunately, this means we only need to add our customizations to this method!

##Sysex Overview
StandardFirmata already uses Sysex commands to perform various tasks on the Arduino, such as sending raw string data or processing I2C transactions. Fortunately, we don't need to understand much about how it is currently being used, we just need to make sure we define a unique command byte for our new command.

Every Sysex command must have a unique command byte which will allow the Arduino to determine *which* sysex command it is receiving. Think of this byte like a command identifier. You can find a list of reserved and pre-defined command bytes at [Firmata's Sysex documentation page](https://github.com/firmata/protocol/blob/master/protocol.md#sysex-message-format). In general however, anything between 0x10 and 0x60 is fair game.

##Implementation

In our example, we will use 0x42 as our command byte. We start by defining our command byte in both the StandardFirmata sketch as well as the UWP application using the Windows Remote Arduino library.

####StandardFirmata
```c++
#define ALL_PINS_COMMAND 0x42
```

####UWP Application
```c#
const byte ALL_PINS_COMMAND = 0x42;
```

###Implementation in your Windows Remote Arduino project

We will bypass the RemoteWiring layer and use the Firmata layer directly to send Sysex commands. In the basic example, we constructed the `RemoteDevice` object by passing our `IStream` object of choice (`BluetoothSerial`), in which case the Firmata layer is constructed for us. In this example, we're going to construct that layer manually so that we can keep a reference to it.

```c#
//member variables
IStream bluetooth;
UwpFirmata firmata;
RemoteDevice arduino;

//MyObject constructor
public MyObject()
{
	//create a bluetooth connection by specifying the device name, in this case
	bluetooth = new BluetoothSerial( "MyBluetoothDevice" );
	
	//construct the firmata client
	firmata = new UwpFirmata();
	
	//last, construct the RemoteWiring layer by passing in our Firmata layer.
	arduino = new RemoteDevice( firmata );
	
	//if you create the firmata client yourself, don't forget to begin it!
	firmata.begin( bluetooth );
	
	//you must always call 'begin' on your IStream object to connect.
	//these parameters do not matter for bluetooth, as they depend on the device. However, these are the best params to use for USB, so they are illustrated here
	bluetooth.begin( 115200, SerialConfig.SERIAL_8N1 );
}
```

Next, we define the function to send our sysex command.

```c#
public void toggleAllDigitalPins( bool setPinsHigh )
{
	//we're defining our own command, so we're free to encode it how we want.
	//let's send a '1' if we want the pins HIGH, and a 0 for LOW
	byte b;
	if( setPinsHigh )
	{
		b = 1;
	}
	else
	{
		b = 0;
	}
	
	//invoke the sendSysex command with ALL_PINS_COMMAND and our data payload as an IBuffer
	firmata.sendSysex( ALL_PINS_COMMAND, new byte[] { b }.AsBuffer() );
}
```

sendSysex accepts an IBuffer object (from Windows.Storage.Streams namespace) There are several ways to create an IBuffer object. The above shows an easy way to generate one in C# from a byte array, but this doesn't work in C++! Let's take another look at how to generate an IBuffer using a method that works in either C# or C++.

```C++
public void toggleAllDigitalPins( bool setPinsHigh )
{
	//a DataWriter object uses an IBuffer as its backing storage
	//we'll write our data to this object and detach the buffer to invoke sendSysex()
	Windows::Storage::Streams::DataWriter ^writer = new Windows::Storage::Streams::DataWriter();
	
	//we're defining our own command, so we're free to encode it how we want.
	//let's send a '1' if we want the pins HIGH, and a 0 for LOW
	writer->WriteByte( setPinsHigh ? 1 : 0 );
	
	//invoke the sendSysex command with ALL_PINS_COMMAND and our data payload as an IBuffer
	firmata->sendSysex( ALL_PINS_COMMAND, writer->DetachBuffer() );
}
```

##Implementing the Arduino side

locate the function `sysexCallback` in StandardFirmata. You'll notice that the function parameters already separate the command from the rest of the payload. This means that when our sysex command is sent the parameter `command` will be 0x42, where `argc` and `argv` will contain the *payload*, which in this case is either 1 or 0.

**Note**
Remember that Firmata is based on the MIDI standard. MIDI distinguishes *status* bytes from *data* bytes by using the MSB of each byte. If the MSB is set, then it is a status byte. If the MSB is not set, then it is a data byte. To comply with this standard and guarantee that a data byte is not mistaken as a command, all payload bytes sent with Firmata *must not* have the MSB set. sendSysex will automatically clear the MSB from each byte in the payload, so if you are sending values which use more than 7 bits of resolution (can be greater than 127 in value), you must manually split them into two or more 7-bit bytes and reassemble them on the Arduino.

The `sysexCallback` function contains a `switch (command)` statement, which will execute blocks of code depending on the value of command. We will add an additional case into this switch statement like so:

```c++
switch (command) {	//this line already exists in the sketch

  case ALL_PINS_COMMAND:
	if( argc == 1 )	//verify we received the whole message
	{
		//the MSB is always sent in the LSB position 2nd byte
		byte val;
		if( argv[0] == 1 )
		{
			val = HIGH;
		}
		else
		{
			val = LOW;
		}
		
		//set the pins! On many Arduino devices (Uno, Leo, Yun, etc) there are 14 digital pins from 0 to 13.
		for( byte i = 0; i < 14; ++i )
		{
			digitalWrite( i, val );
		}
	}
	
	//don't forget to break your case statement, or the code will continue to execute the statement below!
	break;
	
	case I2C_WRITE:			//this line already exists in the sketch
	...						//switch statement continues with other code
```

#Advanced Topic #2: Ports

Many hardware devices use ports to monitor & control the state of GPIO pins. Since digital pins can only be in two states: `HIGH` or `LOW`, it is easy to correspond these states with the values 1 and 0. Therefore, hardware devices can use a single 8-bit register to store the state of 8 pins at once. These registers are called ports, and can directly be associated with the values of digital pins.

##Why is this important?

Firmata also deals with ports when it comes to digital values. After all, it is the format which the pin states are retrieved from the hardware, and it is much more efficient to send an entire port at once than individual pins (especially when multiple pins in a single port need to be updated).

Therefore, Firmata itself uses ports to skip dealing with things as 'pins' and sending (or receiving) them individually. By interfacing with the Firmata layer directly, it is possible to update the state of 8 pins at once and to subscribe to port updates as a whole, rather than pins. In fact, when you subscribe to pin updates in the RemoteWiring layer, the RemoteDevice class is subscribed to the port events from Firmata, converts these into pin values for each pin value that has changed, and provides you an event for each one.

there is a simple math function to convert a pin into a port, which works only because of integer division:
 `port = pin / 8;`
therefore, pins 0-7 are port 0. pins 8-15 are port 1, and so on. You look to the bit position (index) in the port value to determine the value of an individual pin. Essentially,
 `pin = ( port * 8 ) + index;`
therefore, to set only pin 2 `HIGH`, and all other pins of port 0 `LOW`, we would send the binary value 00000100 ( hex: 0x04 ) to port 0, because the bit in position 2 is set.

##Reimplementing Our Example

The sysex example was far more simple than what you would *typically* do with sysex. It was just an example of *how* do use it. Let's accomplish the same thing using ports!

Here is another look at the toggleAllDigitalPins function, reimplemented using ports instead of sysex. `sendDigitalPort` accepts two `uint8_t` parameters; The first parameter represents the port number, while the second parameter is a bitmask representing the state of the 8 pins in that port. The lowest numbered pin is represented in the LSB position of the mask, and the highest numbered pin in the MSB position.

```c#
public void toggleAllDigitalPins( bool setPinsHigh )
{
	/*
	 * we have 14 pins, so that is pin 0-7 in port 0, and pin 8-13 in port 1.
	 */
	if( setPinsHigh )
	{
		firmata.sendDigitalPort( 0, 0xFF );	//all 8 pins of port 0 HIGH
		firmata.sendDigitalPort( 1, 0x3F ); //the first 6 pins of port 1 HIGH
	}
	else
	{
		firmata.sendDigitalPort( 0, 0x00 );	//all pins low
		firmata.sendDigitalPort( 1, 0x00 );	//all pins low
	}
}
```

#Advanced Topic #3: Firmata & Mutex Locking

One fairly large detail we have glossed over so far is the concept of locking the UwpFirmata instance. Those of you familiar with the concept of a mutex will have no trouble understanding what this is for; but for those that aren't, we'll go into some details here.

##Thread safety

Firmata works by passing short messages back and forth between the interacting devices. Bad things could happen if two threads, maybe responding to button clicks, attempt to send messages at the same time. If the device on the other end receives these messages intertwined, it is likely it will not be able to understand them. The best case scenario in this case is that nothing happens, but much worse things could happen. Furthermore, you are unlikely to be made aware of this issue as it occurs!

To prevent this, we've added locking functionality to the UwpFirmata class. When you use RemoteDevice directly, this additional logic is all taken care of for you. Therefore, it is impossible for two `digitalWrite()` calls to get intertwined, for example. However, when you use the UwpFirmata object yourself, it is best practice to `@lock()` and '@unlock()` the object before and after each time it is used, to guarantee that no other logic can interact with the same UwpFirmata object while your logic is taking place.

To demonstrate this, I've written a short sample class that will spin up two threads. The first thread will write an analog value to pin 9 and 10 at regular intervals. The second thread will randomly read the value of pin 13. Let's assume there exists special logic in our version of StandardFirmata that will modify the value of pin 13 (HIGH or LOW) based on the values written to pins 9 and 10.

```c#
public class AnalogWriteAndReport
{
	IStream connection;
	UwpFirmata firmata;
	RemoteDevice arduino;

	//these volatile variables allow me to terminate my threads safely when necessary.
	public volatile bool isRunning;

	public AnalogWriteAndReport()
	{
		//first, we construct our objects. I'm using NetworkSerial to connect to my Arduino over a LAN
		connection = new NetworkSerial( new Windows.Networking.HostName( "192.168.0.113" ), 5000 );
		firmata = new UwpFirmata();
		arduino = new RemoteDevice( firmata );

		//attach the connection to the UwpFirmata object with begin()
		firmata.begin( connection );

		//attach our ConnectionEstablished callback so we can be informed that the device is ready!
		connection.ConnectionEstablished += OnNetworkConnectionEstablished;

		//finally, tell the connection to begin. These arguments (baud and serialconfig) don't matter for the NetworkSerial class,
		//but when they do matter, I like to use 115200 baud rate, and 8N1 is necessary when using USB connections.
		connection.begin( 115200, SerialConfig.SERIAL_8N1 );
	}

	//this function is called when the connection is established, and the device is ready.
	private void OnNetworkConnectionEstablished()
	{
		//I'm going to set pins 9 and 10 to PWM mode, so that I can write analog values to them
		arduino.pinMode( 9, PinMode.PWM );
		arduino.pinMode( 10, PinMode.PWM );

		//Now, I will set pin 13 to INPUT so that it enables reporting, and we'll be informed whenever the pin value changes.
		arduino.pinMode( 13, PinMode.INPUT );

		//next, I'm going to spin up two threads. One will write values to these pins, and the other will periodically read them
		isRunning = true;
		Task.Run( () => Read() );
		Task.Run( () => Write() );
	}

	/*
	 * This function will loop until writerThreadShouldExit is set to true. It will constantly write
	 * a value in the range of [0, 127] to pin 9 and twice that value to pin 10.
	 * 
	 * Remember: there exists special logic in our version of StandardFirmata that will modify the value of pin 10 (HIGH or LOW) based on the value written to these pins.
	 */
	private void Write()
	{
		int pinValue = 0;
		while( isRunning )
		{
			if( pinValue > 127 ) pinValue = 0;

			//this is our "critical section". Since pin 10 must always be twice the value of pin 9, we're going to lock
			//before we send these messages. This guarantees that no other thread can use this UwpFirmata object while we 'own' the lock.
			//it would be very bad if someone tried to read pin 13 in between our two messages!
			firmata.@lock();
			firmata.sendAnalog( 9, pinValue );
			firmata.sendAnalog( 10, pinValue * 2 );
			firmata.@unlock();  //don't forget to unlock!

			pinValue++;
			Task.Delay( 100 );
		}
	}

	/*
	 * This function might simulate a client requesting the current value of pin 13 at random times.
	 * Since pin 13's value depends on the values of pins 9 and 10, it would be very bad if this thread
	 * attempted to read the value of this pin while the other thread is in the middle of changing pins 9 and 10!
	 */
	private void Read()
	{
		Random random = new Random();
		while( isRunning )
		{                
			//we do not need to lock here, since the RemoteDevice will do that for us.
			PinState state = arduino.digitalRead( 13 );

			//do something with the state variable here, like report it to the client!

			//a random delay up to 10 seconds may simulate the random attempts to read the state by a client
			Task.Delay( random.Next( 10000 ) );
		}
	}
}
```