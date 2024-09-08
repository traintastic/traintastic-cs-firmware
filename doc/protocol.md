# Traintastic CS communication protocol

Serial 115200 baud, 8 databits, no parity, 1 stopbit.


## Commands

The command format is: `<opcode> <data length> [<data>...] <checksum>`.

The highest bit of the *opcode* byte indicates if it is a host to Traintastic CS command (`0`) or a Traintastic CS to host command (`1`). The *data length* byte contains the number of data bytes in the message. The *checksum* is a bitwise eXclusive OR of all bytes of the message.


### Host to Traintastic CS

All command that can be send by the host to Traintastic CS, Traintastic CS will never send these commands to the host.

#### Reset

`0x00 0x00 0x00`

Reset Traintastic CS, everything is disabled and powered off.

Response: [ResetOk](#resetok)


#### Ping

`0x01 0x00 0x01`

Response: [Pong](#pong)


#### GetInfo

`0x02 0x00 0x02`


#### InitXpressNet

`0x03 0x00 0x03`

Enable and power on XpressNet, this command can only be sent once, to disable and power down a [Reset](#reset) must be sent.

Response: [InitXpressNetOk](#initxpressnetok)


#### InitS88

`0x04 0x02 <module count> <clock frequency> <checksum>`

- `module count`: The number of 8-port S88 modules connected.
- `clock frequency`: S88 clock frequency in kHz, minimum is 1 kHz, maximum is 250 kHz.

Enable and power on S88, this command can only be sent once, to disable and power down a [Reset](#reset) must be sent.

Response: [InitS88Ok](inits88ok)


### Traintastic CS to host

All command that can be send by the Traintastic CS to the host.


#### ResetOk

`0x80 0x00 0x80`

Send by Traintasic CS when [Reset](#reset) command is executed.


#### Pong

`0x81 0x00 0x81`

Send by Traintasic CS when a [Ping](#ping) command is received.


#### Info

`0x82 ...`


#### InitXpressNetOk

`0x83 0x00 0x83`

Send by Traintasic CS when [InitXpressNet](#initxpressnet) command is executed.


#### InitS88Ok

`0x84 0x00 0x84`

Send by Traintasic CS when [InitS88](#inits88) command is executed.


#### InputStateChanged

`0xA0 0x04 <channel> <address high> <address low> <state> <checksum>`

- `channel`: Input channel, `1`=Loconet, `2`=XpressNet, `3`=S88.
- `address high`: High byte of the 16 bit input address.
- `address low`: Low byte of the 16 bit input address.
- `state`: State of the input, `0`=Unknown, `1`=Low, `2`=High.

Send by Traintastic CS when an input state changes.


#### ThrottleSetSpeedDirection


#### ThrottleSetFunctions


#### Error

`0xFF 0x02 <opcode> <errorcode> <checksum>`

- `opcode`:
- `errorcode`:

Send by Traintatic CS if it receives an invalid command.
