#ifndef TIMINGS_H
#define TIMINGS_H
#include <cassert>
#include <array>
#include <limits>
#include <iostream>
#include <types.hpp>

namespace gb {

// Returns number of MACHINE CYCLES an operation needs to be executed
// Gotta do this because array designators were removed after C++99
inline void CPU::initTimings() {
  auto& _ = timings_;

  // Undefined instructions ////////////////////////
  _[UNDEFINED_00] = std::numeric_limits<int>::max();
  _[UNDEFINED_01] = std::numeric_limits<int>::max();
  _[UNDEFINED_02] = std::numeric_limits<int>::max();
  _[UNDEFINED_03] = std::numeric_limits<int>::max();
  _[UNDEFINED_04] = std::numeric_limits<int>::max();
  _[UNDEFINED_05] = std::numeric_limits<int>::max();
  _[UNDEFINED_06] = std::numeric_limits<int>::max();
  _[UNDEFINED_07] = std::numeric_limits<int>::max();
  _[UNDEFINED_08] = std::numeric_limits<int>::max();
  _[UNDEFINED_09] = std::numeric_limits<int>::max();
  _[UNDEFINED_10] = std::numeric_limits<int>::max();
  /////////////////////////////////////////////////


  // "Miscellaneous" //////////////////////////////////////
  _[NOP]  = 1;
  _[STOP] = 1;
  _[DI]   = 1;
  _[HALT] = 1;
  _[EI]   = 1;
  ////////////////////////////////////////////////////////

  // Bit operations /////////////////////////////////////
  _[RLCA] = 1;
  _[RLA]  = 1;
  _[RRCA] = 1;
  _[RRA]  = 1;
  ////////////////////////////////////////////////////////

  // 16-bit arithmetic ///////////////////////////////////
  _[INC_BC]     = 2;
  _[INC_DE]     = 2;
  _[INC_HL]     = 2;
  _[INC_SP]     = 2;
  _[DEC_BC]     = 2;
  _[DEC_DE]     = 2;
  _[DEC_HL]     = 2;
  _[DEC_SP]     = 2;
  _[ADD_HL_BC]  = 2;
  _[ADD_HL_DE]  = 2;
  _[ADD_HL_HL]  = 2;
  _[ADD_HL_SP]  = 2;
  _[ADD_SP_e]   = 4;
  /////////////////////////////////////////////////////////

  // Control flow /////////////////////////////////////////
  _[JR_e]       = 3;
  _[JR_Z_e]     = 3;
  _[JR_C_e]     = 3;
  _[JR_NZ_e]    = 3;
  _[JR_NC_e]    = 3;
  _[JP_nn]      = 4;
  _[JP_NZ_nn]   = 4;
  _[JP_NC_nn]   = 4;
  _[CALL_NZ_nn] = 6;
  _[CALL_NC_nn] = 6;
  _[RST_0x00]   = 4;
  _[RST_0x10]   = 4;
  _[RST_0x20]   = 4;
  _[RST_0x30]   = 4;
  _[RET_Z]      = 5;
  _[RET_C]      = 5;
  _[RET_NZ]     = 5;
  _[RET_NC]     = 5;
  _[RET]        = 4;
  _[RETI]       = 4;
  _[JP_HL]      = 1;
  _[JP_Z_nn]    = 4;
  _[JP_C_nn]    = 4;
  _[CALL_Z_nn]  = 6;
  _[CALL_C_nn]  = 6;
  _[CALL_nn]    = 6;
  _[RST_0x08]   = 4;
  _[RST_0x18]   = 4;
  _[RST_0x28]   = 4;
  _[RST_0x38]   = 4;
  /////////////////////////////////////////////////

  // 16 bit loads /////////////////////////////////
  _[LD_inn_SP]  = 5;
  _[LD_HL_SPe] = 3;
  _[LD_SP_HL]   = 2;
  _[LD_BC_nn]   = 3;
  _[LD_DE_nn]   = 3;
  _[LD_HL_nn]   = 3;
  _[LD_SP_nn]   = 3;
  _[POP_BC]     = 3;
  _[POP_DE]     = 3;
  _[POP_HL]     = 3;
  _[POP_AF]     = 3;
  _[PUSH_BC]    = 4;
  _[PUSH_DE]    = 4;
  _[PUSH_HL]    = 4;
  _[PUSH_AF]    = 4;
  //////////////////////////////////////////////////

  // 8 bit loads ///////////////////////////////////
  _[LD_A_n]     = 2;
  _[LD_B_n]     = 2;
  _[LD_C_n]     = 2;
  _[LD_D_n]     = 2;
  _[LD_E_n]     = 2;
  _[LD_H_n]     = 2;
  _[LD_L_n]     = 2;
  _[LD_iHL_n]   = 3;
  _[LD_A_A]     = 1;
  _[LD_A_B]     = 1;
  _[LD_A_C]     = 1;
  _[LD_A_D]     = 1;
  _[LD_A_E]     = 1;
  _[LD_A_H]     = 1;
  _[LD_A_L]     = 1;
  _[LD_B_A]     = 1;
  _[LD_B_B]     = 1;
  _[LD_B_C]     = 1;
  _[LD_B_D]     = 1;
  _[LD_B_E]     = 1;
  _[LD_B_H]     = 1;
  _[LD_B_L]     = 1;
  _[LD_C_A]     = 1;
  _[LD_C_B]     = 1;
  _[LD_C_C]     = 1;
  _[LD_C_D]     = 1;
  _[LD_C_E]     = 1;
  _[LD_C_H]     = 1;
  _[LD_C_L]     = 1;
  _[LD_D_A]     = 1;
  _[LD_D_B]     = 1;
  _[LD_D_C]     = 1;
  _[LD_D_D]     = 1;
  _[LD_D_E]     = 1;
  _[LD_D_H]     = 1;
  _[LD_D_L]     = 1;
  _[LD_E_A]     = 1;
  _[LD_E_B]     = 1;
  _[LD_E_C]     = 1;
  _[LD_E_D]     = 1;
  _[LD_E_E]     = 1;
  _[LD_E_H]     = 1;
  _[LD_E_L]     = 1;
  _[LD_H_A]     = 1;
  _[LD_H_B]     = 1;
  _[LD_H_C]     = 1;
  _[LD_H_D]     = 1;
  _[LD_H_E]     = 1;
  _[LD_H_H]     = 1;
  _[LD_H_L]     = 1;
  _[LD_L_A]     = 1;
  _[LD_L_B]     = 1;
  _[LD_L_C]     = 1;
  _[LD_L_D]     = 1;
  _[LD_L_E]     = 1;
  _[LD_L_H]     = 1;
  _[LD_L_L]     = 1;
  _[LD_iHL_A]   = 2;
  _[LD_iHL_B]   = 2;
  _[LD_iHL_C]   = 2;
  _[LD_iHL_D]   = 2;
  _[LD_iHL_E]   = 2;
  _[LD_iHL_H]   = 2;
  _[LD_iHL_L]   = 2;
  _[LD_iBC_A]   = 2;
  _[LD_iDE_A]   = 2;
  _[LD_iHLp_A]  = 2;
  _[LD_iHLm_A]  = 2;
  _[LD_A_iBC]   = 2;
  _[LD_A_iDE]   = 2;
  _[LD_A_iHLp]  = 2;
  _[LD_A_iHLm]  = 2;
  _[LDH_in_A]   = 2;
  _[LDH_A_in]   = 2;
  _[LDH_iC_A]   = 2;
  _[LDH_A_iC]   = 2;
  _[LD_inn_A]   = 4;
  _[LD_A_inn]   = 4;
  _[LD_A_iHL]   = 2;
  _[LD_B_iHL]   = 2;
  _[LD_C_iHL]   = 2;
  _[LD_D_iHL]   = 2;
  _[LD_E_iHL]   = 2;
  _[LD_H_iHL]   = 2;
  _[LD_L_iHL]   = 2;
  //////////////////////////////////////////////////


  // 8 bit arithmetic and logic ////////////////////
  _[INC_A]      = 1;
  _[INC_B]      = 1;
  _[INC_C]      = 1;
  _[INC_D]      = 1;
  _[INC_E]      = 1;
  _[INC_H]      = 1;
  _[INC_L]      = 1;
  _[DEC_A]      = 1;
  _[DEC_B]      = 1;
  _[DEC_C]      = 1;
  _[DEC_D]      = 1;
  _[DEC_E]      = 1;
  _[DEC_H]      = 1;
  _[DEC_L]      = 1;
  _[INC_iHL]    = 3;
  _[DEC_iHL]    = 3;
  _[ADD_A]      = 1;
  _[ADD_B]      = 1;
  _[ADD_C]      = 1;
  _[ADD_D]      = 1;
  _[ADD_E]      = 1;
  _[ADD_H]      = 1;
  _[ADD_L]      = 1;
  _[SUB_A]      = 1;
  _[SUB_B]      = 1;
  _[SUB_C]      = 1;
  _[SUB_D]      = 1;
  _[SUB_E]      = 1;
  _[SUB_H]      = 1;
  _[SUB_L]      = 1;
  _[AND_A]      = 1;
  _[AND_B]      = 1;
  _[AND_C]      = 1;
  _[AND_D]      = 1;
  _[AND_E]      = 1;
  _[AND_H]      = 1;
  _[AND_L]      = 1;
  _[OR_A]       = 1;
  _[OR_B]       = 1;
  _[OR_C]       = 1;
  _[OR_D]       = 1;
  _[OR_E]       = 1;
  _[OR_H]       = 1;
  _[OR_L]       = 1;
  _[ADC_A]      = 1;
  _[ADC_B]      = 1;
  _[ADC_C]      = 1;
  _[ADC_D]      = 1;
  _[ADC_E]      = 1;
  _[ADC_H]      = 1;
  _[ADC_L]      = 1;
  _[SBC_A]      = 1;
  _[SBC_B]      = 1;
  _[SBC_C]      = 1;
  _[SBC_D]      = 1;
  _[SBC_E]      = 1;
  _[SBC_H]      = 1;
  _[SBC_L]      = 1;
  _[XOR_A]      = 1;
  _[XOR_B]      = 1;
  _[XOR_C]      = 1;
  _[XOR_D]      = 1;
  _[XOR_E]      = 1;
  _[XOR_H]      = 1;
  _[XOR_L]      = 1;
  _[CP_A]       = 1;
  _[CP_B]       = 1;
  _[CP_C]       = 1;
  _[CP_D]       = 1;
  _[CP_E]       = 1;
  _[CP_H]       = 1;
  _[CP_L]       = 1;
  _[ADD_iHL]    = 2;
  _[SUB_iHL]    = 2;
  _[AND_iHL]    = 2;
  _[OR_iHL]     = 2;
  _[ADC_iHL]    = 2;
  _[SBC_iHL]    = 2;
  _[XOR_iHL]    = 2;
  _[CP_iHL]     = 2;
  _[ADD_n]      = 2;
  _[SUB_n]      = 2;
  _[AND_n]      = 2;
  _[OR_n]       = 2;
  _[ADC_n]      = 2;
  _[SBC_n]      = 2;
  _[XOR_n]      = 2;
  _[CP_n]       = 2;
  _[DAA]        = 1;
  _[SCF]        = 1;
  _[CPL]        = 1;
  _[CCF]        = 1;
  /////////////////////////////////////
}

void CPU::initTimingsCB() {
  auto& _ = timingsCB_;

  // Rotations ////////////////////////////
  _[RLC_A]     = 2;
  _[RLC_B]     = 2;
  _[RLC_C]     = 2;
  _[RLC_D]     = 2;
  _[RLC_E]     = 2;
  _[RLC_H]     = 2;
  _[RLC_L]     = 2;
  _[RRC_A]     = 2;
  _[RRC_B]     = 2;
  _[RRC_C]     = 2;
  _[RRC_D]     = 2;
  _[RRC_E]     = 2;
  _[RRC_H]     = 2;
  _[RRC_L]     = 2;
  _[RLC_iHL]    = 4;
  _[RRC_iHL]    = 4;
  _[RL_A]      = 2;
  _[RL_B]      = 2;
  _[RL_C]      = 2;
  _[RL_D]      = 2;
  _[RL_E]      = 2;
  _[RL_H]      = 2;
  _[RL_L]      = 2;
  _[RR_A]      = 2;
  _[RR_B]      = 2;
  _[RR_C]      = 2;
  _[RR_D]      = 2;
  _[RR_E]      = 2;
  _[RR_H]      = 2;
  _[RR_L]      = 2;
  _[RL_iHL]     = 4;
  _[RR_iHL]     = 4;
  //////////////////////////////////////////////////

  // Shifts /////////////////////////////////////////
  _[SLA_A]      = 2;
  _[SLA_B]      = 2;
  _[SLA_C]      = 2;
  _[SLA_D]      = 2;
  _[SLA_E]      = 2;
  _[SLA_H]      = 2;
  _[SLA_L]      = 2;
  _[SRA_A]      = 2;
  _[SRA_B]      = 2;
  _[SRA_C]      = 2;
  _[SRA_D]      = 2;
  _[SRA_E]      = 2;
  _[SRA_H]      = 2;
  _[SRA_L]      = 2;
  _[SLA_iHL]     = 4;
  _[SRA_iHL]     = 4;
  _[SRL_A]      = 2;
  _[SRL_B]      = 2;
  _[SRL_C]      = 2;
  _[SRL_D]      = 2;
  _[SRL_E]      = 2;
  _[SRL_H]      = 2;
  _[SRL_L]      = 2;
  _[SRL_iHL]     = 4;
  /////////////////////////////////////////////

  // Swap ///////////////////////////////////
  _[SWAP_A]      = 2;
  _[SWAP_B]      = 2;
  _[SWAP_C]      = 2;
  _[SWAP_D]      = 2;
  _[SWAP_E]      = 2;
  _[SWAP_H]      = 2;
  _[SWAP_L]      = 2;
  _[SWAP_iHL]     = 4;
  //////////////////////////////////////////


  // BIT ///////////////////////////////////////
  _[BIT_0_A]  = 2;
  _[BIT_0_B]  = 2;
  _[BIT_0_C]  = 2;
  _[BIT_0_D]  = 2;
  _[BIT_0_E]  = 2;
  _[BIT_0_H]  = 2;
  _[BIT_0_L]  = 2;
  _[BIT_1_A]  = 2;
  _[BIT_1_B]  = 2;
  _[BIT_1_C]  = 2;
  _[BIT_1_D]  = 2;
  _[BIT_1_E]  = 2;
  _[BIT_1_H]  = 2;
  _[BIT_1_L]  = 2;
  _[BIT_2_A]  = 2;
  _[BIT_2_B]  = 2;
  _[BIT_2_C]  = 2;
  _[BIT_2_D]  = 2;
  _[BIT_2_E]  = 2;
  _[BIT_2_H]  = 2;
  _[BIT_2_L]  = 2;
  _[BIT_3_A]  = 2;
  _[BIT_3_B]  = 2;
  _[BIT_3_C]  = 2;
  _[BIT_3_D]  = 2;
  _[BIT_3_E]  = 2;
  _[BIT_3_H]  = 2;
  _[BIT_3_L]  = 2;
  _[BIT_4_A]  = 2;
  _[BIT_4_B]  = 2;
  _[BIT_4_C]  = 2;
  _[BIT_4_D]  = 2;
  _[BIT_4_E]  = 2;
  _[BIT_4_H]  = 2;
  _[BIT_4_L]  = 2;
  _[BIT_5_A]  = 2;
  _[BIT_5_B]  = 2;
  _[BIT_5_C]  = 2;
  _[BIT_5_D]  = 2;
  _[BIT_5_E]  = 2;
  _[BIT_5_H]  = 2;
  _[BIT_5_L]  = 2;
  _[BIT_6_A]  = 2;
  _[BIT_6_B]  = 2;
  _[BIT_6_C]  = 2;
  _[BIT_6_D]  = 2;
  _[BIT_6_E]  = 2;
  _[BIT_6_H]  = 2;
  _[BIT_6_L]  = 2;
  _[BIT_7_A]  = 2;
  _[BIT_7_B]  = 2;
  _[BIT_7_C]  = 2;
  _[BIT_7_D]  = 2;
  _[BIT_7_E]  = 2;
  _[BIT_7_H]  = 2;
  _[BIT_7_L]  = 2;
  _[BIT_0_iHL] = 3;
  _[BIT_1_iHL] = 3;
  _[BIT_2_iHL] = 3;
  _[BIT_3_iHL] = 3;
  _[BIT_4_iHL] = 3;
  _[BIT_5_iHL] = 3;
  _[BIT_6_iHL] = 3;
  _[BIT_7_iHL] = 3;
  /////////////////////////////////////////////

  // Reset bit //////////////////////////////////
  _[RES_0_A]  = 2;
  _[RES_0_B]  = 2;
  _[RES_0_C]  = 2;
  _[RES_0_D]  = 2;
  _[RES_0_E]  = 2;
  _[RES_0_H]  = 2;
  _[RES_0_L]  = 2;
  _[RES_1_A]  = 2;
  _[RES_1_B]  = 2;
  _[RES_1_C]  = 2;
  _[RES_1_D]  = 2;
  _[RES_1_E]  = 2;
  _[RES_1_H]  = 2;
  _[RES_1_L]  = 2;
  _[RES_2_A]  = 2;
  _[RES_2_B]  = 2;
  _[RES_2_C]  = 2;
  _[RES_2_D]  = 2;
  _[RES_2_E]  = 2;
  _[RES_2_H]  = 2;
  _[RES_2_L]  = 2;
  _[RES_3_A]  = 2;
  _[RES_3_B]  = 2;
  _[RES_3_C]  = 2;
  _[RES_3_D]  = 2;
  _[RES_3_E]  = 2;
  _[RES_3_H]  = 2;
  _[RES_3_L]  = 2;
  _[RES_4_A]  = 2;
  _[RES_4_B]  = 2;
  _[RES_4_C]  = 2;
  _[RES_4_D]  = 2;
  _[RES_4_E]  = 2;
  _[RES_4_H]  = 2;
  _[RES_4_L]  = 2;
  _[RES_5_A]  = 2;
  _[RES_5_B]  = 2;
  _[RES_5_C]  = 2;
  _[RES_5_D]  = 2;
  _[RES_5_E]  = 2;
  _[RES_5_H]  = 2;
  _[RES_5_L]  = 2;
  _[RES_6_A]  = 2;
  _[RES_6_B]  = 2;
  _[RES_6_C]  = 2;
  _[RES_6_D]  = 2;
  _[RES_6_E]  = 2;
  _[RES_6_H]  = 2;
  _[RES_6_L]  = 2;
  _[RES_7_A]  = 2;
  _[RES_7_B]  = 2;
  _[RES_7_C]  = 2;
  _[RES_7_D]  = 2;
  _[RES_7_E]  = 2;
  _[RES_7_H]  = 2;
  _[RES_7_L]  = 2;
  _[RES_0_iHL] = 4;
  _[RES_1_iHL] = 4;
  _[RES_2_iHL] = 4;
  _[RES_3_iHL] = 4;
  _[RES_4_iHL] = 4;
  _[RES_5_iHL] = 4;
  _[RES_6_iHL] = 4;
  _[RES_7_iHL] = 4;
  /////////////////////////////////////

  // Set bit //////////////////////////////////
  _[SET_0_A]  = 2;
  _[SET_0_B]  = 2;
  _[SET_0_C]  = 2;
  _[SET_0_D]  = 2;
  _[SET_0_E]  = 2;
  _[SET_0_H]  = 2;
  _[SET_0_L]  = 2;
  _[SET_1_A]  = 2;
  _[SET_1_B]  = 2;
  _[SET_1_C]  = 2;
  _[SET_1_D]  = 2;
  _[SET_1_E]  = 2;
  _[SET_1_H]  = 2;
  _[SET_1_L]  = 2;
  _[SET_2_A]  = 2;
  _[SET_2_B]  = 2;
  _[SET_2_C]  = 2;
  _[SET_2_D]  = 2;
  _[SET_2_E]  = 2;
  _[SET_2_H]  = 2;
  _[SET_2_L]  = 2;
  _[SET_3_A]  = 2;
  _[SET_3_B]  = 2;
  _[SET_3_C]  = 2;
  _[SET_3_D]  = 2;
  _[SET_3_E]  = 2;
  _[SET_3_H]  = 2;
  _[SET_3_L]  = 2;
  _[SET_4_A]  = 2;
  _[SET_4_B]  = 2;
  _[SET_4_C]  = 2;
  _[SET_4_D]  = 2;
  _[SET_4_E]  = 2;
  _[SET_4_H]  = 2;
  _[SET_4_L]  = 2;
  _[SET_5_A]  = 2;
  _[SET_5_B]  = 2;
  _[SET_5_C]  = 2;
  _[SET_5_D]  = 2;
  _[SET_5_E]  = 2;
  _[SET_5_H]  = 2;
  _[SET_5_L]  = 2;
  _[SET_6_A]  = 2;
  _[SET_6_B]  = 2;
  _[SET_6_C]  = 2;
  _[SET_6_D]  = 2;
  _[SET_6_E]  = 2;
  _[SET_6_H]  = 2;
  _[SET_6_L]  = 2;
  _[SET_7_A]  = 2;
  _[SET_7_B]  = 2;
  _[SET_7_C]  = 2;
  _[SET_7_D]  = 2;
  _[SET_7_E]  = 2;
  _[SET_7_H]  = 2;
  _[SET_7_L]  = 2;
  _[SET_0_iHL] = 4;
  _[SET_1_iHL] = 4;
  _[SET_2_iHL] = 4;
  _[SET_3_iHL] = 4;
  _[SET_4_iHL] = 4;
  _[SET_5_iHL] = 4;
  _[SET_6_iHL] = 4;
  _[SET_7_iHL] = 4;
  /////////////////////////////////////
}

inline int CPU::getBusyCyclesCB(gb::CBOpcode opcode) {
  const int busyCycles = timingsCB_[opcode];
  assert(busyCycles != 0);
  return busyCycles;
}

inline int CPU::getBusyCycles(Opcode opcode) {
  const int busyCycles = timings_[opcode];
  assert(busyCycles != 0);
  return busyCycles;
}


}

#endif