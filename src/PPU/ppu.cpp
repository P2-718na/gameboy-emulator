#include "ppu.hpp"
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
  std::bitset<8> reg = bus->read(STATAddress);
  reg[flag] = value;
  bus->write(STATAddress, reg.to_ulong(), AddressBus::PPU); // Todo test
}

void PPU::setPPUMode(PPUMode mode) {
  assert(mode >= 0 && mode <= 3);

  // STAT Interrupts are triggered when a specific mode is entered.
  switch (mode) {
    case HBlank:
      assert(getPPUMode() != HBlank);
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 0);
      // HBlank == mode 0
      if (STAT(Mode_0_Interrupt_Enable)) {
        requestSTATInterruptIfPossible();
      }
      break;
    case VBlank:
      assert(getPPUMode() != VBlank);
      STAT(PPU_Mode_msb, 0);
      STAT(PPU_Mode_lsb, 1);
      // VBlank == mode1
      if (STAT(Mode_1_Interrupt_Enable)) {
        requestSTATInterruptIfPossible();
      }
      gameboy->requestInterrupt(INTERRUPT_VBLANK);
      break;
    case OAMScan:
      assert(getPPUMode() != OAMScan);
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 0);
      if (STAT(Mode_2_Interrupt_Enable)) {
        requestSTATInterruptIfPossible();
      }
      break;
    case Drawing:
      assert(getPPUMode() != Drawing);
      STAT(PPU_Mode_msb, 1);
      STAT(PPU_Mode_lsb, 1);
      break;
  }
}

void PPU::LY(const word value) const {
  bus->write(LYAddress, value, AddressBus::PPU);
}

PPU::color PPU::OBP0Palette(gb::PPU::color input) {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(OBP0Address) & (0b11 << shift)) >> shift;
}
PPU::color PPU::OBP1Palette(gb::PPU::color input) {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(OBP1Address) & (0b11 << shift)) >> shift;
}
PPU::color PPU::BGPalette(gb::PPU::color input) {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(BGPAddress) & (0b11 << shift)) >> shift;
}

void PPU::lineEndLogic(word ly) {
  assert(ly < 154u);
  assert(lineClockCounter == 114);

  lineClockCounter = 0;

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

  const word LYC = bus->read(LYCAddress);

  // The Game Boy constantly compares the value of the LYC and LY registers.
  // When both values are identical, the “LYC=LY” flag in the STAT register is
  // set, and (if enabled) a STAT interrupt is requested.
  if (LY() == LYC && STAT(LY_LYC_Interrupt_Enable)) {
    // TOdo these two things have the same name... Bad
    requestSTATInterruptIfPossible();
    STAT(LY_LYC_Flag, true);
    return;
  }

  // TODO Ly_LYC flag shoul also be updated whenever we write a new LYC value to bus address
  STAT(LY_LYC_Flag, false);
  STATAlreadyRequestedThisLine = false;
}

void PPU::computeBackgroundLine() {
  const dword tilemapBaseAddress = getTilemapBaseAddress(false);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == 0x8000;

  // This is the current tile we are drawing. We need to take into account the scrolling!
  const int tileY = ((LY() + SCY()) / 8) % tilemapSideSize_;

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != tilemapSideSize_; ++tileX) {
    // Tile numbers are in a 32x32 grid. We want to loop over the full line at current tileY.
    const dword tileNumberAddress = tilemapBaseAddress + (tileX + tileY*tilemapSideSize_);
    const word tileNumber_u = bus->read(tileNumberAddress);
    const auto tileNumber_s = static_cast<signed char>(bus->read(tileNumberAddress));

    // Starting from tiledataBase address, we have the tiles indexed by their tile number.
    // Each tile takes 2 words per 8 lines of space.
    constexpr int tileSize = 2 * 8; // (words)
    // So, first we compute the offset given by the tile number, using the correct addressing method...
    const int tiledataTileOffset = isAddressing8000 ? (tileNumber_u * tileSize) : (tileNumber_s * tileSize);

    // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
    // We have already computed the scrolling (we are selecting the tile at the scrolled posiiton) but we still need
    // LY and SCY to compute the line (taking modulo 8 = width of a tile).
    const int tileDataRowOffset = 2 * ((SCY() + LY()) % 8);

    // Then we sum the two offsets.
    const dword tiledataAddress = tiledataBaseAddress + tiledataTileOffset + tileDataRowOffset;

    backgroundLineBufferLsb[tileX] = bus->read(tiledataAddress);
    backgroundLineBufferMsb[tileX] = bus->read(tiledataAddress + 1);
  }
}

