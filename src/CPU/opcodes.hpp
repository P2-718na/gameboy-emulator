#ifndef OPCODES_H
#define OPCODES_H

#include <cassert>

namespace gb {
inline void CPU::executeOpcode(const OPCODE opcode) {
  // busyCycles needs to be set before executing opcode as
  // conditional jumps may increase its value
  busyCycles = getBusyCycles(opcode);

  assert(opcode != CB && "Special CB opcode needs to be parsed before calling this function!");
  assert(busyCycles != 0 && "Busy cycles need to be set before executing instructions!");

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
      crashed = true;
      break;


    // Misc ////////////////////////////////////////////////////////
    case NOP:
      break;

    case STOP:
      // ... But the Game Boy isn’t normal.
      // Depending on various factors, the STOP instruction might do different things.
      // Will it actually enter STOP mode?
      // Will it enter HALT mode instead?
      // Will it just be a NOP? Will it perform the speed switch I requested?
      // Will it magically become a 1-byte opcode and execute its second byte as another opcode?
      // Will it glitch the CPU in a non-deterministic fashion?
      // Follow the chart to figure out!
      // https://gbdev.io/pandocs/imgs/stop_diagram.svg
      // So, bear in mind that this is just an approximate implementation.
      halted = true;
      break;

    case HALT:
      halted = true;
      break;

    case DI:
      IME = false;
      break;

    case EI:
      // TODO EI effect should be delayed by one instruction
      IME = true;
      break;
    /////////////////////////////////////////////////////////////////

    // Bit Operations ///////////////////////////////////////////////
    case RLA: {
      const bool carry = nthBit(A, 7);
      A <<= 1;
      A |= F[FC];
      F[FC] = carry;
      F[FZ] = false;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    case RLCA: {
      const bool carry = nthBit(A, 7);
      A <<= 1;
      A |= carry;
      F[FC] = carry;
      F[FZ] = false;
      F[FH] = false;
      F[FN] = false;
      break;
    }

    case RRA: {
      const bool carry = nthBit(A, 0);
      A >>= 1;
      A |= (F[FC] << 7);
      F[FC] = carry;
      F[FZ] = false;
      F[FN] = false;
      F[FH] = false;
      break;
    }

    case RRCA: {
      const bool carry = nthBit(A, 0);
      A >>= 1;
      A |= (carry << 7);
      F[FC] = carry;
      F[FZ] = false; // Documentation conflicts for this. Tests pass tho.
      F[FH] = false;
      F[FN] = false;
      break;
    }
    ///////////////////////////////////////////////////////////

    // 16-bit Arithmetics /////////////////////////////////////
#define CASE_INC(X)                               \
    case INC_ ## X: {                             \
      X(X()+1);                                   \
      break;                                      \
    }

      CASE_INC(BC);
      CASE_INC(DE);
      CASE_INC(HL);
#undef CASE_INC

    case INC_SP:
      ++SP;
      break;

#define CASE_DEC(X)                               \
    case DEC_ ## X: {                             \
      X(X()-1);                                   \
      break;                                      \
    }

      CASE_DEC(BC);
      CASE_DEC(DE);
      CASE_DEC(HL);
#undef CASE_DEC

    case DEC_SP:
      --SP;
      break;

#define CASE_ADD_HL(X) {                             \
    case ADD_HL_ ## X:                               \
      const dword result = HL() + X();               \
      F[FN] = false;                                 \
      F[FH] = getHalfCarryFlag(HL(), X());           \
      F[FC] = getCarryFlag(HL(), X());               \
      HL(result);                                    \
      break;                                         \
    }
      CASE_ADD_HL(BC);
      CASE_ADD_HL(DE);
      CASE_ADD_HL(HL);
