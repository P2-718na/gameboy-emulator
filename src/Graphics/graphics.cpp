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
  ram_->write(STATAddress, reg.to_ulong(), Memory::Ppu); // Todo test
}

void Graphics::setPPUMode(PPUMode mode) {
  assert(mode >= 0 && mode <= 3);

  switch (mode) {
    case HBlank:
      assert(getPPUMode() != HBlank);
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 0);
      break;
    case VBlank:
      assert(getPPUMode() != VBlank);
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 1);
      cpu_->requestInterrupt(VBlankBit);
      break;
    case OAMScan:
      assert(getPPUMode() != OAMScan);
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 0);
      break;
    case Drawing:
      assert(getPPUMode() != Drawing);
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 1);
      break;
  }
}

void Graphics::LY(const word value) const {
  ram_->write(LYAddress, value);
}

void Graphics::lineEndLogic(word ly) {
  assert(ly >= 0 && ly < 154);
  assert(lineDotCounter_ == 114);

  lineDotCounter_ = 0;

  // todo this should call interrupts
  switch (ly) {
    case 153:
      assert(getPPUMode() == VBlank);

      LY(0);
      setPPUMode(OAMScan);
      ++frameCount;
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

void Graphics::drawLine(bool drawWindow) {
  const dword tilemapBaseAddress = getTilemapBaseAddress(drawWindow);
  const dword tiledataBaseAddress = getTiledataBaseAddress(drawWindow);
  const bool  addressing8000 = tiledataBaseAddress == 0x8000;

  if (drawWindow) {
    // todo
    //  set tileX start in loop appropriately
  }

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != tilesInLine_; ++tileX) {
    const int tileY = LY() / 8;

    const dword tileNumberAddress = tilemapBaseAddress + getTilemapOffset(drawWindow, tileX, tileY);
    const unsigned char tileNumber = ram_->read(tileNumberAddress);
    const signed char signedTileNumber = ram_->read(tileNumberAddress);

    const int tiledataOffset = 2*((SCY() + LY())%8) // vertical position in current tile
      + (addressing8000 ? tileNumber*16 : signedTileNumber*16);

    const dword tiledataAddress = tiledataBaseAddress + tiledataOffset;

    if (tiledataOffset != 0) {
      //std::printf("%04i\n", tiledataOffset);
    }

    const word lsb = ram_->read(tiledataAddress);
    const word msb = ram_->read(tiledataAddress+1);

    flushDwordToBuffer(msb, lsb, tileX);
  }
}

void Graphics::drawOneWholeLine() {
  drawLine(false);

  if (!LCDC(Window_Display_Enable)) {
    return;
  }
// Todo
  //drawLine(true);
}

void Graphics::flushDwordToBuffer(std::bitset<8> msb, std::bitset<8> lsb, int tileX) {
  for (int i = 7; i != -1; --i) {
    const int pixelX = tileX*8 + (7-i);
    screenBuffer_[pixelX][LY()] = msb[i] << 1 | lsb[i];
  }
}

dword Graphics::getTilemapBaseAddress(bool drawWindow) const {
   const bool bankSwitchCond
     =  (!drawWindow && LCDC(BG_Tile_Map_Select))
     || (drawWindow && LCDC(Window_Tile_Map_Select));

   if (bankSwitchCond) {
     return 0x9C00;
   }
   return 0x9800;
}

int Graphics::getTilemapOffset(bool drawWindow, int tileX, int tileY) const {
  const word offsetX = drawWindow
                     ? tileX - WX() - 7
                     : 0x1F & (SCX() / 8 + tileX);

  const word offsetY = drawWindow
                     ? tileY - WY()
                     : (0xFF & (SCY() + LY())) / 8; // Todo check /8

  const dword offset = offsetX + (offsetY*tilemapSideSize_);
  assert(offset < tilemapSideSize_*tilemapSideSize_);

  return offset;
}

dword Graphics::getTiledataBaseAddress(bool drawWindow) const {
  if (LCDC(Tile_Data_Select_Mode)) {
    return 0x8000;
  }

  return 0x9000;
}

//bool Graphics::drawingWindow() const {
//  if (getTileX() >= WX()-7 && getTileY() >= WY()) {
//    return true;
//  }
//
//  return false;
//}
//
//
//
//word Graphics::fetcherGetTileDataLow() {
//  const dword address = fetcherGetTileAddress();
//  if (address != 0x8000) {
//    std::printf("READING ADDRESS: %04X\n, CURRENT TILE: (%03i, %03i), TILEMAP: %02x", address, getTileX(), getTileY(), fetcherGetTileNumber());
//  }
//  return ram_->read(address);
//}
//
//word Graphics::fetcherGetTileDataHigh() {
//  const dword address = fetcherGetTileAddress();
//  return ram_->read(address+1);
//}

Graphics::Graphics(Gameboy* gameboy, Memory* ram) : ram_{ram}, gameboy{gameboy} {
  STAT(STAT_Unused_Bit, true);
  setPPUMode(OAMScan); // Todo actually find a reference that states this is correct boot mode lol
};

void Graphics::machineClock() {
  const PPUMode mode = getPPUMode();

  assert(mode >= 0 && mode <= 3);

  // PPU State machine
  switch (mode) {
    case HBlank:
      assert(lineDotCounter_ >= 63);
      assert(lineDotCounter_ < 114);

      // Do HBlank logic if any

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

      ++lineDotCounter_;
      if (lineDotCounter_ == 20) {
        setPPUMode(Drawing);
      }
      break;

    case Drawing:
      assert(lineDotCounter_ >= 20 && lineDotCounter_ < 63);
      if (lineDotCounter_ == 20) {
        drawOneWholeLine();
      }

      ++lineDotCounter_;

      if (lineDotCounter_ == 63) {
        setPPUMode(HBlank);
      }
      break;
  }
}

Graphics::PPUMode Graphics::getPPUMode() const {
  return (PPUMode)(STAT(PPU_Mode_msb) << 1 | STAT(PPU_Mode_lsb));
}

void Graphics::printStatus() const {
  std::printf("____________________________________PPU__\n");
  std::printf("| DOT | M | LY  | LYC | CMP | SCY | SCX |\n");
  std::printf("| %03i | %01i | %03i | $%02X |  %01i  | %03i | %03i |\n",
              lineDotCounter_,
              getPPUMode(),
              LY(),
              ram_->read(LYCAddress),
              STAT(LY_LYC_Flag),
              SCY(),
              SCX()
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void Graphics::printBuffer() const {
  std::cout << "\033[2J" << std::endl;
  static constexpr std::array<char, 4> chars{'.', 'o', 'O', '#'};


  for (int y = 0; y != 144; ++y) {
    for (int x = 0; x != 160; ++x) {
      std::cout << chars[screenBuffer_[x][y].to_ulong()];
    }

    std::printf("\n");
  }
}

void Graphics::printTileData() const {
  for (dword address = 0x8000; address != 0x9800;) {
    const dword lsb = ram_->read(address++);
    const dword msb = ram_->read(address++);
    std::printf("%04X ", lsb | (msb << 8));

    if (address % 64 == 0) {
      std::printf("\n");
    }

    if (address % (64*8) == 0) {
      std::printf("\n");
    }
  }
}

// todo tiles are printed in different order (tile rows)
void Graphics::printTileMap() const {
  for (dword address = 0x9800; address != 0xA000; ++address) {
    if (address % 32 == 0) {
      std::printf("\n");
    }

    if (address == 0x9C00) {
      std::printf("\n");
    }

    const auto lsb = ram_->read(address);
    std::printf("%02X ", lsb);
  }
}

word Graphics::WY() const {
  return ram_->read(WYAddress);
}

word Graphics::WX() const {
  return ram_->read(WXAddress);
}

word Graphics::SCY() const {
  return ram_->read(SCYAddress);
}

word Graphics::SCX() const {
  return ram_->read(SCXAddress);
}

word Graphics::LY() const {
  return ram_->read(LYAddress);
}

bool Graphics::isScreenOn() const {
  return LCDC(LCD_Display_Enable);
}

// Todo this has to go
void Graphics::doTheUglyHackyThing(Processor* cpu) {
  cpu_ = cpu;
}


} // namespace gb