void PPU::computeWindowLine() {
  // We can spare ourselves some additional computation if we already check if
  // the last pixel of the line is outside the window.
  if (!isInsideWindow(width_-1, LY())) {
    return;
  }

  const dword tilemapBaseAddress = getTilemapBaseAddress(true);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == 0x8000;

  // This is the current tile we are drawing. We need to take into account the scrolling!
  // Here, the modulus is added just in case we are drawing outside the window
  const int tileY = ((LY() - WY()) / 8) % tilemapSideSize_;

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != tilemapSideSize_; ++tileX) {
    // Tile numbers are in a 32x32 grid. We want to loop over the full line at current tileY.
    const dword tileNumberAddress = tilemapBaseAddress + (tileX + tileY*tilemapSideSize_);
    const word tileNumber_u = bus->read(tileNumberAddress);
    const auto tileNumber_s = static_cast<signed char>(bus->read(tileNumberAddress));

    // Starting from tiledataBase address, we have the tiles indexed by their tile number.
    // Each tile takes 2 words per 8 lines of space.
    constexpr int tileSize = 2 * 8; // (words)
    // So, first we compute the offset given by the tile number, using the correct addressing method...
    const int tiledataTileOffset = isAddressing8000 ? (tileNumber_u * tileSize) : (tileNumber_s * tileSize);

    // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
    // We have already computed the scrolling (we are selecting the tile at the scrolled posiiton) but we still need
    // LY and SCY to compute the line (taking modulo 8 = width of a tile).
    const int tileDataRowOffset = 2 * ((LY()-WY()) % 8);

    // Then we sum the two offsets.
    const dword tiledataAddress = tiledataBaseAddress + tiledataTileOffset + tileDataRowOffset;

    windowLineBufferLsb[tileX] = bus->read(tiledataAddress);
    windowLineBufferMsb[tileX] = bus->read(tiledataAddress + 1);
  }
}

void PPU::drawCurrentLine() {
  // Todo make pretty
  if (!LCDC(BG_Window_Enable)) {
    windowLineBufferLsb.fill(0);
    windowLineBufferMsb.fill(0);
    backgroundLineBufferLsb.fill(0);
    backgroundLineBufferMsb.fill(0);
  } else {
    computeBackgroundLine();
    computeWindowLine();
  }

  changeBufferFormatToColorArray();
  flushLineToScreenBuffer();
  flushSpritesToScreenBuffer();
  resetOamBuffer();
}

void PPU::requestSTATInterruptIfPossible() {
  // At most one stat interrupt per line
  if (STATAlreadyRequestedThisLine) {
    return;
  }
  STATAlreadyRequestedThisLine = true;
  gameboy->requestInterrupt(INTERRUPT_ID ::INTERRUPT_STAT);
}

void PPU::changeBufferFormatToColorArray() {
  for (int tileX = 0; tileX != tilemapSideSize_; ++tileX) {
    const std::bitset<8> backgroundMsb = backgroundLineBufferMsb[tileX];
    const std::bitset<8> backgroundLsb = backgroundLineBufferLsb[tileX];
    const std::bitset<8> windowMsb = windowLineBufferMsb[tileX];
    const std::bitset<8> windowLsb = windowLineBufferLsb[tileX];

    for (int bit = 0; bit != 8; ++bit) {
      const int pixelX = tileX * 8 + (7-bit);

      const color backgroundValue = backgroundMsb[bit] << 1 | backgroundLsb[bit];
      backgroundLineBuffer[pixelX] = backgroundValue;

      const color windowValue = windowMsb[bit] << 1 | windowLsb[bit];
      windowLineBuffer[pixelX] = windowValue;
    }
  }
}

void PPU::flushLineToScreenBuffer() {
  for (int x = 0; x != width_; ++x) {
    const color value = isInsideWindow(x, LY())
      ? windowLineBuffer[(x - WX() + 7) % (tilemapSideSize_ * 8)]
      : BGPalette(backgroundLineBuffer[(x + SCX()) % (tilemapSideSize_ * 8)]);

    gameboy->screenBuffer[x + LY() * width_] = value;
  }
}

