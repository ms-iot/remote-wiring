using Microsoft.Maker.Firmata;
using Microsoft.Maker.RemoteWiring;
using Microsoft.Maker.Serial;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RemoteWiringUnitTests
{
    class MockStream : IStream
    {
        // Test Values
        public MockBoard Board;

        public bool ConnectionReady;
        public ushort BufferSize;
        public List<UInt16> ResponseBuffer;
        public int WriteBufferIndex;
        public List<UInt16> ActiveReadBuffer;
        public List<UInt16> LastFlushedReadBuffer;
        public uint BaudRate;

        private bool writeBufferFlushing;

        public event IStreamConnectionCallback ConnectionEstablished;
        public event IStreamConnectionCallbackWithMessage ConnectionFailed;
        public event IStreamConnectionCallbackWithMessage ConnectionLost;

        public void FlushResponseBuffer()
        {
            if (!this.writeBufferFlushing)
                this.writeBufferFlushing = true;
        }

        public MockStream(MockBoard board)
        {
            this.Board = board;
            this.ConnectionReady = true;
            this.BufferSize = 100;
            this.ResponseBuffer = new List<UInt16>();
            this.ActiveReadBuffer = new List<UInt16>();
            this.LastFlushedReadBuffer = new List<UInt16>();
        }

        public ushort available()
        {
            return this.BufferSize;
        }

        public void begin(uint baud_, SerialConfig config_)
        {
            this.BaudRate = baud_;
        }

        public bool connectionReady()
        {
            return this.ConnectionReady;
        }

        public void end()
        {
            throw new NotImplementedException();
        }

        public void flush()
        {
            this.LastFlushedReadBuffer = new List<UInt16>(this.ActiveReadBuffer);
            this.ActiveReadBuffer.Clear();
            if (this.LastFlushedReadBuffer.Count == 0) return;

            bool isSysEx = false;

            Command command = (Command)this.LastFlushedReadBuffer[0];

            switch(command)
            {
                case Command.START_SYSEX:
                    isSysEx = true;
                    break;
                case Command.SET_PIN_MODE:
                    this.Board.Pins[this.LastFlushedReadBuffer[1]].CurrentMode = (PinMode)this.LastFlushedReadBuffer[2];
                    break;
                case Command.DIGITAL_MESSAGE:
                    var portNumber = this.LastFlushedReadBuffer[0] & 0xF;

                    ushort portValue = (ushort)(this.LastFlushedReadBuffer[1] | (this.LastFlushedReadBuffer[2] << 7));
                    var pinValue = new BitArray(BitConverter.GetBytes(portValue));

                    var totalPins = this.Board.Pins.Count();
                    var offset = portNumber * 8;

                    for (int pinCounter = offset; (pinCounter < totalPins) && (pinCounter < offset + 8); pinCounter++)
                    {
                        this.Board.Pins[pinCounter].CurrentValue = Convert.ToUInt16(pinValue[pinCounter - offset]);
                    }
                    break;
            }

            if (isSysEx)
            {
                switch ((SysexCommand)this.LastFlushedReadBuffer[1])
                {
                    case SysexCommand.CAPABILITY_QUERY:
                        this.sendMessage(prepareCapabilityResponseMessage(this.Board));
                        break;
                }
            }

            this.LastFlushedReadBuffer.Clear();
        }

        public void @lock()
        {
            Debug.WriteLine("Lock requested");
        }

        public ushort read()
        {
            SpinWait.SpinUntil(() => { return this.writeBufferFlushing; }, 10000000);
            if (this.WriteBufferIndex == this.ResponseBuffer.Count())
            {
                // This value signifies the end of the stream
                this.WriteBufferIndex = 0;
                this.writeBufferFlushing = false;
                return 65535;
            }

            var buffer = this.ResponseBuffer[this.WriteBufferIndex];
            this.WriteBufferIndex++;
            return buffer;            
        }

        public void unlock()
        {
            Debug.WriteLine("Unlock requested");
        }

        public ushort write(byte c_)
        {
            this.ActiveReadBuffer.Add(c_);
            return 1;
        }

        public void SendDigitalUpdateMessage(byte pinNumber, PinState state)
        {
            sendMessage(prepareDigitalUpdateMessage(pinNumber, state));
        }

        private void sendMessage(List<UInt16> message)
        {
            this.ResponseBuffer.Clear();
            this.ResponseBuffer = message;
            this.FlushResponseBuffer();
        }

        private static List<UInt16> prepareCapabilityResponseMessage(MockBoard board)
        {
            var message = new List<UInt16>();
            message.Add((ushort)Command.START_SYSEX);
            message.Add((ushort)SysexCommand.CAPABILITY_RESPONSE);

            foreach (var pin in board.Pins)
            {
                foreach (var mode in pin.SupportedModes)
                {
                    message.Add((ushort)mode.Key);
                    message.Add(mode.Value);
                }

                message.Add(127);
            }

            message.Add((ushort)Command.END_SYSEX);
            return message;
        }

        private static List<UInt16> prepareDigitalUpdateMessage(byte pinNumber, PinState state)
        {
            var message = new List<UInt16>();

            int port;
            UInt16 portValue;

            getCurrentDigitalPortValueForPin(pinNumber, state, out port, out portValue);

            var commandByte = ((byte)Command.DIGITAL_MESSAGE) | (port & 0x0F);

            message.Add((ushort)commandByte);
            message.Add(portValue);
            message.Add((ushort)(portValue >> 7));
            return message;
        }

        private static void getCurrentDigitalPortValueForPin(byte pinNumber, PinState state, out int port, out UInt16 portValue)
        {
            portValue = 0;
            port = (pinNumber / 8);

            byte port_mask = (byte)(1 << (pinNumber % 8));

            if (PinState.HIGH == state)
            {
                portValue |= port_mask;
            }
            else
            {
                portValue &= (UInt16)~port_mask;
            }
        }

        public ushort print(byte[] buffer_)
        {
            throw new NotImplementedException();
        }

        public ushort print(double value_, short decimal_place_)
        {
            throw new NotImplementedException();
        }

        public ushort print(double value_)
        {
            throw new NotImplementedException();
        }

        public ushort print(uint value_, Radix base_)
        {
            throw new NotImplementedException();
        }

        public ushort print(uint value_)
        {
            throw new NotImplementedException();
        }

        public ushort print(int value_, Radix base_)
        {
            throw new NotImplementedException();
        }

        public ushort print(int value_)
        {
            throw new NotImplementedException();
        }

        public ushort print(byte c_)
        {
            throw new NotImplementedException();
        }

        public ushort write(byte[] buffer_)
        {
            throw new NotImplementedException();
        }
    }
}
