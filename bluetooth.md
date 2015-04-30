#Hooking up your Bluetooth device

This guide walks you through the steps to hook up a Bluetooth device to your Arduino!

####Important Note:

Some Arduino devices, such as the Uno, only have a single `Serial` line to communicate over pins 0 and 1 as well as via USB to your computer. If you are using one of these devices, you will either need to program your Arduino *before* you hook up pins 0 and 1 to the Bluetooth device, or you will have to disconnect these wires *each time* you upload a sketch to your Arduino.

1. Connect the power and ground rails on the breadboard to the 5V and GND pins, respectively, on the Arduino. Using color coded wires (red and black) will make it easy to keep track of the power connections.
 
 ![Project Start](https://ms-iot.github.io/content/images/remote-wiring/samples/basic/step01.JPG)
 
2. Plug your bluetooth device on the breadboard and connect the VCC and GND pins to the power and ground rails, respectively, on the breadboard.
 
 ![VCC and ground](https://ms-iot.github.io/content/images/remote-wiring/samples/basic/step02.JPG)
 
3. Connect the TX-0 pin on the bluetooth device to the RX pin on the Arduino. Similarly, connect the RX-1 pin on the bluetooth device to the TX pin on the Arduino.
 
 ![Send and Receive](https://ms-iot.github.io/content/images/remote-wiring/samples/basic/step03.JPG)
 
   * Notice the yellow wire in the image goes from the transmit pin of the bluetooth device to the receive pin of the Arduino and vice versa for the orange wire. This step is critical to establish serial communication between the bluetooth device and the Arduino, allowing the messages transmitted from one device to be received by the other.
 
 ![Send and Receive](https://ms-iot.github.io/content/images/remote-wiring/samples/basic/step03_2.JPG)
 ![Send and Receive](https://ms-iot.github.io/content/images/remote-wiring/samples/basic/step03_3.JPG)
 
That's it! You can now send and receive Bluetooth messages automatically through StandardFirmata or by calling `Serial.begin( 115200 );` in your own (unrelated) sketches!