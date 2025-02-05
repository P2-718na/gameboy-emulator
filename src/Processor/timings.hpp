#ifndef TIMINGS_H
#define TIMINGS_H
#include <cassert>
#include <iostream>
#include <types.hpp>

namespace gb {

inline int Processor::getBusyCycles(Opcode opcode) {
  assert(opcode != CB);

  // TODO this is probably very inefficient. Better to use an array
  //  which is easily accessible
  switch (opcode) {

    case CALL_nn:
      return 6;

    case LD_inn_A:
    case RET:
    case PUSH_BC:
      return 4;

    case JR_e:
    case POP_BC:
    case INC_iHL:
    case DEC_iHL:
    case LD_DE_nn:
    case LD_HL_nn:
    case LD_iHL_n:
    case LD_SP_nn:
    case LDH_in_A:
    case LDH_A_in:
      return 3;

    case INC_BC:
    case INC_DE:
    case INC_HL:
    case INC_SP:
    case DEC_BC:
    case DEC_DE:
    case DEC_HL:
    case DEC_SP:
    case CP_n:
    case CP_iHL:
    case LD_A_n:
    case LD_B_n:
    case LD_C_n:
    case LD_D_n:
    case LD_E_n:
    case LD_H_n:
    case LD_L_n:
    case LD_A_iDE:
    case LD_iHL_A:
    case LD_iHLm_A:
    case LD_iHLp_A:
    case LDH_iC_A:
    case JR_NZ_e: // Timing varies if flag is set
    case JR_Z_e: // Timing varies if flag is set
      return 2;

    case INC_A:
    case INC_B:
    case INC_C:
    case INC_D:
    case INC_E:
    case INC_H:
    case INC_L:
    case DEC_A:
    case DEC_B:
    case DEC_C:
    case DEC_D:
    case DEC_E:
    case DEC_H:
    case DEC_L:
    case SUB_A:
    case SUB_B:
    case SUB_C:
    case SUB_D:
    case SUB_E:
    case SUB_H:
    case SUB_L:
    case RLA:
    case RLCA:
    case LD_A_A:
    case LD_A_B:
    case LD_A_C:
    case LD_A_D:
    case LD_A_E:
    case LD_A_H:
    case LD_A_L:
    case LD_C_A:
    case LD_D_A:
    case LD_D_B:
    case LD_D_C:
    case LD_D_D:
    case LD_D_E:
    case LD_D_H:
    case LD_D_L:
    case LD_H_A:
    case LD_H_B:
    case LD_H_C:
    case LD_H_D:
    case LD_H_E:
    case LD_H_H:
    case LD_H_L:
    case NOP:
    case XOR_A:
      return 1;

    default:
      std::printf("ERROR! Unknown Opcode timing: 0x%02X\n", opcode);
      return 1;
  }
}

inline int Processor::getBusyCyclesCB(CBOpcode opcode) {
  // TODO this is probably very inefficient. Better to use an array
  //   which is easily accessible
  switch (opcode) {
    case RL_A:
    case RL_B:
    case RL_C:
    case RL_D:
    case RL_E:
    case RL_H:
    case RL_L:
    case RR_A:
    case RR_B:
    case RR_C:
    case RR_D:
    case RR_E:
    case RR_H:
    case RR_L:
    case BIT_0_A:
    case BIT_0_B:
    case BIT_0_C:
    case BIT_0_D:
    case BIT_0_E:
    case BIT_0_H:
    case BIT_0_L:
    case BIT_1_A:
    case BIT_1_B:
    case BIT_1_C:
    case BIT_1_D:
    case BIT_1_E:
    case BIT_1_H:
    case BIT_1_L:
    case BIT_2_A:
    case BIT_2_B:
    case BIT_2_C:
    case BIT_2_D:
    case BIT_2_E:
    case BIT_2_H:
    case BIT_2_L:
    case BIT_3_A:
    case BIT_3_B:
    case BIT_3_C:
    case BIT_3_D:
    case BIT_3_E:
    case BIT_3_H:
    case BIT_3_L:
    case BIT_4_A:
    case BIT_4_B:
    case BIT_4_C:
    case BIT_4_D:
    case BIT_4_E:
    case BIT_4_H:
    case BIT_4_L:
    case BIT_5_A:
    case BIT_5_B:
    case BIT_5_C:
    case BIT_5_D:
    case BIT_5_E:
    case BIT_5_H:
    case BIT_5_L:
    case BIT_6_A:
    case BIT_6_B:
    case BIT_6_C:
    case BIT_6_D:
    case BIT_6_E:
    case BIT_6_H:
    case BIT_6_L:
    case BIT_7_A:
    case BIT_7_B:
    case BIT_7_C:
    case BIT_7_D:
    case BIT_7_E:
    case BIT_7_H:
    case BIT_7_L:
      return 2;

    case RL_HL:
    case RR_HL:
    case BIT_0_HL:
    case BIT_1_HL:
    case BIT_2_HL:
    case BIT_3_HL:
    case BIT_4_HL:
    case BIT_5_HL:
    case BIT_6_HL:
    case BIT_7_HL:
      return 4; // Todo here the tow different references specify different timings...
                // I believe the mistake is in the "complete reference".

    default:
      std::printf("ERROR! Unknown CBopcode timing: 0x%02X\n", opcode);
      return 1;
  }
}



}

#endif