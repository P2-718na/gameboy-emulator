#ifndef OPCODES_H
#define OPCODES_H
#include <iostream>
#include <cassert>

namespace gb {

inline int Processor::getBusyCycles(Opcode opcode) {
  assert(opcode != CB);

  switch (opcode) {

    case CALL_nn:
      return 6;

    case INC_HL:
    case DEC_HL:
    case LD_DE_nn:
    case LD_HL_nn:
    case LD_SP_nn:
    case LDH_n_A:
      return 3;

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
    case LD_A_n:
    case LD_C_n:
    case LD_A_DE:
    case LD_HL_A:
    case LD_HLm_A:
    case LDH_C_A:
    case JR_NZ_e: // Timing vaeies if flag is set
      return 2;

    case NOP:
    case XOR_A:
      return 1;

    default:
      std::printf("ERROR! Unknown Opcode timing: 0x%02X\n", opcode);
      return 1;
  }
}

inline int Processor::getBusyCyclesCB(CBOpcode opcode) {
  switch (opcode) {
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
    case BIT_0_HL:
    case BIT_1_HL:
    case BIT_2_HL:
    case BIT_3_HL:
    case BIT_4_HL:
    case BIT_5_HL:
    case BIT_6_HL:
    case BIT_7_HL:
      return 3; // Todo here the tow different references specify different timings...

    default:
      std::printf("ERROR! Unknown CBopcode timing: 0x%02X\n", opcode);
      return 1;
  }
}

inline void Processor::executeOpcode(Opcode opcode) {
  assert(opcode != CB);
  assert(busyCycles != 0);

  switch (opcode) {
    case JR_NZ_e: {
      const signed char e = popPC();
      if (!F[FZ]) {
        PC += e;
        ++busyCycles;
      }
      break;
    }

    case LD_A_n:
      A = popPC();
    break;

    case LD_C_n:
      C = popPC();
    break;

    case LD_A_DE:
      A = ram_->read(DE());
      break;

    case LD_HL_A:
      ram_->write(HL(), A);
      break;

    case LD_HL_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      H = msb;
      L = lsb;
      break;
    }

    case LD_DE_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      DE(msb, lsb);
      break;
    }

    case LD_SP_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      setSP(msb, lsb);
      break;
    }

    case LD_HLm_A: {
      ram_->write(HL(), A);
      HL(HL()-1);
      break;
    }

    case LDH_C_A:
      ram_->write(twoWordToDword(0xFF, C), A);
      break;

    case LDH_n_A: {
      const auto n = popPC();
      ram_->write(twoWordToDword(0xff, n), A);
      break;
    }

    case INC_C: {
      // TODO this definition is sus check that this is correct
      bool bit3 = nthBit(C, 3);
      C += 1;
      bool carry3 = (bit3 == 1 && nthBit(C, 3) == 0);
      F[FZ] = C == 0;
      F[FN] = false;
      F[FH] = carry3; // this is set if there is a carry per bit 3 (??)
      break;
    }

    case XOR_A:
      A ^= B;
      F[FZ] = !A;
      F[FN] = false;
      F[FH] = false;
      F[FC] = false;
      break;

    case NOP:
      break;

    case CALL_nn: {
      auto lsb = popPC();
      auto msb = popPC();
      auto nn = twoWordToDword(msb, lsb);
      --SP;
      ram_->write(SP--, dwordMsb(PC));
      ram_->write(SP, dwordLsb(PC));
      PC = nn;
      break;
    }

    default:
      std::printf("ERROR! Unknown Opcode: 0x%02X\n", opcode);
      break;
  }
}

inline void Processor::executeCBOpcode(CBOpcode opcode) {
  assert(busyCycles != 0);

  switch (opcode) {
    case BIT_7_H: {
      bool bit = nthBit(H, 7);
      F[FZ] = bit == 0;
      F[FN] = 0;
      F[FH] = 1;
      break;
    }

    default:
      std::printf("ERROR! Unknown CBopcode: 0x%02X\n", opcode);
    break;
  }
}

}

#endif