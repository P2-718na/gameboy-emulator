#ifndef OPCODES_H
#define OPCODES_H

namespace gb {

inline int Processor::getBusyCycles(Opcode opcode) {
  switch (opcode) {
    case LD_HL_nn:
    case LD_SP_nn:
      return 3;

    case LD_HLm_A:
      return 2;

    case NOP:
    case XOR_A:
      return 1;

    default:
      std::cout << "ERROR! Unknown opcode cycles: " << opcode << std::endl;
      return 1;
  }
}

inline void Processor::executeOpcode(Opcode opcode) {

  switch (opcode) {
    case LD_HL_nn: {
      const word lsb = ram_->read(PC);
      ++PC;
      const word msb = ram_->read(PC);
      ++PC;
      H = msb;
      L = lsb;
      break;
    }

    case LD_SP_nn: {
      const word lsb = ram_->read(PC);
      ++PC;
      const word msb = ram_->read(PC);
      ++PC;
      setSP(msb, lsb);
      break;
    }

    case LD_HLm_A: {
      ram_->write(HL(), A);
      HL(HL()-1);
      break;
    }

    case XOR_A:
      A ^= B;
      F[FZ] = !A;
      F[FN] = false;
      F[FH] = false;
      F[FC] = false;
      break;
      break;

    case NOP:
      break;

    default:
      std::cout << "ERROR! Unknown opcode: " << opcode << std::endl;
      break;
  }
}

}

#endif