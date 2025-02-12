ADD_LIBRARY(Frontend STATIC engine.cpp)

TARGET_LINK_LIBRARIES(Frontend Gameboy sfml-graphics sfml-window)
