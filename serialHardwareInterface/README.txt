There will be a Serial Interface Manager who will be responsble for interfacing to the incoming requests
and doll them out to the appropriate Serial Interface handler.  Each Serial Interface handler will
be its own thread.  All communication between threads will be done with ZeroMQ inproc interfaces.
There will need to be different Serial Interface handlers for each different interface.  There will be
ones for TTY devices for the front/rear panel controllers, and then a custom one for handling internal,
external, and the PSU.

The Serial Interface Manager will handle the JSON parsing, and will abstract that away from
the lower level handlers.  The low level handlers will just need to worry about the communicatinos.

To test with Pseudo-TTY devices, use the following socat command:
socat pty,raw,echo=0,link=/home/mberman/ttyS98 pty,raw,echo=0,link=/home/mberman/ttyS99
Where /home/mberman/ can be replaced with any path, and the TTY device names can also be replaced