#undef CASE_ADD_HL

    case ADD_HL_SP: {
      const dword result = HL() + SP;
      F[FN] = false;
      F[FH] = getHalfCarryFlag(HL(), SP);
      F[FC] = getCarryFlag(HL(), SP);
      HL(result);
      break;
    }

    case ADD_SP_e: {
      const auto e = popPCSigned();
      const dword result = SP + e;
      F[FN] = false;
      F[FZ] = false;
      F[FH] = getHalfCarryFlag(dwordLsb(SP), e);
      F[FC] = getCarryFlag(dwordLsb(SP), e);
      SP = result;
      break;
    }

      /////////////////////////////////////////////////////////

    // Control FLow //////////////////////////////////////////
    // Relative jumps
    case JR_Z_e: {
      jr(F[FZ]);
      break;
    }
    case JR_C_e: {
      jr(F[FC]);
      break;
    }
    case JR_NZ_e: {
      jr(!F[FZ]);
      break;
    }
    case JR_NC_e: {
      jr(!F[FC]);
      break;
    }
    case JR_e: {
      jr(true);
      break;
    }

      // Returns
    case RET_Z: {
      ret(F[FZ]);
      break;
    }
    case RET_C: {
      ret(F[FC]);
      break;
    }
    case RET_NZ: {
      ret(!F[FZ]);
      break;
    }
    case RET_NC: {
      ret(!F[FC]);
      break;
    }
    case RET: {
      ret(true);
      break;
    }
    case RETI: {
      ret(true);
      IME = true;
      break;
    }

      // Immediate Jumps //
    case JP_nn: {
      jpImm(true);
      break;
    }
    case JP_Z_nn: {
      jpImm(F[FZ]);
      break;
    }
    case JP_C_nn: {
      jpImm(F[FC]);
      break;
    }
    case JP_NZ_nn: {
      jpImm(!F[FZ]);
      break;
    }
    case JP_NC_nn: {
      jpImm(!F[FC]);
      break;
    }
    case JP_HL: {
      PC = HL();
      break;
    }

      // Calls //
    case CALL_nn: {
      callImm(true);
      break;
    }
    case CALL_Z_nn: {
      callImm(F[FZ]);
      break;
    }
    case CALL_C_nn: {
      callImm(F[FC]);
      break;
    }
    case CALL_NZ_nn: {
      callImm(!F[FZ]);
      break;
    }
    case CALL_NC_nn: {
      callImm(!F[FC]);
      break;
    }

      // Reset //
#define CASE_RST(A)                               \
    case RST_ ## A: {                             \
      pushPCToStack();                            \
      PC = twoWordToDword(0x00, A);               \
      break;                                      \
    }

    CASE_RST(0x00);
    CASE_RST(0x10);
    CASE_RST(0x20);
    CASE_RST(0x30);
    CASE_RST(0x08);
    CASE_RST(0x18);
    CASE_RST(0x28);
    CASE_RST(0x38);
#undef CASE_RST
      ////////////////////////////////////////////////////////////////////

    // 16 Bit loads ////////////////////////////////////////////////
    case LD_inn_SP: {
      const auto lsb = popPC();
      const auto msb = popPC();
      const auto nn = twoWordToDword(msb, lsb);
      bus->write(nn, dwordLsb(SP));
      bus->write(nn + 1, dwordMsb(SP));
      break;
    }

    case LD_HL_SPe: {
      const auto e = popPCSigned();
      HL(SP + e);
      F[FC] = getCarryFlag(dwordLsb(SP), e);
      F[FH] = getHalfCarryFlag(dwordLsb(SP), e);
      F[FZ] = false;
      F[FN] = false;
      break;
    }

    case LD_SP_HL: {
      SP = HL();
      break;
    }


    case LD_SP_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      setSP(msb, lsb);
      break;
    }
    case LD_BC_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      BC(msb, lsb);
      break;
    }
    case LD_DE_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      DE(msb, lsb);
      break;
    }
    case LD_HL_nn: {
      const word lsb = popPC();
      const word msb = popPC();
      HL(msb, lsb);
      break;
    }

    case POP_BC:
      C = bus->read(SP++);
      B = bus->read(SP++);
      break;
    case POP_DE:
      E = bus->read(SP++);
      D = bus->read(SP++);
      break;
    case POP_HL:
      L = bus->read(SP++);
      H = bus->read(SP++);
      break;
    case POP_AF:
      F = bus->read(SP++) & 0b11110000;
      A = bus->read(SP++);
      break;

    case PUSH_BC:
      bus->write(--SP, B);
      bus->write(--SP, C);
      break;
    case PUSH_DE:
      bus->write(--SP, D);
      bus->write(--SP, E);
      break;
    case PUSH_HL:
      bus->write(--SP, H);
      bus->write(--SP, L);
      break;
    case PUSH_AF:
      bus->write(--SP, A);
      bus->write(--SP, F.to_ulong());
      break;
    ////////////////////////////////////////////////////////////////

    // 8 bit loads //////////////////////////////////////////////////
#define CASE_LDIMM(X)                             \
    case LD_ ## X ## _n: {                        \
      loadImm(X);                                 \
      break;                                      \
    }

    CASE_LDIMM(A);
    CASE_LDIMM(B);
    CASE_LDIMM(C);
    CASE_LDIMM(D);
    CASE_LDIMM(E);
    CASE_LDIMM(H);
    CASE_LDIMM(L);
#undef CASE_LDIMM

    case LD_iHL_n:
      iHL(popPC());
      break;

