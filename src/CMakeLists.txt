ADD_EXECUTABLE(emulator main.cpp)

ADD_SUBDIRECTORY(Frontend)
ADD_SUBDIRECTORY(Gameboy)
ADD_SUBDIRECTORY(PPU)
ADD_SUBDIRECTORY(CPU)
ADD_SUBDIRECTORY(TimerController)
ADD_SUBDIRECTORY(AddressBus)
ADD_SUBDIRECTORY(Cartridge)

TARGET_LINK_LIBRARIES(emulator Frontend)

SET_TARGET_PROPERTIES(
        emulator PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
