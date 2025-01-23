#ifndef GAMEBOY_TYPES_HPP
#define GAMEBOY_TYPES_HPP

#include <cstdint>

#include <utility>

namespace gb {

// This space can be used to define other "universal" types, if needed
// in the future.
using word  = unsigned char;
using dword = uint16_t;
using addr  = uint16_t;

typedef enum {
  NOP      = 0x00,
  LD_HL_nn = 0x21,
  LD_SP_nn = 0x31,
  LD_HLm_A = 0x32,
  XOR_A    = 0xAF
} Opcode;

}

#endif  // define GAMEBOY_TYPES_HPP