bool PPU::isInsideWindow(int x, int y) const {
  if (!LCDC(Window_Display_Enable)) {
    return false;
  }

  return y >= WY() && x >= WX() - 7;
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

dword PPU::getTiledataBaseAddress() const {
  return LCDC(Tile_Data_Select_Mode)
    ? 0x8000
    : 0x9000;
}

void PPU::resetOamBuffer() {
  oamLineBuffer.clear();
  oamLineBuffer.reserve(10);
}

void PPU::addSpriteToBufferIfNeeded(const int spriteNumber) {
  // TOdo address bus $FE00-FE9F OAM memory
  constexpr int wordsPerSprite = 4;
  const dword spriteAddress = 0xFE00 + wordsPerSprite * spriteNumber;

  const Sprite sprite{
    .yPos       = bus->read(spriteAddress),
    .xPos       = bus->read(spriteAddress + 1),
    .tileNumber = bus->read(spriteAddress + 2),
    .flags      = bus->read(spriteAddress + 3)
  };

  if (oamLineBuffer.size() == 10) {
     return;
  }

  if (sprite.xPos == 0) {
     return;
  }

  // Careful! 16 myst be on the side of LY() otherwise it underflows
  if (sprite.yPos  > LY() + 16) {
     return;
  }

  // Careful! 16 myst be on the side of LY() otherwise it underflows
  if (sprite.yPos + getSpriteHeight() <= LY() + 16) {
     return;
  }

  assert(sprite.yPos > 0);

  oamLineBuffer.push_back(sprite);
}

int PPU::getSpriteHeight() const {
  if (LCDC(Sprite_Size)) {
     return 16;
  }

  return 8;
}

void PPU::flushSpritesToScreenBuffer() {
  if (!LCDC(Sprite_Enable)) {
     return;
  }

  std::array<color, 8> spritePixels;

  for (auto& sprite : oamLineBuffer) {
     constexpr int tileSize = 2 * 8; // (words)
     // Sprites always use 8000 addressing method

     const word spriteLine = LY() - (sprite.yPos - 16);
     bool drawingBottomTile = spriteLine > 7;
     assert(spriteLine < 16 && "Only visible sprites should be added to buffer. Something went wrong.");
     word tileNumber;

     if (sprite.flags[6]) {
      drawingBottomTile = !drawingBottomTile;
     }

     if (getSpriteHeight() == 8) {
      tileNumber = sprite.tileNumber;
     } else if (drawingBottomTile) {
      tileNumber = sprite.tileNumber | 0b00000001;
     } else {
      tileNumber = sprite.tileNumber & 0b11111110;
     }
     const int tiledataTileOffset = tileNumber * tileSize;

     // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
     assert(sprite.yPos != 0);
     const int tileDataRowOffset = sprite.flags[6]
     ? 2 * (8 - ( LY() - (sprite.yPos - 16)) % 8)
     : 2 * (( LY() - (sprite.yPos - 16)) % 8);

     const std::bitset<8> tileDataLsb = bus->read(0x8000 + tiledataTileOffset + tileDataRowOffset);
     const std::bitset<8> tileDataMsb = bus->read(0x8000 + tiledataTileOffset + tileDataRowOffset + 1);

     for (int bit = 0; bit != 8; ++bit) {
      const color value = tileDataMsb[bit] << 1 | tileDataLsb[bit];
      spritePixels[7-bit] = value;
     }

     //todo const sprite Width
     for (int spriteX = 0; spriteX != 8; ++spriteX) {
      const int screenX = spriteX - 8 + sprite.xPos;

      if (screenX >= width_) {
        break;
      }

      const bool flipX = sprite.flags[5];
      const color value = spritePixels[flipX ? 7 - spriteX : spriteX];
      if (value == 0) {
        continue;
      }

      // Priority flag
      if (!sprite.flags[7] || backgroundLineBuffer[screenX] == 0) {
        const auto palettedValue = sprite.flags[4]
           ? OBP1Palette(value)
           : OBP0Palette(value);
        gameboy->screenBuffer[screenX + LY() * width_] = palettedValue;
      }
     }
  }
}

PPU::PPU(Gameboy* gameboy, AddressBus* bus)
  : bus{ bus }
  , gameboy{ gameboy } {
  STAT(STAT_Unused_Bit, true);
  setPPUMode(OAMScan); // Todo actually find a reference that states this is correct boot mode lol
  resetOamBuffer();
};

void PPU::machineClock() {
  const PPUMode mode = getPPUMode();
  assert(mode >= 0 && mode <= 3);

  // PPU State machine
  switch (mode) {
    case HBlank:
      assert(lineClockCounter >= 63);
      assert(lineClockCounter < 114);

      // During HBlank PPU does nothing.

      ++lineClockCounter;
      if (lineClockCounter == 114) {
        const auto ly = LY();
        assert(ly < 144);
        lineEndLogic(ly);
      }
      break;


    case VBlank:

      // During VBlank PPU does nothing.

      ++lineClockCounter;
      if (lineClockCounter == 114) {
        const auto ly = LY();
        assert(ly >= 144);
        assert(ly < 154);
        lineEndLogic(ly);
      }
      break;

    case OAMScan: {
      assert(lineClockCounter < 20);

      // We need to check 40 sprites per 20 clock cycles.
      const int spriteNumber = lineClockCounter * 2;
      addSpriteToBufferIfNeeded(spriteNumber);
      addSpriteToBufferIfNeeded(spriteNumber + 1);

      ++lineClockCounter;
      if (lineClockCounter == 20) {
        setPPUMode(Drawing);
      }
      break;
    }

    case Drawing:
      assert(lineClockCounter >= 20 && lineClockCounter < 63);
      ++lineClockCounter;

      if (lineClockCounter == 21) {
        drawCurrentLine();
        break;
      }

      if (lineClockCounter == 63) {
        setPPUMode(HBlank);
        break;
      }
  }
}

PPU::PPUMode PPU::getPPUMode() const {
  return static_cast<PPUMode>(STAT(PPU_Mode_msb) << 1 | STAT(PPU_Mode_lsb));
}

void PPU::printStatus() const {
  std::printf("____________________________________PPU__\n");
  std::printf("| DOT | M | LY  | LYC | CMP | SCY | SCX |\n");
  std::printf("| %03i | %01i | %03i | $%02X |  %01i  | %03i | %03i |\n",
    lineClockCounter,
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