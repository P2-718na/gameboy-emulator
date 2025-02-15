#include "PPU.hpp"
#include <bitset>
#include <cassert>
#include <gameboy.hpp>

#include "address-bus.hpp"

namespace gb {

bool PPU::LCDC(LCDCBit flag) const {
  const std::bitset<8> reg = bus->read(LCDCAddress);
  return reg[flag];
}

void PPU::LCDC(LCDCBit flag, bool value) {
  std::bitset<8> reg = bus->read(LCDCAddress);
  reg[flag] = value;
  bus->write(LCDCAddress, reg.to_ulong()); // Todo test
}

bool PPU::STAT(STATBit flag) const {
  if (flag == LY_LYC_Flag) {
    return bus->read(LYCAddress) == bus->read(LYAddress);
  }

  const std::bitset<8> reg = bus->read(STATAddress);
  return reg[flag];
}

void PPU::STAT(STATBit flag, bool value) {
  assert(flag != LY_LYC_Flag);

  std::bitset<8> reg = bus->read(STATAddress);
  reg[flag] = value;
  bus->write(STATAddress, reg.to_ulong(), AddressBus::Ppu); // Todo test
}

void PPU::setPPUMode(PPUMode mode) {
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
      gameboy->requestInterrupt(VBlankBit);
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

void PPU::LY(const word value) const {
  bus->write(LYAddress, value);
}

void PPU::lineEndLogic(word ly) {
  assert(ly < 154u);
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

void PPU::drawBackground() {
  const dword tilemapBaseAddress = getTilemapBaseAddress(false);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == 0x8000;

  const int tileY = LY() / 8;

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != tilesInLine_; ++tileX) {
    const dword tileNumberAddress = tilemapBaseAddress + getTilemapOffset(false, tileX, tileY);
    const word tileNumber = bus->read(tileNumberAddress);
    const auto signedTileNumber = static_cast<signed char>(tileNumber);

    const int tiledataOffset = 2*((SCY() + LY())%8) // vertical position in current tile
      + (isAddressing8000 ? tileNumber*16 : signedTileNumber*16);

    const dword tiledataAddress = tiledataBaseAddress + tiledataOffset;

    lineBufferLsb[tileX] = bus->read(tiledataAddress);
    lineBufferMsb[tileX] = bus->read(tiledataAddress + 1);
  }
}

void PPU::drawWindow() {
  if (!LCDC(Window_Display_Enable)) {
    return;
  }

  const int tileY = LY() / 8;

  if (LY() < WY()) {
    return;
  }

  const dword tilemapBaseAddress = getTilemapBaseAddress(true);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == 0x8000;

  // Todo handle what to do if WX < 7;
  int tilesToDraw = tilesInLine_ - WX()/8;
  if (tilesToDraw < 0) {
    tilesToDraw = 0;
  }

  // TileX refers to the "Window tiles". To map these to screen tiles, we need to add (WX() - 7),
  for (int tileX = 0; tileX != tilesToDraw; ++tileX) {
    const dword tileNumberAddress = tilemapBaseAddress + getTilemapOffset(true, tileX, tileY);
    const word tileNumber = bus->read(tileNumberAddress);
    const auto signedTileNumber = static_cast<signed char>(tileNumber);

    const int tiledataOffset = 2*(LY()%8) // vertical position in current tile
      + (isAddressing8000 ? tileNumber*16 : signedTileNumber*16);

    const dword tiledataAddress = tiledataBaseAddress + tiledataOffset;

    lineBufferLsb[tileX + WX()/8] = bus->read(tiledataAddress);
    lineBufferMsb[tileX + WX()/8] = bus->read(tiledataAddress + 1);
  }
}


void PPU::drawCurrentFullLine() {
  drawBackground();
  drawWindow();
  // Todo draw sprites
  //drawSprites()

  for (int tileX = 0; tileX != tilesInLine_; ++tileX) {
    const word lsb = lineBufferLsb[tileX];
    const word msb = lineBufferMsb[tileX];

    flushDwordToBuffer(msb, lsb, tileX);
  }
}

void PPU::flushDwordToBuffer(std::bitset<8> msb, std::bitset<8> lsb, int tileX) {
  for (int i = 7; i != -1; --i) {
    const int pixelX = tileX * 8 + (7-i);
    const color value = msb[i] << 1 | lsb[i];
    setPixel(pixelX, LY(), value);
  }
}

void PPU::setPixel(int x, int y, color value) {
  gameboy->screenBuffer[x + y*width_] = value;
}

dword PPU::getTilemapBaseAddress(bool drawingWindow) const {
   const bool bankSwitchCond
     =  (!drawingWindow && LCDC(BG_Tile_Map_Select))
     || (drawingWindow && LCDC(Window_Tile_Map_Select));

   if (bankSwitchCond) {
     return 0x9C00;
   }
   return 0x9800;
}

int PPU::getTilemapOffset(bool drawingWindow, int tileX, int tileY) const {
  assert(tileX >= 0);
  assert(tileY >= 0);

  const word offsetX = drawingWindow
                     ? tileX
                     : 0x1F & (SCX() / 8 + tileX);

  const word offsetY = drawingWindow
                     ? tileY
                     : (0xFF & (SCY() + LY())) / 8;

  const dword offset = offsetX + (offsetY*tilemapSideSize_);
  assert(offset < tilemapSideSize_*tilemapSideSize_);

  return offset;
}

dword PPU::getTiledataBaseAddress() const {
  return LCDC(Tile_Data_Select_Mode)
    ? 0x8000
    : 0x9000;
}

PPU::PPU(Gameboy* gameboy, AddressBus* ram) : GBComponent {gameboy, ram} {
  STAT(STAT_Unused_Bit, true);
  setPPUMode(OAMScan); // Todo actually find a reference that states this is correct boot mode lol
};

void PPU::machineClock() {
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
        drawCurrentFullLine();
      }

      ++lineDotCounter_;

      if (lineDotCounter_ == 63) {
        setPPUMode(HBlank);
      }
      break;
  }
}

PPU::PPUMode PPU::getPPUMode() const {
  return static_cast<PPUMode>(STAT(PPU_Mode_msb) << 1 | STAT(PPU_Mode_lsb));
}

void PPU::printStatus() const {
  std::printf("____________________________________PPU__\n");
  std::printf("| DOT | M | LY  | LYC | CMP | SCY | SCX |\n");
  std::printf("| %03i | %01i | %03i | $%02X |  %01i  | %03i | %03i |\n",
              lineDotCounter_,
              getPPUMode(),
              LY(),
              bus->read(LYCAddress),
              STAT(LY_LYC_Flag),
              SCY(),
              SCX()
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void PPU::printTileData() const {
  for (dword address = 0x8000; address != 0x9800;) {
    const dword lsb = bus->read(address++);
    const dword msb = bus->read(address++);
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
void PPU::printTileMap() const {
  for (dword address = 0x9800; address != 0xA000; ++address) {
    if (address % 32 == 0) {
      std::printf("\n");
    }

    if (address == 0x9C00) {
      std::printf("\n");
    }

    const auto lsb = bus->read(address);
    std::printf("%02X ", lsb);
  }
}

word PPU::WY() const {
  return bus->read(WYAddress);
}

word PPU::WX() const {
  return bus->read(WXAddress);
}

word PPU::SCY() const {
  return bus->read(SCYAddress);
}

word PPU::SCX() const {
  return bus->read(SCXAddress);
}

word PPU::LY() const {
  return bus->read(LYAddress);
}

} // namespace gb