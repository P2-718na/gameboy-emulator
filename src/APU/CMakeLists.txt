ADD_LIBRARY(APU STATIC apu.cpp)

set(GBAPU_SRC
        "../../dist/gbapu/src/_internal.cpp"
        "../../dist/gbapu/src/Apu.cpp"
)

add_library(gbapu STATIC ${GBAPU_SRC})

TARGET_LINK_LIBRARIES(APU Memory gbapu sfml-audio)
