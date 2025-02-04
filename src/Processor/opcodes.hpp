#ifndef OPCODES_H
#define OPCODES_H
#include <iostream>
#include <cassert>

namespace gb {
inline void Processor::executeOpcode(Opcode opcode) {
  assert(opcode != CB);
  assert(busyCycles != 0);

  switch (opcode) {
    case JR_e: {
      const signed char e = popPC();
      PC += e;
      break;
    }

    case JR_NZ_e: {
      const signed char e = popPC();
      if (!F[FZ]) {
        PC += e;
        ++busyCycles;
      }
      break;
    }

    case JR_Z_e: {
      const signed char e = popPC();
      if (F[FZ]) {
        PC += e;
        ++busyCycles;
      }
      break;
    }

    case LD_A_n:
      A = popPC();
      break;

    case LD_A_E:
      A = E;
      break;

    case LD_B_n:
      B = popPC();
     break;

    case LD_C_n:
      C = popPC();
     break;

    case LD_D_n:
      D = popPC();
     break;

    case LD_E_n:
      E = popPC();
     break;

    case LD_H_n:
      H = popPC();
     break;

    case LD_L_n:
      L = popPC();
     break;

    case LD_C_A:
      C = A;
      break;

    case LD_D_A:
      D = A;
      break;

    case LD_H_A:
      H = A;
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

    case LD_nn_A: {
      auto lsb = popPC();
      auto msb = popPC();
      ram_->write(twoWordToDword(msb, lsb), A);
      break;
    }

    case LD_SP_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      setSP(msb, lsb);
      break;
    }

    case LD_HLp_A: {
      ram_->write(HL(), A);
      HL(HL()+1);
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

    case LDH_A_n: {
      const word n = popPC();
      A = ram_->read(twoWordToDword(0xFF, n));
      break;
    }

    case LDH_n_A: {
      const word n = popPC();
      ram_->write(twoWordToDword(0xff, n), A);
      break;
    }

    case INC_B: {
      // TODO this definition is sus but it appears correct.
      bool bit3 = nthBit(B, 3);
      B += 1;
      bool carry3 = (bit3 == 1 && nthBit(B, 3) == 0); // Todo add tests
      F[FZ] = B == 0;
      F[FN] = false;
      F[FH] = carry3;
      break;
    }

    case INC_C: {
      // TODO this definition is sus but it appears correct.
      bool bit3 = nthBit(C, 3);
      C += 1;
      bool carry3 = (bit3 == 1 && nthBit(C, 3) == 0); // Todo add tests
      F[FZ] = C == 0;
      F[FN] = false;
      F[FH] = carry3; // see this is set if there is a carry per bit 3 (??)
      break;
    }

    case DEC_A: {
      decrementRegister(A);
      break;
    }

    case DEC_B: {
      decrementRegister(B);
      break;
    }
    case DEC_C: {
      decrementRegister(C);
      break;
    }

    case INC_DE:
      DE(DE() + 1);
      break;

    case INC_HL:
      HL(HL() + 1);
      break;

    case CP_n: {
      const word n = popPC();
      const word result = A - n;
      F[FZ] = result == 0;
      F[FN] = true;
      F[FH] = nthBit(A, 3) == 0 && nthBit(n, 3) == 1; // Todo test this
      F[FC] = nthBit(A, 7) == 0 && nthBit(n, 7) == 1; // Todo test this
      // For this it says that it checks the borrow bit and not carry
      /// 10000 -
      /// 01000 =
      /// -------
      ///  1000
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

    case RLA: {
      const bool carry = nthBit(C, 7);;
      A <<= 1;
      A |= F[FC];
      F[FC] = carry;
      F[FZ] = A == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    case RLCA:
      F[FC] = nthBit(A, 7);
      A <<= 1;
      A |= F[FC];
      F[FH] = false;
      F[FN] = false;
      break;

    case RET: {
      auto lsb = ram_->read(SP++);
      auto msb = ram_->read(SP++);
      PC = twoWordToDword(msb, lsb);
      break;
    }

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

    case PUSH_BC:
      --SP;
      ram_->write(SP--, B);
      ram_->write(SP, C);
      break;

    case POP_BC: {
      auto lsb = ram_->read(SP++);
      auto msb = ram_->read(SP++);
      BC(msb, lsb);
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
    case RL_C: {
      const bool carry = nthBit(C, 7);;
      C <<= 1;
      C |= F[FC];
      F[FC] = carry;
      F[FZ] = C == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

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