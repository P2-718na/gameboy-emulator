ADD_LIBRARY(Frontend STATIC frontend.cpp)

TARGET_LINK_LIBRARIES(Frontend Gameboy sfml-graphics sfml-window)
