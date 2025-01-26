#include "graphics.hpp"
#include <bitset>
#include <cassert>

#include <iostream>
#include <ostream>

#include "memory.hpp"

namespace gb {

bool Graphics::LCDC(LCDCBit flag) const {
  const std::bitset<8> reg = ram_->read(LCDCAddress);
  return reg[flag];
}

void Graphics::LCDC(LCDCBit flag, bool value) {
  std::bitset<8> reg = ram_->read(LCDCAddress);
  reg[flag] = value;
  ram_->write(LCDCAddress, reg.to_ulong()); // Todo test
}

bool Graphics::STAT(STATBit flag) const {
  if (flag == LY_LYC_Flag) {
    return ram_->read(LYCAddress) == ram_->read(LYAddress);
  }

  const std::bitset<8> reg = ram_->read(STATAddress);
  return reg[flag];
}

void Graphics::STAT(STATBit flag, bool value) {
  assert(flag != LY_LYC_Flag);

  std::bitset<8> reg = ram_->read(STATAddress);
  reg[flag] = value;
  ram_->write(STATAddress, reg.to_ulong()); // Todo test
}


void Graphics::setPPUMode(PPUMode mode) {
  assert(mode >= 0 && mode <= 3);

  switch (mode) {
    case HBlank:
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 0);
    break;
    case VBlank:
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 1);
    break;
    case OAMScan:
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 0);
    break;
    case Drawing:
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 1);
    break;
  }
}

word Graphics::LY() const {
  return ram_->read(LYAddress);
}

void Graphics::LY(const word value) const {
  ram_->write(LYAddress, value);
}


void Graphics::lineEndLogic(word ly) {
  assert(ly >= 0 && ly < 154);
  assert(lineDotCounter_ == 114);

  lineDotCounter_ = 0;

  switch (ly) {
    case 153:
      assert(getPPUMode() == VBlank);

      LY(0);
      setPPUMode(OAMScan);
      break;

    case 143:
      assert(getPPUMode() == HBlank);

      LY(ly+1);
      setPPUMode(VBlank);
      break;

    default: {
      const auto mode = getPPUMode();
      assert(mode == HBlank || mode == VBlank);

      LY(ly + 1);
      if (mode == HBlank) {
        setPPUMode(OAMScan);
      }
      break;
    }
  }
}

Graphics::Graphics() = default;

void Graphics::connectMemory(Memory* ram) {
  // Fixme this cause sigsegv if anything is done before setting ram.
  //  rewrite this in a bettereway
  ram_ = ram;
  STAT(STAT_Unused_Bit, 1);
  setPPUMode(OAMScan); // Todo actually find a reference that states this is correct boot mode lol
}

void Graphics::dotClock() {
  const PPUMode mode = getPPUMode();

  assert(mode >= 0 && mode <= 3);

  // PPU State machine
  switch (mode) {
    case HBlank:
      assert(lineDotCounter_ >= 63);
      assert(lineDotCounter_ < 114);

      // Do HBlank loigc

      ++lineDotCounter_;
      if (lineDotCounter_ == 114) {
        const auto ly = LY();
        assert(ly < 144);
        lineEndLogic(ly);
      }
      break;


    case VBlank:

      // Do VBlank logic if any

      ++lineDotCounter_;
      if (lineDotCounter_ == 114) {
        const auto ly = LY();
        assert(ly >= 144);
        assert(ly < 154);
        lineEndLogic(ly);
      }
      break;

    case OAMScan:
      assert(lineDotCounter_ < 20);

      // Do oam scan logic
      // Todo actually this timing is not fixed so
      //  we need to change some asserts

      ++lineDotCounter_;
      if (lineDotCounter_ == 20) {
        setPPUMode(Drawing);
      }
      break;

    case Drawing:
      assert(lineDotCounter_ >= 20 && lineDotCounter_ < 63);

      // Do fifo logic

      ++lineDotCounter_;
      if (lineDotCounter_ == 63) {
        setPPUMode(HBlank);
      }
      break;
  }
}


void Graphics::machineClock() {
  // 4 dots per machine clock.
  // Doing it this way breaks the correct timings.
  for (int i = 0; i != 4; ++i) {
    dotClock();
  }
}


Graphics::PPUMode Graphics::getPPUMode() const {
  return (PPUMode)(STAT(PPU_Mode_msb) << 1 | STAT(PPU_Mode_lsb));
}

void Graphics::printStatus() {
  std::printf("__PPU_______________________________________________________\n");
  std::printf("| DOT | M | LY  | LYC | CMP |\n");
  std::printf("| %03i | %01i | %03i | $%02X |  %01i  |\n",
    lineDotCounter_,
    getPPUMode(),
    LY(),
    ram_->read(LYCAddress),
    STAT(LY_LYC_Flag)
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

word Graphics::WY() const {
  return ram_->read(WYAddress);
}

word Graphics::WX() const {
  return ram_->read(WXAddress);
}


} // namespace gb