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

// Todo rename all opcodes with parenthesis as indirect
typedef enum : word {
  NOP      = 0x00,

  XOR_A    = 0xAF,

  JR_e     = 0x18,
  JR_Z_e   = 0x28,
  JR_NZ_e  = 0x20,

  PUSH_BC  = 0xC5,
  POP_BC   = 0xC1,

  LD_A_n   = 0x3E,
  LD_B_n   = 0x06,
  LD_C_n   = 0x0E,
  LD_D_n   = 0x16,
  LD_E_n   = 0x1E,
  LD_H_n   = 0x26,
  LD_L_n   = 0x2E,
  LD_HL_n   = 0x36,

  LD_A_A   = 0x7F,
  LD_A_B   = 0x78,
  LD_A_C   = 0x79,
  LD_A_D   = 0x7A,
  LD_A_E   = 0x7B,
  LD_A_H   = 0x7C,
  LD_A_L   = 0x7D,
  LD_A_DE  = 0x1A,
  LD_C_A   = 0x4F,
  LD_D_A   = 0x57,
  LD_D_B   = 0x50,
  LD_D_C   = 0x51,
  LD_D_D   = 0x52,
  LD_D_E   = 0x53,
  LD_D_H   = 0x54,
  LD_D_L   = 0x55,
  LD_H_A   = 0x67,
  LD_H_B   = 0x60,
  LD_H_C   = 0x61,
  LD_H_D   = 0x62,
  LD_H_E   = 0x63,
  LD_H_H   = 0x64,
  LD_H_L   = 0x65,
  LD_DE_nn = 0x11,
  LD_nn_A  = 0xEA,
  LD_HL_A  = 0x77,
  LD_HL_nn = 0x21,
  LD_HLm_A = 0x32,
  LD_HLp_A = 0x22,
  LD_SP_nn = 0x31,
  LDH_C_A  = 0xE2,
  LDH_n_A  = 0xE0,
  LDH_A_n  = 0xF0,

  INC_A    = 0x3C,
  INC_B    = 0x04,
  INC_C    = 0x0C,
  INC_D    = 0x14,
  INC_E    = 0x1C,
  INC_H    = 0x24,
  INC_L    = 0x2C,
  INC_iHL  = 0x34, // Indirect HL
  DEC_A    = 0x3D,
  DEC_B    = 0x05,
  DEC_C    = 0x0D,
  DEC_D    = 0x15,
  DEC_E    = 0x1D,
  DEC_H    = 0x25,
  DEC_L    = 0x2D,
  DEC_iHL  = 0x35, // Indirect HL
  CP_n     = 0xFE,

  INC_BC   = 0x03,
  INC_DE   = 0x13,
  INC_HL   = 0x23, // Direct HL
  INC_SP   = 0x33,
  DEC_BC   = 0x0B,
  DEC_DE   = 0x1B,
  DEC_HL   = 0x2B, // Direct HL
  DEC_SP   = 0x3B,

  RLA      = 0x17,
  RLCA     = 0x07,
  CB       = 0xCB,

  RET      = 0xC9,
  CALL_nn  = 0xCD,
} Opcode;

typedef enum : word {
  RL_A     = 0x17,
  RL_B     = 0x10,
  RL_C     = 0x11,
  RL_D     = 0x12,
  RL_E     = 0x13,
  RL_H     = 0x14,
  RL_L     = 0x15,
  RL_HL    = 0x16,
  RR_A     = 0x1F,
  RR_B     = 0x18,
  RR_C     = 0x19,
  RR_D     = 0x1A,
  RR_E     = 0x1B,
  RR_H     = 0x1C,
  RR_L     = 0x1D,
  RR_HL    = 0x1E,

  BIT_0_A  = 0x47,
  BIT_0_B  = 0x40,
  BIT_0_C  = 0x41,
  BIT_0_D  = 0x42,
  BIT_0_E  = 0x43,
  BIT_0_H  = 0x44,
  BIT_0_L  = 0x45,
  BIT_1_A  = 0x4F,
  BIT_1_B  = 0x48,
  BIT_1_C  = 0x49,
  BIT_1_D  = 0x4A,
  BIT_1_E  = 0x4B,
  BIT_1_H  = 0x4C,
  BIT_1_L  = 0x4D,
  BIT_2_A  = 0x57,
  BIT_2_B  = 0x50,
  BIT_2_C  = 0x51,
  BIT_2_D  = 0x52,
  BIT_2_E  = 0x53,
  BIT_2_H  = 0x54,
  BIT_2_L  = 0x55,
  BIT_3_A  = 0x5F,
  BIT_3_B  = 0x58,
  BIT_3_C  = 0x59,
  BIT_3_D  = 0x5A,
  BIT_3_E  = 0x5B,
  BIT_3_H  = 0x5C,
  BIT_3_L  = 0x5D,
  BIT_4_A  = 0x67,
  BIT_4_B  = 0x60,
  BIT_4_C  = 0x61,
  BIT_4_D  = 0x62,
  BIT_4_E  = 0x63,
  BIT_4_H  = 0x64,
  BIT_4_L  = 0x65,
  BIT_5_A  = 0x6F,
  BIT_5_B  = 0x68,
  BIT_5_C  = 0x69,
  BIT_5_D  = 0x6A,
  BIT_5_E  = 0x6B,
  BIT_5_H  = 0x6C,
  BIT_5_L  = 0x6D,
  BIT_6_A  = 0x77,
  BIT_6_B  = 0x70,
  BIT_6_C  = 0x71,
  BIT_6_D  = 0x72,
  BIT_6_E  = 0x73,
  BIT_6_H  = 0x74,
  BIT_6_L  = 0x75,
  BIT_7_A  = 0x7F,
  BIT_7_B  = 0x78,
  BIT_7_C  = 0x79,
  BIT_7_D  = 0x7A,
  BIT_7_E  = 0x7B,
  BIT_7_H  = 0x7C,
  BIT_7_L  = 0x7D,
  BIT_0_HL = 0x46,
  BIT_1_HL = 0x4E,
  BIT_2_HL = 0x56,
  BIT_3_HL = 0x5E,
  BIT_4_HL = 0x66,
  BIT_5_HL = 0x6E,
  BIT_6_HL = 0x76,
  BIT_7_HL = 0x7E,
} CBOpcode;

}

#endif  // define GAMEBOY_TYPES_HPP
