#include <cassert>
#include <bitset>
#include "graphics.hpp"

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
  const std::bitset<8> reg = ram_->read(STATAddress);
  return reg[flag];
}

void Graphics::STAT(STATBit flag, bool value) {
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


Graphics::Graphics() {
  STAT(STAT_Unused_Bit, 1);
};


void Graphics::connectMemory(Memory* ram) {
  ram_ = ram;
}

PPUMode Graphics::getPPUMode() const {
  return STAT(PPU_Mode_msb) << 1 | STAT(PPU_Mode_lsb);
}

word Graphics::WY() const {
  return ram_->read(WYAddress);
}

word Graphics::WX() const {
  return ram_->read(WXAddress);
}


} // namespace gb