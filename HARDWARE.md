# Hardware primer for the Game Boy
The internals of the Game Boy consist of a CPU, a PPU ("Picture Processing Unit"),
an APU ("Audio Processing Unit") and (eventually) an additional controller (MBC, "Memory Bank Controller")
embedded into the game cartridge. The whole machine runs at a clock speed of ~4MHz ("dot clock")
but most operations require multiple of 4 clock cycles to runs, effectively making it a 1MHz clock ("machine clock").

A very good general overview of the hardware can be found in _The Ultimate Game Boy Talk_, on
[YouTube](https://www.youtube.com/watch?v=HyzD8pNlpwI).

There does not exist one single official resource that explains _all_ the hardware. Throughout the years,
the community has put together a series of references. These are the ones I used for developing my emulator:

|           Reference            | Comment                                                                                                   | Link                                              |
|:------------------------------:|-----------------------------------------------------------------------------------------------------------|---------------------------------------------------|
|            Pandocs             | Probably the most complete reference for the Game Boy hardware as a whole.                                | https://gbdev.io/pandocs/                         |
|             GBEDG              | Incomplete "guide" for gameboy emulator development. Explained some PPU concepts better than Pandocs did. | https://hacktix.github.io/GBEDG/                  |
|              wiki              | Useful for general references.                                                                            | https://gbdev.gg8.se/wiki/articles/Main_Page      |
| "Complete" technical reference | Useful for implementing/referencing CPU instructions.                                                     | https://gekkio.fi/files/gb-docs/gbctr.pdf         |
|           CPU Manual           | Official manual from Nintendo. Information here is very inaccurate, but still useful at times.            | http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf    |
|      Z80 Game development      | Resource that describes some aspects of the Z80 CPU (The Game Boy uses a modified version of the Z80).    | https://jgmalcolm.com/z80/                        |
|          eZ80 Heaven           | Additional CPU development resource.                                                                      | https://ez80.readthedocs.io/en/latest/index.html  |