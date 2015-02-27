#RemoteWiring

The RemoteWiring library is a Windows Runtime Component that provides remote access to an Arduino Uno. By using a Windows Runtime Component, the library is automatically projected into the Windows Runtime languages of C++/CX, C# and JavaScript. RemoteWiring is built as a wrapper on top of Firmata, a well recognized protocol for interfacing with the Arduino, SparkCore and various other maker projects.

The implementation is a three layer cake, where each layer provides an entry-point exposed as a Windows Runtime Component. A consumer can choose to use the RemoteWiring interface for simple instructions, like GPIO, or the user can interface with the Firmata layer directly for more complex commands like I2C. The layered construction also allows individual layers to be swapped, which provides the ability to supply alternate implementations at each layer. For example, the current transport layer is BluetoothSerial, which subclasses the ISerial interface. BluetoothSerial could be swapped for another component adhering to the ISerial interface which provides communication over USB.

The layer structure (wherein each layer consumes the layer beneath it):

- RemoteWiring
- Firmata
- ISerial (BluetoothSerial)

##ISerial:
- `available(void) -> int`
-- gets the number of bytes in the stream

- `begin(int, char) -> void`
-- initializes the serial stream

- `end(void) -> void`
-- finalizes the serial stream

- `read(void) -> short`
-- reads a single character from the incoming stream

- `write(char) -> void`
-- writes a single character to the outgoing stream

These functions ***MUST*** be guaranteed to be synchronous operations in order to be consumed by the Firmata layer.

Check [Arduino.cc](http://arduino.cc/en/Reference/Stream) for more details.

##Firmata:
The implementation of Firmata is taken directly from the [Firmata repository](https://github.com/firmata/protocol/blob/master/protocol.md), with absolute minimal changes (i.e. removing arduino/hardware dependencies), and is wrapped by a transparent* Windows Runtime Component library class.

*The wrapper does not change or add functionality, it only provides parameter smoothing (i.e. `char *` -> `IBuffer`) and paradigm translation (i.e. callbacks -> events).

##RemoteWiring:
A user-friendly wrapper for Firmata, providing an Arduino feel for GPIO access.

Example:
```C++/CX
        using namespace Wiring::Serial;
        using namespace Wiring;
        
        BluetoothSerial ^bt = ref new BluetoothSerial;
        RemoteWiring ^arduino = ref new RemoteWiring(bt);
        
        bt->begin(57600, 0);
        
        arduino->pinMode(7, RemoteWiringDefines::OUTPUT);
        arduino->pinMode(9, RemoteWiringDefines::OUTPUT);
        
        arduino->digitalWrite(7, RemoteWiringDefines::HIGH);
        arduino->analogWrite(9, 128);
```

##Notes:

In order to invoke the bluetooth capabilities of a WinRT application, you will need to open the `package.appxmanifest` file of your project by right-clicking and selecting the "View Code" option. Then find the `<Capabilities>` tag and paste the following as a child node.
```xml
<m2:DeviceCapability Name="bluetooth.rfcomm">
  <m2:Device Id="any">
    <m2:Function Type="name:serialPort"/>
  </m2:Device>
</m2:DeviceCapability>
```

For more details, visit [MSDN: How to specify device capabilities in a package manifest (Windows Runtime apps)](https://msdn.microsoft.com/en-us/library/windows/apps/dn263092.aspx).