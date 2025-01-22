ADD_LIBRARY(Engine STATIC engine.cpp)

TARGET_LINK_LIBRARIES(Engine Config Gameboy sfml-graphics sfml-window)
