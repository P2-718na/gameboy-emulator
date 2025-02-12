ADD_LIBRARY(Gameboy STATIC gameboy.cpp)

TARGET_LINK_LIBRARIES(Gameboy Cartridge CPU PPU TimerController AddressBus)
