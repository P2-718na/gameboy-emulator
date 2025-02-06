#ifndef OPCODES_H
#define OPCODES_H

#include <iostream>
#include <cassert>

namespace gb {
inline void Processor::executeOpcode(Opcode opcode) {
  assert(opcode != CB);
  assert(busyCycles != 0);

  switch (opcode) {
    case UNDEFINED_00:
    case UNDEFINED_01:
    case UNDEFINED_02:
    case UNDEFINED_03:
    case UNDEFINED_04:
    case UNDEFINED_05:
    case UNDEFINED_06:
    case UNDEFINED_07:
    case UNDEFINED_08:
    case UNDEFINED_09:
    case UNDEFINED_10:
      crash();
      break;


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

    case LD_A_H:
      A = H;
      break;

    case LD_A_L:
      A = L;
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

    case LD_A_iDE:
      A = ram_->read(DE());
      break;

    case LD_iHL_A:
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

    case LD_inn_A: {
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

    case LD_iHLp_A: {
      ram_->write(HL(), A);
      HL(HL()+1);
      break;
    }

    case LD_iHLm_A: {
      ram_->write(HL(), A);
      HL(HL()-1);
      break;
    }

    case LDH_iC_A:
      ram_->write(twoWordToDword(0xFF, C), A);
      break;

    case LDH_A_in: {
      const word n = popPC();
      A = ram_->read(twoWordToDword(0xFF, n));
      break;
    }

    case LDH_in_A: {
      const word n = popPC();
      ram_->write(twoWordToDword(0xff, n), A);
      break;
    }

    case INC_A:
      incrementRegister(A);
      break;

    case INC_B:
      incrementRegister(B);
      break;

    case INC_C:
      incrementRegister(C);
      break;

    case INC_D:
      incrementRegister(D);
      break;

    case INC_E:
      incrementRegister(E);
      break;

    case INC_H:
      incrementRegister(H);
      break;

    case INC_L:
      incrementRegister(L);
      break;

    case DEC_A:
      decrementRegister(A);
      break;

    case DEC_B:
      decrementRegister(B);
      break;

    case DEC_C:
      decrementRegister(C);
      break;

    case DEC_D:
        decrementRegister(D);
        break;

    case DEC_E:
        decrementRegister(E);
        break;

    case DEC_H:
        decrementRegister(H);
        break;

    case DEC_L:
        decrementRegister(L);
        break;

    case SUB_A:
      subRegister(A);
      break;

    case SUB_B:
      subRegister(B);
      break;

    case SUB_C:
      subRegister(C);
      break;

    case SUB_D:
      subRegister(D);
      break;

    case SUB_E:
      subRegister(E);
      break;

    case SUB_H:
      subRegister(H);
      break;

    case SUB_L:
      subRegister(L);
      break;

    case INC_DE:
      DE(DE() + 1);
      break;

    case INC_HL:
      HL(HL() + 1);
      break;

    case CP_n: {
      const word n = popPC();
      cmpRegister(n);
      break;
    }

    case CP_iHL: { //todo
      const word n = ram_->read(HL());
      cmpRegister(n);
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
      const bool carry = nthBit(A, 7);;
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
      std::printf("\033[1;31mERROR! Unknown Opcode: 0x%02X\n\033[0m", opcode);
      break;
  }
}

inline void Processor::executeCBOpcode(CBOpcode opcode) {
  assert(busyCycles != 0);

  switch (opcode) {
#define CASE_RLC(X)                               \
    case RLC_ ## X: {                              \
      const bool carry = nthBit(X, 7);            \
      X <<= 1;                                    \
      X |= carry;                                 \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_RLC(A);
    CASE_RLC(B);
    CASE_RLC(C);
    CASE_RLC(D);
    CASE_RLC(E);
    CASE_RLC(H);
    CASE_RLC(L);
#undef CASE_RLC

#define CASE_RRC(X)                               \
    case RRC_ ## X: {                             \
      const bool carry = nthBit(X, 0);            \
      X >>= 1;                                    \
      X |= (carry<<7);                            \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_RRC(A);
    CASE_RRC(B);
    CASE_RRC(C);
    CASE_RRC(D);
    CASE_RRC(E);
    CASE_RRC(H);
    CASE_RRC(L);
#undef CASE_RRC

    case RLC_iHL: {
      const bool carry = nthBit(iHL(), 7);
      const word result = (iHL() << 1) | (carry);
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    case RRC_iHL: {
      const bool carry = nthBit(iHL(), 0);
      const word result = (iHL() >> 1) | (carry << 7);
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

#define CASE_RL(X)                                \
    case RL_ ## X: {                              \
      const bool carry = nthBit(X, 7);            \
      X <<= 1;                                    \
      X |= F[FC];                                 \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_RL(A);
    CASE_RL(B);
    CASE_RL(C);
    CASE_RL(D);
    CASE_RL(E);
    CASE_RL(H);
    CASE_RL(L);
#undef CASE_RL

#define CASE_RR(X)                                \
    case RR_ ## X: {                              \
      const bool carry = nthBit(X, 0);            \
      X >>= 1;                                    \
      X |= (F[FC] << 7);                          \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_RR(A);
    CASE_RR(B);
    CASE_RR(C);
    CASE_RR(D);
    CASE_RR(E);
    CASE_RR(H);
    CASE_RR(L);
#undef CASE_RR

    case RL_iHL: {
      const bool carry = nthBit(iHL(), 7);
      const word result = (iHL() << 1) | F[FC];
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    case RR_iHL: {
      const bool carry = nthBit(iHL(), 0);
      const word result = (iHL() >> 1) | (F[FC] << 7);
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

#define CASE_SLA(X)                               \
    case SLA_ ## X: {                             \
      const bool carry = nthBit(X, 7);            \
      X <<= 1;                                    \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_SLA(A);
    CASE_SLA(B);
    CASE_SLA(C);
    CASE_SLA(D);
    CASE_SLA(E);
    CASE_SLA(H);
    CASE_SLA(L);
#undef CASE_SLA

      // Arithmetic right shift effectively performs a division by 2.
      // (This works for two's complement numbers).
#define CASE_SRA(X)                               \
    case SRA_ ## X: {                             \
      const bool carry = nthBit(X, 0);            \
      const bool sign = nthBit(X, 7);             \
      X >>= 1;                                    \
      X |= (sign << 7);                           \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_SRA(A);
    CASE_SRA(B);
    CASE_SRA(C);
    CASE_SRA(D);
    CASE_SRA(E);
    CASE_SRA(H);
    CASE_SRA(L);
#undef CASE_SRA

    case SLA_iHL: {
      const bool carry = nthBit(iHL(), 7);
      const word result = iHL() << 1;
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }
    case SRA_iHL: {
      const bool carry = nthBit(iHL(), 0);
      const bool sign = nthBit(iHL(), 7);
      const word result = (iHL() >> 1) | (sign << 7);
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

#define CASE_SRL(X)                               \
    case SRL_ ## X: {                             \
      const bool carry = nthBit(X, 0);            \
      X >>= 1;                                    \
      F[FC] = carry;                              \
      F[FZ] = X == 0;                             \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_SRL(A);
    CASE_SRL(B);
    CASE_SRL(C);
    CASE_SRL(D);
    CASE_SRL(E);
    CASE_SRL(H);
    CASE_SRL(L);
#undef CASE_SRL

    case SRL_iHL: {
      const bool carry = nthBit(iHL(), 0);
      const word result = iHL() >> 1;
      iHL(result);
      F[FC] = carry;
      F[FZ] = result == 0;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    // todo swap is not well documented; check for tests/examples
    //  A nibble is an aggregation of 4 bits.
#define CASE_SWAP(X)                              \
    case SWAP_ ## X: {                            \
      const word copy = X << 4;                   \
      X >>= 4;                                    \
      X |= copy;                                  \
      F[FZ] = X == 0;                             \
      F[FC] = false;                              \
      F[FN] = false;                              \
      F[FH] = false;                              \
      break;                                      \
    }

    CASE_SWAP(A);
    CASE_SWAP(B);
    CASE_SWAP(C);
    CASE_SWAP(D);
    CASE_SWAP(E);
    CASE_SWAP(H);
    CASE_SWAP(L);
#undef CASE_SWAP

    case SWAP_iHL: {
      word msn = iHL() << 4; // most significant nibble
      word lsn = iHL() >> 4; // least significant nibble
      iHL(msn | lsn);
      F[FZ] = iHL() == 0;
      F[FC] = false;
      F[FN] = false;
      F[FH] = false;
      break;
   }

#define CASE_BIT(N, X)                            \
    case BIT_ ## N ## _ ## X: {                   \
      bool bit = nthBit(X, N);                    \
      F[FZ] = bit == 0;                           \
      F[FN] = false;                              \
      F[FH] = true;                               \
      break;                                      \
    }

    CASE_BIT(0, A);
    CASE_BIT(0, B);
    CASE_BIT(0, C);
    CASE_BIT(0, D);
    CASE_BIT(0, E);
    CASE_BIT(0, H);
    CASE_BIT(0, L);
    CASE_BIT(1, A);
    CASE_BIT(1, B);
    CASE_BIT(1, C);
    CASE_BIT(1, D);
    CASE_BIT(1, E);
    CASE_BIT(1, H);
    CASE_BIT(1, L);
    CASE_BIT(2, A);
    CASE_BIT(2, B);
    CASE_BIT(2, C);
    CASE_BIT(2, D);
    CASE_BIT(2, E);
    CASE_BIT(2, H);
    CASE_BIT(2, L);
    CASE_BIT(3, A);
    CASE_BIT(3, B);
    CASE_BIT(3, C);
    CASE_BIT(3, D);
    CASE_BIT(3, E);
    CASE_BIT(3, H);
    CASE_BIT(3, L);
    CASE_BIT(4, A);
    CASE_BIT(4, B);
    CASE_BIT(4, C);
    CASE_BIT(4, D);
    CASE_BIT(4, E);
    CASE_BIT(4, H);
    CASE_BIT(4, L);
    CASE_BIT(5, A);
    CASE_BIT(5, B);
    CASE_BIT(5, C);
    CASE_BIT(5, D);
    CASE_BIT(5, E);
    CASE_BIT(5, H);
    CASE_BIT(5, L);
    CASE_BIT(6, A);
    CASE_BIT(6, B);
    CASE_BIT(6, C);
    CASE_BIT(6, D);
    CASE_BIT(6, E);
    CASE_BIT(6, H);
    CASE_BIT(6, L);
    CASE_BIT(7, A);
    CASE_BIT(7, B);
    CASE_BIT(7, C);
    CASE_BIT(7, D);
    CASE_BIT(7, E);
    CASE_BIT(7, H);
    CASE_BIT(7, L);
#undef CASE_BIT

#define CASE_BIT_iHL(N)                         \
  case BIT_ ## N ## _iHL: {                     \
    bool bit = nthBit(iHL(), N);                \
    F[FZ] = bit == 0;                           \
    F[FN] = false;                              \
    F[FH] = true;                               \
    break;                                      \
  }

    CASE_BIT_iHL(0);
    CASE_BIT_iHL(1);
    CASE_BIT_iHL(2);
    CASE_BIT_iHL(3);
    CASE_BIT_iHL(4);
    CASE_BIT_iHL(5);
    CASE_BIT_iHL(6);
    CASE_BIT_iHL(7);
#undef CASE_BIT_iHL

#define CASE_RES(N, X)                                  \
    case RES_ ## N ## _ ## X: {                         \
      auto temp = static_cast<std::bitset<8>>(X);       \
      temp[N] = false;                                  \
      X = temp.to_ulong();                              \
      break;                                            \
    }

    CASE_RES(0, A);
    CASE_RES(0, B);
    CASE_RES(0, C);
    CASE_RES(0, D);
    CASE_RES(0, E);
    CASE_RES(0, H);
    CASE_RES(0, L);
    CASE_RES(1, A);
    CASE_RES(1, B);
    CASE_RES(1, C);
    CASE_RES(1, D);
    CASE_RES(1, E);
    CASE_RES(1, H);
    CASE_RES(1, L);
    CASE_RES(2, A);
    CASE_RES(2, B);
    CASE_RES(2, C);
    CASE_RES(2, D);
    CASE_RES(2, E);
    CASE_RES(2, H);
    CASE_RES(2, L);
    CASE_RES(3, A);
    CASE_RES(3, B);
    CASE_RES(3, C);
    CASE_RES(3, D);
    CASE_RES(3, E);
    CASE_RES(3, H);
    CASE_RES(3, L);
    CASE_RES(4, A);
    CASE_RES(4, B);
    CASE_RES(4, C);
    CASE_RES(4, D);
    CASE_RES(4, E);
    CASE_RES(4, H);
    CASE_RES(4, L);
    CASE_RES(5, A);
    CASE_RES(5, B);
    CASE_RES(5, C);
    CASE_RES(5, D);
    CASE_RES(5, E);
    CASE_RES(5, H);
    CASE_RES(5, L);
    CASE_RES(6, A);
    CASE_RES(6, B);
    CASE_RES(6, C);
    CASE_RES(6, D);
    CASE_RES(6, E);
    CASE_RES(6, H);
    CASE_RES(6, L);
    CASE_RES(7, A);
    CASE_RES(7, B);
    CASE_RES(7, C);
    CASE_RES(7, D);
    CASE_RES(7, E);
    CASE_RES(7, H);
    CASE_RES(7, L);
#undef CASE_RES


#define CASE_RES_iHL(N)                               \
  case RES_ ## N ## _iHL: {                           \
    auto temp = static_cast<std::bitset<8>>(iHL());   \
    temp[N] = false;                                  \
    iHL(temp.to_ulong());                             \
    break;                                            \
  }

    CASE_RES_iHL(0);
    CASE_RES_iHL(1);
    CASE_RES_iHL(2);
    CASE_RES_iHL(3);
    CASE_RES_iHL(4);
    CASE_RES_iHL(5);
    CASE_RES_iHL(6);
    CASE_RES_iHL(7);
#undef CASE_RES_iHL


#define CASE_SET(N, X)                                  \
    case SET_ ## N ## _ ## X: {                         \
      auto temp = static_cast<std::bitset<8>>(X);       \
      temp[N] = true;                                   \
      X = temp.to_ulong();                              \
      break;                                            \
    }

    CASE_SET(0, A);
    CASE_SET(0, B);
    CASE_SET(0, C);
    CASE_SET(0, D);
    CASE_SET(0, E);
    CASE_SET(0, H);
    CASE_SET(0, L);
    CASE_SET(1, A);
    CASE_SET(1, B);
    CASE_SET(1, C);
    CASE_SET(1, D);
    CASE_SET(1, E);
    CASE_SET(1, H);
    CASE_SET(1, L);
    CASE_SET(2, A);
    CASE_SET(2, B);
    CASE_SET(2, C);
    CASE_SET(2, D);
    CASE_SET(2, E);
    CASE_SET(2, H);
    CASE_SET(2, L);
    CASE_SET(3, A);
    CASE_SET(3, B);
    CASE_SET(3, C);
    CASE_SET(3, D);
    CASE_SET(3, E);
    CASE_SET(3, H);
    CASE_SET(3, L);
    CASE_SET(4, A);
    CASE_SET(4, B);
    CASE_SET(4, C);
    CASE_SET(4, D);
    CASE_SET(4, E);
    CASE_SET(4, H);
    CASE_SET(4, L);
    CASE_SET(5, A);
    CASE_SET(5, B);
    CASE_SET(5, C);
    CASE_SET(5, D);
    CASE_SET(5, E);
    CASE_SET(5, H);
    CASE_SET(5, L);
    CASE_SET(6, A);
    CASE_SET(6, B);
    CASE_SET(6, C);
    CASE_SET(6, D);
    CASE_SET(6, E);
    CASE_SET(6, H);
    CASE_SET(6, L);
    CASE_SET(7, A);
    CASE_SET(7, B);
    CASE_SET(7, C);
    CASE_SET(7, D);
    CASE_SET(7, E);
    CASE_SET(7, H);
    CASE_SET(7, L);
#undef CASE_SET

#define CASE_SET_iHL(N)                                 \
    case SET_ ## N ## _iHL: {                           \
      auto temp = static_cast<std::bitset<8>>(iHL());   \
      temp[N] = true;                                   \
      iHL(temp.to_ulong());                             \
      break;                                            \
    }

    CASE_SET_iHL(0);
    CASE_SET_iHL(1);
    CASE_SET_iHL(2);
    CASE_SET_iHL(3);
    CASE_SET_iHL(4);
    CASE_SET_iHL(5);
    CASE_SET_iHL(6);
    CASE_SET_iHL(7);
#undef CASE_SET_iHL

    default:
      std::printf("ERROR! Unknown CBopcode: 0x%02X\n", opcode);
      assert(false);
      break;
  }
}

}

#endif