#define CASE_LD(X, Y)                             \
    case LD_ ## X ## _ ## Y: {                    \
      X = Y;                                      \
      break;                                      \
    }

      CASE_LD(A, A);
      CASE_LD(B, A);
      CASE_LD(C, A);
      CASE_LD(D, A);
      CASE_LD(E, A);
      CASE_LD(H, A);
      CASE_LD(L, A);
      CASE_LD(A, B);
      CASE_LD(B, B);
      CASE_LD(C, B);
      CASE_LD(D, B);
      CASE_LD(E, B);
      CASE_LD(H, B);
      CASE_LD(L, B);
      CASE_LD(A, C);
      CASE_LD(B, C);
      CASE_LD(C, C);
      CASE_LD(D, C);
      CASE_LD(E, C);
      CASE_LD(H, C);
      CASE_LD(L, C);
      CASE_LD(A, D);
      CASE_LD(B, D);
      CASE_LD(C, D);
      CASE_LD(D, D);
      CASE_LD(E, D);
      CASE_LD(H, D);
      CASE_LD(L, D);
      CASE_LD(A, E);
      CASE_LD(B, E);
      CASE_LD(C, E);
      CASE_LD(D, E);
      CASE_LD(E, E);
      CASE_LD(H, E);
      CASE_LD(L, E);
      CASE_LD(A, H);
      CASE_LD(B, H);
      CASE_LD(C, H);
      CASE_LD(D, H);
      CASE_LD(E, H);
      CASE_LD(H, H);
      CASE_LD(L, H);
      CASE_LD(A, L);
      CASE_LD(B, L);
      CASE_LD(C, L);
      CASE_LD(D, L);
      CASE_LD(E, L);
      CASE_LD(H, L);
      CASE_LD(L, L);
#undef CASE_LD


#define CASE_LD_iHL(X)                            \
    case LD_iHL_ ## X : {                         \
      iHL(X);                                     \
      break;                                      \
    }

    CASE_LD_iHL(A);
    CASE_LD_iHL(B);
    CASE_LD_iHL(C);
    CASE_LD_iHL(D);
    CASE_LD_iHL(E);
    CASE_LD_iHL(H);
    CASE_LD_iHL(L);
#undef CASE_LD_iHL

#define CASE_LD_iHL(X)                            \
    case LD_ ## X ## _iHL: {                      \
      X = iHL();                                  \
      break;                                      \
    }

    CASE_LD_iHL(A);
    CASE_LD_iHL(B);
    CASE_LD_iHL(C);
    CASE_LD_iHL(D);
    CASE_LD_iHL(E);
    CASE_LD_iHL(H);
    CASE_LD_iHL(L);
#undef CASE_LD_iHL

    case LD_A_iBC:
      A = bus->read(BC());
      break;
    case LD_A_iDE:
      A = bus->read(DE());
      break;
    case LD_iBC_A:
      bus->write(BC(), A);
      break;
    case LD_iDE_A:
      bus->write(DE(), A);
      break;

    case LD_iHLp_A: {
      iHL(A);
      HL(HL()+1);
      break;
    }
    case LD_iHLm_A: {
      iHL(A);
      HL(HL()-1);
      break;
    }
    case LD_A_iHLp: {
      A = iHL();
      HL(HL()+1);
      break;
    }
    case LD_A_iHLm: {
      A = iHL();
      HL(HL()-1);
      break;
    }

    case LDH_in_A: {
      const auto n = popPC();
      bus->write(twoWordToDword(0xFF, n), A);
      break;
    }
    case LDH_A_in: {
      const auto n = popPC();
      A = bus->read(twoWordToDword(0xFF, n));
      break;
    }

    case LDH_iC_A:
      bus->write(twoWordToDword(0xFF, C), A);
      break;
    case LDH_A_iC:
      A = bus->read(twoWordToDword(0xFF, C));
      break;


    case LD_inn_A: {
      auto lsb = popPC();
      auto msb = popPC();
      bus->write(twoWordToDword(msb, lsb), A);
      break;
    }
    case LD_A_inn: {
      auto lsb = popPC();
      auto msb = popPC();
      A = bus->read(twoWordToDword(msb, lsb));
      break;
    }
      //////////////////////////////////////////////////

    // 8 Bit arithmetic and logic ////////////////////////

