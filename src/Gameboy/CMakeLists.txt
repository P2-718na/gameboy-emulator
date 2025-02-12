ADD_LIBRARY(Gameboy STATIC gameboy.cpp)

TARGET_LINK_LIBRARIES(Gameboy CPU PPU TimerController AddressBus)
