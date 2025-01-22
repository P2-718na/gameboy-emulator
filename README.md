# gameboy-emulator
gameboy go brr

TODO


This is the second part of my coding exam [final project][1] at University of
Bologna, year one physics course. All the code in this project
(excluding external libraries) is written entirely by [me (Matteo Bonacini)][2].

--------------------------------------------------------------------------------

## Dependencies
- [Lyra](https://github.com/bfgroup/Lyra) (bundled)
- [Doctest](https://github.com/onqtam/doctest) (bundled)
- [SFML](http://www.sfml-dev.org/) (required)
- [CMake](https://cmake.org/) (recommended)

## Building
Make sure to install all the required dependencies before continuing.

The preferred way to build this code is by using CMake. Although it is
recommended to build the program in `Release` configuration for better
performance, only the `Debug` configuration satisfies all the requirements
imposed by the project assignment (`-g` and `-fsanitize="address"` flags are
only present in the latter).

```shell
# Clone the repo
git clone git@github.com:P2-718na/gameboy-emulator.git

# Create and cd to build directory
take gameboy-emulator/build

# Prepare build files. Use "Debug" instead of "Release" to build in debug mode.
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build everything
make
```

This will configure all the needed files. Three executables will be generated
(see [Running] for additional information on what they do).

```bash
gameboy

test      # Run tests
```

## Running
([tl;dr] at the end of this section for a quick, automatic setup).

TODO

### Testing
To run tests, run
```bash
./test
```

### tl;dr
Since there are many steps to running this program, I added an helper script to
quickly setup everything. After building, run  
 
```bash 
./quick-run.sh
```

from the CMake build directory. (Make sure the file is executable, if needed use
`chmod +x quick-run.sh`).

--------------------------------------------------------------------------------

## Components
What follows is a quick overview of every component of this project. More
information can be found in code comments. I didn't include a detailed
description of every class method, since the comments in the code do a
sufficiently good job at explaining just that.

### Engine
Class that handles drawing the simulation to screen, input and output. This is
responsible for:

- Printing output to the console
- Creating and updating the rendering window
- Drawing the daylight cycle tint
- Stepping forward the simulation
- Handling user input

TODO

### World
TODO

### Tests
I used two methods of testing for this project:

1. Doctest unit tests
2. `assert` statements

To run unit tests, see [Testing]. `asserts` statements are active only while
running in `Debug` mode.

--------------------------------------------------------------------------------

## Additional notes

### On code correctness
I used several tools to make sure that my code is correct, clean and consistent.
Namely:

1. Clang-Format to check code formatting ([.clang-format][B]).
2. Clang-Tidy to check for common bad-practice warnings
   ([CLion default configuration][3]).
3. cpplint.py to check for some additional details that Clang-Tidy could not
   pick up ([Google styleguide][4]).

I also run the code through _Valgrind Memcheck_ and I made sure that there are
no memory-related errors in my code.

--------------------------------------------------------------------------------

[2]: https://github.com/P2-718na
[3]: https://confluence.jetbrains.com/display/CLION/Clang-Tidy+in+CLion%3A+default+configuration?_ga=2.184137826.59717557.1623227743-1021145942.1623227743
[4]: https://google.github.io/styleguide/cppguide.html

[B]: .clang-format


[Building]: #building
[Running]: #running
[Testing]: #testing
[tl;dr]: #tldr