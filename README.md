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

The client application can be run with:

```bash
./build/client
```

Or, if using the Makefile:

```bash
make run
```

## Implementation Details

### Client Implementation

The client application connects to three TCP server ports (4001, 4002, 4003) and processes their output. Here's how it works:

1. Connection Phase:
   - The client establishes separate TCP socket connections to each port
   - Each connection is verified before proceeding
   - If any connection fails, the client provides detailed error information and exits gracefully

2. Data Collection:
   - Data is read continuously from all three ports in parallel
   - A 100ms time window is used to collect data from each port
   - For each port, only the most recent value within the time window is kept
   - If no data arrives during a time window, that port's value is marked as "--"

3. Output Generation:
   - Every 100ms, the client generates a JSON formatted output
   - The output includes a timestamp (milliseconds since Unix epoch)
   - Values from all three ports are included, whether they received data or not
   - The output is written to standard output (stdout)
   - Each JSON object is written on a new line

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
