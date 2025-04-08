ADD_EXECUTABLE(test
        sample.test.cpp
        cpu.test.cpp
        address-bus.test.cpp
        cartridge.test.cpp
        gameboy.test.cpp
        ppu.test.cpp
        #timer-controller.test.cpp
        blargg.test.cpp
)
TARGET_LINK_LIBRARIES(test Cartridge PPU Gameboy CPU AddressBus TimerController)

SET_TARGET_PROPERTIES(test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
