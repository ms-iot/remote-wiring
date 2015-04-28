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

####Note:

Whenever you construct the `UwpFirmata` object yourself, you **must** call `startListening()` in order to enable any inputs. This is done so that you can define your own behaviors, as some projects may not care about inputs, or may need to process inputs themselves. When the `UwpFirmata` object is constructed for you (as in the basic examples) this function is called for you.

**You will not receive any events, I2C replies, or valid data returned from digitalRead() or analogRead() if you do not call `startListening()` on the `UwpFirmata` object**

```c#
//member variables
IStream bluetooth;
UwpFirmata firmata;
RemoteDevice arduino;

//MyObject constructor
public MyObject()
{
	//create a default bluetooth connection
	bt = new BluetoothSerial();
	
	//construct the firmata client
	firmata = new UwpFirmata();
	
	//last, construct the RemoteWiring layer by passing in our Firmata layer.
	arduino = new RemoteDevice( firmata );
	
	//if you create the firmata client yourself, don't forget to begin it!
	firmata.begin( bt );
	
	//since we do not care about inputs in this example, we will not call firmata.startListening()!
	//firmata.startListening();
	
	//you must always call 'begin' on your IStream object to connect.
	bt.begin( 115200, 0 );
}
```

Next, we define the function to send our sysex command.

```c#
public void toggleAllDigitalPins( bool setPinsHigh )
{
	//begin our sysex command with our special command
	firmata.beginSysex( ALL_PINS_COMMAND );
	
	//let's send a '1' if we want the pins HIGH, and a 0 for LOW
	if( setPinsHigh )
	{
		firmata.appendSysex( 1 );
	}
	else
	{
		firmata.appendSysex( 0 );
	}
	
	//nothing is actually sent to the Arduino until the endSysex() function is invoked!
	firmata.endSysex();
}
```

##Implementing the Arduino side

locate the function `sysexCallback` in StandardFirmata. You'll notice that the function parameters already separate the command from the rest of the payload. This means that when our sysex command is sent the parameter `command` will be 0x42, where `argc` and `argv` will contain the *payload*, ie- anything that was added by calling `appendSysex` on the Remote Arduino side.

**Note**
Remember that Firmata is based on the MIDI standard. MIDI distinguishes *status* bytes from *data* bytes by using the MSB of each byte. If the MSB is set, then it is a status byte. If the MSB is not set, then it is a data byte. To comply with this standard and guarantee that a data byte is not mistaken as a command, all payload bytes sent with Firmata are sent as two bytes. This means that we must reassemble our bytes on the Arduino side.

The `sysexCallback` function contains a `switch (command)` statement, which will execute blocks of code depending on the value of command. We will add an additional case into this switch statement like so:

```c++
switch (command) {	//this line already exists in the sketch

  case ALL_PINS_COMMAND:
	if( argc == 2 )	//see note above
	{
		//the MSB is always sent in the LSB position 2nd byte
		byte val = argv[0] | ( argv[1] << 7 );
		
		//set the pins! On many Arduino devices (Uno, Leo, Yun, etc) there are 14 digital pins from 0 to 13.
		for( byte i = 0; i < 14; ++i )
		{
			//digitalWrite will set the pin LOW if val is 0, and HIGH if it is anything else!
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
therefore, pins 0-7 are port 0. pins 8-15 are port 1, and so on. You look to the bit position in the port value to determine the value of an individual pin. Essentially,
 `pin = ( port * 8 ) + index;`
therefore, to set only pin 2 `HIGH`, and all other pins of port 0 `LOW`, we would send the binary value 00000100 ( hex: 0x04 ) to port 0, because the bit in position 2 is set.

##Reimplementing Our Example

The sysex example was far more simple than what you would *typically* do with sysex. It was just an example of *how* do use it. Let's accomplish the same thing using ports!

Here is another look at the toggleAllDigitalPins function, reimplemented using ports instead of sysex.

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
		firmata.sendDigitalPort( 0, 0 );	//all pins low
		firmata.sendDigitalPort( 1, 0 );	//all pins low
	}
}
```