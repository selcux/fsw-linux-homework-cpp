# FSW Linux Homework Solution

This project implements a client application that connects to TCP server ports and processes their output according to the specified requirements.

## Dependencies

- C++17 or higher
- CMake (3.0 or higher)
- Make
- POSIX-compliant operating system (Linux)
- Standard C++ library

## Building

The project uses CMake as its build system. To build:

```bash
mkdir -p build
cmake -S . -B build
cmake --build build
```

Alternatively, you can use the provided Makefile:

```bash
make build
```

To clean the build:

```bash
make clean
```

## Running

The client applications can be run with:

```bash
# client1
./build/client1
# client2
./build/client2
```

Or, if using the Makefile:

```bash
# client1
make run-client1
# client2
make run-client2
```

> [!NOTE]
> You can use `SERVER_ADDR` to specify a different server IP address (default is `127.0.0.1`).
> For example:
> `SERVER_ADDR=192.168.1.100 make run-client1` # or client2

## Implementation Details

### Client1 Implementation

Client1 is a straightforward TCP client that connects to three server ports (4001, 4002, and 4003) and displays the received data in JSON format at regular 100ms intervals. Here's how it works:

1. **Initialization**:
   - Creates a Client instance with proper signal handling (SIGINT, SIGTERM)
   - Configures server address (defaults to 127.0.0.1 if SERVER_ADDR env variable not set)
   - Adds three TCP ports (4001, 4002, 4003) to monitor

2. **Connection Setup**:
   - Establishes non-blocking TCP connections to all configured ports
   - Creates an epoll instance for efficient I/O multiplexing
   - Initializes data structures for storing received data

3. **Main Processing Loop**:
   - Uses epoll to monitor all TCP sockets for incoming data
   - When data arrives:
     - Reads the data into a buffer
     - Removes trailing newlines
     - Stores the data in corresponding socket's position
   - Every 100ms:
     - Prints a JSON object containing:
       - Current timestamp (in milliseconds)
       - Values from all three ports (out1, out2, out3)
     - Resets data buffer with "--" placeholders

4. **Error Handling**:
   - Comprehensive error checking at each step (socket creation, connection, epoll operations)
   - Returns specific error codes and messages for different failure scenarios
   - Proper cleanup of resources on exit

### Client2 Implementation

Client2 extends Client1's functionality by adding control capabilities based on the data received. It monitors the same three TCP ports but implements additional logic to control frequency and amplitude via UDP commands.

1. **Key Differences from Client1**:
   - Shorter sampling interval (20ms instead of 100ms)
   - Adds UDP control communication on port 4000
   - Implements behavior control logic based on output3 values

2. **Control Logic**:
   - Monitors output3 (port 4003) for value changes
   - Implements threshold-based behavior:

     ```cpp
     When output3 < 3.0:
       frequency = 1000
       amplitude = 8000
     When output3 ≥ 3.0:
       frequency = 2000
       amplitude = 4000
     ```

3. **UDP Command Protocol**:
   - Sends 8-byte control messages containing:
     - Write operation code (2)
     - Object identifier (1)
     - Property code:
       - 0xFF for frequency
       - 0xAA for amplitude
     - Value to set
   - All values are sent in network byte order (big-endian)
   - Each property change requires a separate UDP message

4. **Real-time Processing**:
   - Immediately processes output3 changes
   - Sends UDP control messages without blocking
   - Uses non-blocking sockets for both TCP and UDP
   - Creates a new UDP socket for each control message to avoid state management

5. **Error Handling Additions**:
   - UDP-specific error handling
   - Data conversion error handling for output3 values
   - Network byte order conversion checks
   - Socket cleanup after each UDP transmission

> [!NOTE]
> The frequency values sent to the server assume that 1Hz equals 1000 and 2Hz equals 2000 in the server's interpretation of the control messages.

## Server Output Analysis

### Observed Patterns

1. Port 4001 (out1):
   - Sinusoidal wave pattern
   - Amplitude: ±5.0 units
   - Highest frequency among the three outputs
   - Continuous sampling with minimal gaps

2. Port 4002 (out2):
   - Jagged wave pattern
   - Amplitude: ±5.0 units
   - Medium frequency
   - Linear ramping between -5.0 and +5.0
   - Intermittent sampling with "--" gaps

3. Port 4003 (out3):
   - Square wave/step function pattern
   - Alternates between 0.0 and 5.0
   - Lowest frequency
   - Very sparse sampling with long "--" gaps
   - Most intermittent signal among the three
