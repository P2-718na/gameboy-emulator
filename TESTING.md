# Testing

This document outlines the three main testing methods used to ensure reliability of all the components 
in the Game Boy emulator project. The test are run using the [Doctest] framework (bundled
in the source files). The aim of these tests is to cover as much of the code base as possible, all while
remaining within the limits of the Doctest framework. 

### Building and running tests

Tests are built alongside the main executable when calling `make`, or you can build them individually:
```bash
take build
cmake ..
make test
```

If you are building without CMake, make sure to build tests in debug mode! This allows the `assert` statements to
function properly. Also, make sure to copy the needed test ROMs to the correct path inside the build folder.

To run the tests, run
```bash
./test -s
```
from the build directory. The `-s` option is used to display successful tests as well.
This will run both unit tests and the test ROMs, in order. An exit value of `0` indicates
that all the tests are successful.

## 1. Unit testing

The emulator uses the [Doctest] framework for unit testing. These tests are located in the `tests/`
directory and cover individual components of the emulator:

- `cpu.test.cpp`: Tests basic CPU operation.
- `address-bus.test.cpp`: Tests basic memory access and addressing.
- `ppu.test.cpp`: Tests PPU timing and initialization.
- `timer-controller.test.cpp`: Extensively tests timer functionality.
- `cartridge.test.cpp`: Tests ROM loading and parsing.
- `gameboy.test.cpp`: Tests the main emulator class functionality and interface.
- `frontend.test.cpp`: Tests that the code throws under certain conditions.

The `Frontend` class has the fewer tests. This is because it interacts strongly with the operating
system and the user and not every feature can be tested easily. To ensure it works properly,
I have resorted to "manual" testing.

## 2. Test ROMs

Test ROMs are special Game Boy ROMs designed specifically to test the accuracy of emulators.
Unlike regular games, these ROMs contain test programs that verify specific hardware behaviors and features.
They are essential tools in emulator development because they provide a standardized way to test hardware accuracy.
Most test ROMs are developed by the community, and they can be more or less _strict_ in checking
correct behavior. They are a very useful tool to check aspects of the hardware in which multiple
components are involved simultaneously (for example, the PPU).

One small downside of using test ROMs is that they require some degree of CPU functionality in order to work.
If the CPU is not working correctly to begin with, test ROMs could fail to report a failed test,
or worse, report a test as successful even if it was not. This means that some degree of manual checking
has to be done, at least in the first phases of development.

### Blargg's tests

Blargg's tests are the most famous test ROMs available. The full test suite was written in
assembly, and it thoroughly checks most aspects of the Game Boy hardware. The tests can be run separately,
and they report their results both using the Game Boy screen and its serial port. This has the
advantage of being able to run these tests even before a working PPU implementation is available.

When running tests, the following test ROMs from Blargg's suite are automatically run:
  - `cpu_instrs`: Tests CPU instruction accuracy and timing
  - `instr_timing`: Verifies instruction cycle counts
  - `interrupt_time`: Tests interrupt timing and behavior
  - `halt_bug`: Tests the CPU's HALT instruction bug behavior

In the current version of the emulator, only `cpu_instrs` will actually pass. This is expected behavior,
and there are several reasons for this:

1. **Hardware Approximations**: The emulator makes some approximations for performance and simplicity. For example:
   - The PPU doesn't implement the exact pixel FIFO behavior
   - Some hardware bugs and edge cases aren't replicated
   - Timing is machine-cycle accurate but not pixel-cycle accurate

2. **Unimplemented Features**: Some test ROMs check features that aren't yet implemented:
   - Audio processing (APU) tests will fail as audio isn't implemented
   - RTC (Real-Time Clock) tests will fail for MBC3 cartridges
   - Some serial communication tests may fail

3. **Performance vs Accuracy Trade-offs**: Some test ROMs check for exact hardware behavior that would be
too expensive to implement perfectly. For example:
   - Exact memory bus timing
   - Precise LCD timing and pixel-by-pixel rendering
   - Hardware-level bugs that affect very few games

The goal of this emulator is to run most commercial games correctly, not to pass every test ROM.
`cpu_instrs` checks that _all_ CPU instructions work correctly, and passing this test is a satisfactory result
in itself. I have included the other ROMs in the tests to make sure that _they fail_ (as I know they should).
This should help avoid some possible false positives in testing.

I did not include any PPU-testing ROM because they would most likely fail. The PPU is an approximation
and most ROMs available only check for _exact_ results. Thus, there is no point in adding them.


## 3. Runtime Assertions

The emulator uses C++ assertions (`assert()`) throughout the codebase to catch potential issues during development
and debugging. These assertions check for:

- Invalid memory accesses
- Incorrect register values
- Invalid state transitions
- Hardware timing violations

Assertions are enabled in debug builds and are automatically disabled in release builds for performance.
Disabling assertions should not have any effect on the final results, as they should not be used as a means of control
flow, regardless.

## Test Coverage

The combination of these three testing methods provides comprehensive coverage:
This multi-layered approach helps maintain the emulator's accuracy and reliability
while balancing performance and development effort. Code Coverage results are reported as a badge in the main
README file of this repository. 

[Doctest]: https://github.com/doctest/doctest