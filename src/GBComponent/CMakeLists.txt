ADD_LIBRARY(GBComponent STATIC gb-component.cpp)

TARGET_LINK_LIBRARIES(GBComponent Gameboy AddressBus)