#define SINGLE_BIT_CASES(X)                       \
    case INC_ ## X: {                             \
      incRegister(X);                             \
      break;                                      \
    }                                             \
    case DEC_ ## X: {                             \
      decRegister(X);                             \
      break;                                      \
    }                                             \
    case ADD_ ## X: {                             \
      addRegister(X);                             \
      break;                                      \
    }                                             \
    case SUB_ ## X: {                             \
      subRegister(X);                             \
      break;                                      \
    }                                             \
    case AND_ ## X: {                             \
      andRegister(X);                             \
      break;                                      \
    }                                             \
    case OR_ ## X: {                              \
      orRegister(X);                              \
      break;                                      \
    }                                             \
    case ADC_ ## X: {                             \
      adcRegister(X);                             \
      break;                                      \
    }                                             \
    case SBC_ ## X: {                             \
      sbcRegister(X);                             \
      break;                                      \
    }                                             \
    case XOR_ ## X: {                             \
      xorRegister(X);                             \
      break;                                      \
    }                                             \
    case CP_ ## X: {                              \
      cpRegister(X);                              \
      break;                                      \
    }                                             \

    SINGLE_BIT_CASES(A);
    SINGLE_BIT_CASES(B);
    SINGLE_BIT_CASES(C);
    SINGLE_BIT_CASES(D);
    SINGLE_BIT_CASES(E);
    SINGLE_BIT_CASES(H);
    SINGLE_BIT_CASES(L);
#undef SINGLE_BIT_CASES

    case INC_iHL: {
      F[FH] = getHalfCarryFlag(iHL(), 1);
      iHL(iHL() + 1);
      F[FZ] = iHL() == 0;
      F[FN] = false;
      break;
    }
    case DEC_iHL: {
      F[FH] = (iHL() & 0xf) - 1 < 0;
      iHL(iHL() - 1);
      F[FZ] = iHL() == 0;
      F[FN] = true;
      break;
    }
    case ADD_iHL: {
      addRegister(iHL());
      break;
    }
    case SUB_iHL: {
      subRegister(iHL());
      break;
    }
    case AND_iHL: {
      andRegister(iHL());
      break;
    }
    case OR_iHL: {
      orRegister(iHL());
      break;
    }
    case ADC_iHL: {
      adcRegister(iHL());
      break;
    }
    case SBC_iHL: {
      sbcRegister(iHL());
      break;
    }
    case XOR_iHL: {
      xorRegister(iHL());
      break;
    }
    case CP_iHL: {
      cpRegister(iHL());
      break;
    }

    case ADD_n: {
      const word n = popPC();
      addRegister(n);
      break;
    }
    case SUB_n: {
      const word n = popPC();
      subRegister(n);
      break;
    }
    case AND_n: {
      const word n = popPC();
      andRegister(n);
      break;
    }
    case OR_n: {
      const word n = popPC();
      orRegister(n);
      break;
    }
    case ADC_n: {
      const word n = popPC();
      adcRegister(n);
      break;
    }
    case SBC_n: {
      const word n = popPC();
      sbcRegister(n);
      break;
    }
    case XOR_n: {
      const word n = popPC();
      xorRegister(n);
      break;
    }
    case CP_n: {
      const word n = popPC();
      cpRegister(n);
      break;
    }

    case DAA: {
      // Taken from https://forums.nesdev.org/viewtopic.php?t=15944
      if (!F[FN]) {  // after an addition, adjust if (half-)carry occurred or if result is out of bounds
        if (F[FC] || A > 0x99) { A += 0x60; F[FC] = true; }
        if (F[FH] || (A & 0x0f) > 0x09) { A += 0x6; }
      } else {  // after a subtraction, only adjust if (half-)carry occurred
        if (F[FC]) { A -= 0x60; }
        if (F[FH]) { A -= 0x6; }
      }
      F[FZ] = (A == 0); // the usual z flag
      F[FH] = false; // h flag is always cleared
      break;
    }

    case SCF:
      F[FC] = true;
      F[FN] = false;
      F[FH] = false;
      break;

    case CPL:
      A = ~A; // Flips all bits of A
      F[FN] = true;
      F[FH] = true;
      break;

    case CCF:
      F[FC] = !F[FC]; //flips carry flag
      F[FN] = false;
      F[FH] = false;
      break;
    ///////////////////////////////////////////////////


    default:
      assert(false && "Unknown Opcode; this should not be possible.");
      break;
  }
}

inline void CPU::executeCBOpcode(CB_OPCODE opcode) {
  // busyCycles needs to be set before executing
  // cbopcode as conditional jumps may
  // increase its value
  busyCycles = getBusyCyclesCB(opcode);

  assert(busyCycles != 0);

  switch (opcode) {
#define CASE_RLC(X)                               \
    case RLC_ ## X: {                             \
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
      assert(false && "Unknown CB Opcode; this should not be possible.");
      break;
  }
}

}

#endif