#include "ppu.hpp"
#include <bitset>
#include <cassert>
#include <gameboy.hpp>

#include "address-bus.hpp"

namespace gb {

bool PPU::LCDC(LCDC_BIT flag) const {
  const std::bitset<8> reg = bus->read(REG_LCDC);
  return reg[flag];
}

void PPU::LCDC(LCDC_BIT flag, bool value) {
  std::bitset<8> reg = bus->read(REG_LCDC);
  reg[flag] = value;
  bus->write(REG_LCDC, reg.to_ulong());
}

bool PPU::STAT(const STAT_BIT flag) const {
  if (flag == LY_EQUALS_LYC) {
    return bus->read(REG_LYC) == bus->read(REG_LY);
  }

  const std::bitset<8> reg = bus->read(REG_STAT);
  return reg[flag];
}

void PPU::STAT(const STAT_BIT flag, const bool value) {
  std::bitset<8> reg = bus->read(REG_STAT);
  reg[flag] = value;
  bus->write(REG_STAT, reg.to_ulong(), AddressBus::PPU);
}

void PPU::setPPUMode(const PPU_MODE mode) {
  assert(mode >= 0 && mode <= 3);

  // STAT Interrupts are triggered when a specific mode is entered.
  switch (mode) {
    case H_BLANK:
      assert(getPPUMode() != H_BLANK);
      STAT(PPU_MODE_MSB, false);
      STAT(PPU_MODE_LSB, false);
      // HBlank == mode 0
      if (STAT(MODE_0_INTERRUPT_ENABLE)) {
        tryRequestSTATInterrupt();
      }
      break;
    case V_BLANK:
      assert(getPPUMode() != V_BLANK);
      STAT(PPU_MODE_MSB, false);
      STAT(PPU_MODE_LSB, true);
      // VBlank == mode1
      if (STAT(MODE_1_INTERRUPT_ENABLE)) {
        tryRequestSTATInterrupt();
      }
      gameboy->requestInterrupt(INTERRUPT_VBLANK);
      break;
    case OAM_SCAN:
      assert(getPPUMode() != OAM_SCAN);
      STAT(PPU_MODE_MSB, true);
      STAT(PPU_MODE_LSB, false);
      if (STAT(MODE_2_INTERRUPT_ENABLE)) {
        tryRequestSTATInterrupt();
      }
      break;
    case DRAWING:
      assert(getPPUMode() != DRAWING);
      STAT(PPU_MODE_MSB, 1);
      STAT(PPU_MODE_LSB, 1);
      break;
  }
}

void PPU::LY(const word value) const {
  bus->write(REG_LY, value, AddressBus::PPU);
}

PPU::color PPU::applyPalette0(gb::PPU::color input) const {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(REG_OBP0) & (0b11 << shift)) >> shift;
}
PPU::color PPU::applyPalette1(gb::PPU::color input) const {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(REG_OBP1) & (0b11 << shift)) >> shift;
}
PPU::color PPU::applyPaletteBG(gb::PPU::color input) const {
  const auto shift = input.to_ulong() * 2;
  return (bus->read(REG_BGP) & (0b11 << shift)) >> shift;
}

void PPU::lineEndLogic(const word ly) {
  assert(ly < 154u);
  assert(currentLineClockCounter == 114);

  currentLineClockCounter = 0;

  switch (ly) {
    case 153:
      assert(getPPUMode() == V_BLANK);

      LY(0);
      setPPUMode(OAM_SCAN);
      ++frameCount;
      break;

    case 143:
      assert(getPPUMode() == H_BLANK);

      LY(ly+1);
      setPPUMode(V_BLANK);
      break;

    default: {
      const auto mode = getPPUMode();
      assert(mode == H_BLANK || mode == V_BLANK);

      LY(ly + 1);
      if (mode == H_BLANK) {
        setPPUMode(OAM_SCAN);
      }
      break;
    }
  }

  const word LYC = bus->read(REG_LYC);

  // The Game Boy constantly compares the value of the LYC and LY registers.
  // When both values are identical, the “LYC=LY” flag in the STAT register is
  // set, and (if enabled) a STAT interrupt is requested.
  if (LY() == LYC && STAT(LY_LYC_INTERRUPT_ENABLE)) {
    tryRequestSTATInterrupt();
    STAT(LY_EQUALS_LYC, true);
    return;
  }

  // TODO LY_LYC flag should also be updated whenever we write
  //  a new LYC value to bus address
  STAT(LY_EQUALS_LYC, false);
  STATAlreadyRequestedThisLine = false;
}

void PPU::prepareBackgroundLine() {
  if (!LCDC(BG_WINDOW_ENABLE)) {
    backgroundLineBufferLsb.fill(0);
    backgroundLineBufferMsb.fill(0);
    return;
  }
  // This is all straight from docs
  const dword tilemapBaseAddress = getTilemapBaseAddress(false);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == TILEDATA_BASE_8000;

  // This is the current tile we are drawing. We need to take into account the scrolling!
  const int tileY = ((LY() + SCY()) / 8) % TILEMAP_SIDE_SIZE;

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != TILEMAP_SIDE_SIZE; ++tileX) {
    // Tile numbers are in a 32x32 grid. We want to loop over the full line at current tileY.
    const dword tileNumberAddress = tilemapBaseAddress + (tileX + tileY*TILEMAP_SIDE_SIZE);
    const word tileNumber_u = bus->read(tileNumberAddress);
    const auto tileNumber_s = static_cast<signed char>(bus->read(tileNumberAddress));

    // Starting from tiledataBase address, we have the tiles indexed by their tile number.
    // Each tile takes 2 words per 8 lines of space.
    assert(TILE_SIZE_IN_WORDS == 2 * 8);
    // So, first we compute the offset given by the tile number, using the correct addressing method...
    const int tiledataTileOffset = isAddressing8000 ? (tileNumber_u * TILE_SIZE_IN_WORDS) : (tileNumber_s * TILE_SIZE_IN_WORDS);

    // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
    // We have already computed the scrolling (we are selecting the tile at the scrolled posiiton) but we still need
    // LY and SCY to compute the line (taking modulo 8 = width of a tile).
    assert(TILE_WIDTH == 8);
    const int tileDataRowOffset = WORDS_PER_TILE_LINE * ((SCY() + LY()) % TILE_WIDTH);

    // Then we sum the two offsets.
    const dword tiledataAddress = tiledataBaseAddress + tiledataTileOffset + tileDataRowOffset;

    backgroundLineBufferLsb[tileX] = bus->read(tiledataAddress);
    backgroundLineBufferMsb[tileX] = bus->read(tiledataAddress + 1);
  }
}

void PPU::prepareWindowLine() {
  if (!LCDC(BG_WINDOW_ENABLE)) {
    windowLineBufferLsb.fill(0);
    windowLineBufferMsb.fill(0);
    return;
  }

  // We can spare ourselves some additional computation if we check here if
  // the last pixel of the line is outside the window.
  if (!isPositionInsideWindow(WIDTH - 1, LY())) {
    return;
  }

  const dword tilemapBaseAddress = getTilemapBaseAddress(true);
  const dword tiledataBaseAddress = getTiledataBaseAddress();
  const bool  isAddressing8000 = tiledataBaseAddress == TILEDATA_BASE_8000;

  // This is the current tile we are drawing. We need to take into account the scrolling!
  // Here, the modulus is added just in case we are drawing outside the window
  assert(TILE_WIDTH == 8);
  const int tileY = ((LY() - WY()) / TILE_WIDTH) % TILEMAP_SIDE_SIZE;

  // Loop through each tile in the current line
  for (int tileX = 0; tileX != TILEMAP_SIDE_SIZE; ++tileX) {
    // Tile numbers are in a 32x32 grid. We want to loop over the full line at current tileY.
    const dword tileNumberAddress = tilemapBaseAddress + (tileX + tileY*TILEMAP_SIDE_SIZE);
    const word tileNumber_u = bus->read(tileNumberAddress);
    const auto tileNumber_s = static_cast<signed char>(bus->read(tileNumberAddress));

    // Starting from tiledataBase address, we have the tiles indexed by their tile number.
    // Each tile takes 2 words per 8 lines of space.
    assert(TILE_SIZE_IN_WORDS == 2 * 8);
    // So, first we compute the offset given by the tile number, using the correct addressing method...
    const int tiledataTileOffset = isAddressing8000 ? (tileNumber_u * TILE_SIZE_IN_WORDS) : (tileNumber_s * TILE_SIZE_IN_WORDS);

    // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
    // We have already computed the scrolling (we are selecting the tile at the scrolled posiiton) but we still need
    // LY and SCY to compute the line (taking modulo 8 = width of a tile).
    const int tileDataRowOffset = WORDS_PER_TILE_LINE * ((LY()-WY()) % TILE_WIDTH);

    // Then we sum the two offsets.
    const dword tiledataAddress = tiledataBaseAddress + tiledataTileOffset + tileDataRowOffset;

    windowLineBufferLsb[tileX] = bus->read(tiledataAddress);
    windowLineBufferMsb[tileX] = bus->read(tiledataAddress + 1);
  }
}

void PPU::drawCurrentLine() {
  prepareBackgroundLine();
  prepareWindowLine();
  computeColorBuffers();
  flushLineToScreenBuffer();
  flushSpritesToScreenBuffer();
  resetOamBuffer();
}

void PPU::tryRequestSTATInterrupt() {
  // At most one stat interrupt per line
  if (STATAlreadyRequestedThisLine) {
    return;
  }
  STATAlreadyRequestedThisLine = true;
  gameboy->requestInterrupt(INTERRUPT_ID ::INTERRUPT_STAT);
}

void PPU::computeColorBuffers() {
  for (int tileX = 0; tileX != TILEMAP_SIDE_SIZE; ++tileX) {
    const std::bitset<8> backgroundMsb = backgroundLineBufferMsb[tileX];
    const std::bitset<8> backgroundLsb = backgroundLineBufferLsb[tileX];
    const std::bitset<8> windowMsb = windowLineBufferMsb[tileX];
    const std::bitset<8> windowLsb = windowLineBufferLsb[tileX];

    for (int bit = 0; bit != 8; ++bit) {
      // Here 7 is not the magic number WX_SHIFT!
      const int pixelX = tileX * 8 + (7 - bit);

      const color backgroundValue = backgroundMsb[bit] << 1 | backgroundLsb[bit];
      backgroundLineBuffer[pixelX] = backgroundValue;

      const color windowValue = windowMsb[bit] << 1 | windowLsb[bit];
      windowLineBuffer[pixelX] = windowValue;
    }
  }
}

void PPU::flushLineToScreenBuffer() const {
  for (int x = 0; x != WIDTH; ++x) {
    const color value = isPositionInsideWindow(x, LY())
      ? windowLineBuffer[(x - WX() + WX_SHIFT) % (TILEMAP_SIDE_SIZE * TILE_WIDTH)]
      : applyPaletteBG(backgroundLineBuffer[(x + SCX()) % (TILEMAP_SIDE_SIZE * TILE_WIDTH)]);

    gameboy->screenBuffer[x + LY() * WIDTH] = value;
  }
}

bool PPU::isPositionInsideWindow(const int x, const int y) const {
  if (!LCDC(WINDOW_DISPLAY_ENABLE)) {
    return false;
  }

  return y >= WY() && x >= WX() - WX_SHIFT;
}

dword PPU::getTilemapBaseAddress(const bool drawingWindow) const {
   const bool bankSwitchCond
     =  (!drawingWindow && LCDC(BG_TILE_MAP_SELECT))
     || (drawingWindow && LCDC(WINDOW_TILE_MAP_SELECT));

   if (bankSwitchCond) {
     return TILEMAP_BASE_1;
   }

   return TILEMAP_BASE_0;
}

dword PPU::getTiledataBaseAddress() const {
  return LCDC(TILE_DATA_SELECT_MODE)
    ? TILEDATA_BASE_8000
    : TILEDATA_BASE_9000;
}

void PPU::resetOamBuffer() {
  // Reserve and clear won't cause reallocation after
  // the space was allocated for the first time
  OAMLineBuffer.clear();
  OAMLineBuffer.reserve(MAX_SPRITES_PER_LINE);
}

void PPU::addSpriteToBufferIfNeeded(const int spriteNumber) {
  constexpr int wordsPerSprite = 4;
  const dword spriteAddress = OAM_MEMORY_LOWER_BOUND + wordsPerSprite * spriteNumber;

  const Sprite sprite{
    .yPos       = bus->read(spriteAddress),
    .xPos       = bus->read(spriteAddress + 1),
    .tileNumber = bus->read(spriteAddress + 2),
    .flags      = bus->read(spriteAddress + 3)
  };

  if (OAMLineBuffer.size() == MAX_SPRITES_PER_LINE) {
     return;
  }

  if (sprite.xPos == 0) {
     return;
  }

  // Careful! MAX_SPRITE_HEIGHT must be on the side of LY() otherwise it underflows
  if (sprite.yPos  > LY() + MAX_SPRITE_HEIGHT) {
     return;
  }
  if (sprite.yPos + getSpriteHeight() <= LY() + MAX_SPRITE_HEIGHT) {
     return;
  }
  assert(sprite.yPos > 0);

  OAMLineBuffer.push_back(sprite);
}

int PPU::getSpriteHeight() const {
  if (LCDC(SPRITE_SIZE)) {
    // Game is running in double height sprite mode!
    return 16;
  }

  return 8;
}

void PPU::flushSpritesToScreenBuffer() {
  if (!LCDC(SPRITE_ENABLE)) {
     return;
  }

  std::array<color, 8> spritePixels;
  // TODO this is very long and ugly. It needs to be refactored.
  for (auto& sprite : OAMLineBuffer) {
    // Sprites always use 8000 addressing method
    const word spriteLine = LY() - (sprite.yPos - MAX_SPRITE_HEIGHT);
    bool drawingBottomTile = spriteLine > 7;
    assert(spriteLine < MAX_SPRITE_HEIGHT && "Only visible sprites should be added to buffer. Something went wrong.");
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
    const int tiledataTileOffset = tileNumber * TILE_SIZE_IN_WORDS;

    // Then, we need to choose the line of the tile we are drawing right now. Each line is two words.
    assert(sprite.yPos != 0);
    const int tileDataRowOffset = sprite.flags[6]
    ? WORDS_PER_TILE_LINE * (TILE_WIDTH - ( LY() - (sprite.yPos - MAX_SPRITE_HEIGHT)) % TILE_WIDTH)
    : WORDS_PER_TILE_LINE * (( LY() - (sprite.yPos - MAX_SPRITE_HEIGHT)) % TILE_WIDTH);

    const std::bitset<8> tileDataLsb = bus->read(TILEDATA_BASE_8000 + tiledataTileOffset + tileDataRowOffset);
    const std::bitset<8> tileDataMsb = bus->read(TILEDATA_BASE_8000 + tiledataTileOffset + tileDataRowOffset + 1);

    for (int bit = 0; bit != SPRITE_WIDTH; ++bit) {
      const color value = tileDataMsb[bit] << 1 | tileDataLsb[bit];
      // Here 7 is not the magic WX_SHIFT!
      spritePixels[7-bit] = value;
    }

    for (int spriteX = 0; spriteX != SPRITE_WIDTH; ++spriteX) {
      const int screenX = spriteX - SPRITE_WIDTH + sprite.xPos;

      if (screenX >= WIDTH) {
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
           ? applyPalette1(value)
           : applyPalette0(value);
        gameboy->screenBuffer[screenX + LY() * WIDTH] = palettedValue;
      }
    }
  }
}

PPU::PPU(Gameboy* gameboy, AddressBus* bus)
  : bus{ bus }
  , gameboy{ gameboy } {
  STAT(STAT_UNUSED_BIT, true);
  setPPUMode(OAM_SCAN); // TODO actually find a reference that states this is correct mode at boot
  resetOamBuffer();
};

// Todo this function is too long, it should be broken up into smaller pieces.
void PPU::machineClock() {
  const PPU_MODE mode = getPPUMode();
  assert(mode >= 0 && mode <= 3);

  // PPU State machine
  switch (mode) {
    case H_BLANK: {
      assert(currentLineClockCounter >= 63);
      assert(currentLineClockCounter < 114);

      // During HBlank PPU does nothing.

      ++currentLineClockCounter;
      if (currentLineClockCounter != 114) {
        // End of line has not been reached
        break;
      }

      const auto ly = LY();
      assert(ly < 144);
      lineEndLogic(ly);
      break;
    }

    case V_BLANK: {
      // During VBlank PPU does nothing.

      ++currentLineClockCounter;
      if (currentLineClockCounter != 114) {
        // End of line has not been reached
        break;
      }

      const auto ly = LY();
      assert(ly >= 144);
      assert(ly < 154);
      lineEndLogic(ly);
      break;
    }

    case OAM_SCAN: {
      assert(currentLineClockCounter < 20);

      // We need to check 40 sprites per 20 clock cycles.
      // This approximately spreads out the load the same
      // way the real hardware would.
      constexpr int spritesParsedPerClock{2};
      const int spriteIndex = currentLineClockCounter * spritesParsedPerClock;

      // If any of the sprites need to be drawn in the current line,
      // then add them to OAMLineBuffer (only if there is still space).
      addSpriteToBufferIfNeeded(spriteIndex);
      addSpriteToBufferIfNeeded(spriteIndex + 1);

      ++currentLineClockCounter;
      if (currentLineClockCounter != 20) {
        break;
      }
      setPPUMode(DRAWING);
      break;
    }

    case DRAWING: {
      assert(currentLineClockCounter >= 20 && currentLineClockCounter < 63);
      ++currentLineClockCounter;

      // We first check if we need to switch mode as it
      // is slightly faster than checking for draw timing first.
      if (currentLineClockCounter == 63) {
        setPPUMode(H_BLANK);
        break;
      }

      // The whole line gets drawn atomically here.
      if (currentLineClockCounter == 21) {
        drawCurrentLine();
        break;
      }
    }
  }
}

PPU::PPU_MODE PPU::getPPUMode() const {
  return static_cast<PPU_MODE>(STAT(PPU_MODE_MSB) << 1 | STAT(PPU_MODE_LSB));
}

void PPU::printStatus() const {
  std::printf("____________________________________PPU__\n");
  std::printf("| DOT | M | LY  | LYC | CMP | SCY | SCX |\n");
  std::printf("| %03i | %01i | %03i | $%02X |  %01i  | %03i | %03i |\n",
    currentLineClockCounter,
              getPPUMode(),
              LY(),
              bus->read(REG_LYC),
              STAT(LY_EQUALS_LYC),
              SCY(),
              SCX()
  );
  std::printf("‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\n");
}

void PPU::printTileData() const {
  for (dword address = TILEDATA_LOWER_BOUND; address != TILEDATA_UPPER_BOUND;) {
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
  for (dword address = TILEMAP_LOWER_BOUND; address != TILEMAP_UPPER_BOUND; ++address) {
    if (address % 32 == 0) {
      std::printf("\n");
    }

    if (address == TILEMAP_BASE_1) {
      std::printf("\n");
    }

    const auto lsb = bus->read(address);
    std::printf("%02X ", lsb);
  }
}

word PPU::WY() const {
  return bus->read(REG_WY);
}

word PPU::WX() const {
  return bus->read(REG_WX);
}

word PPU::SCY() const {
  return bus->read(REG_SCY);
}

word PPU::SCX() const {
  return bus->read(REG_SCX);
}

word PPU::LY() const {
  return bus->read(REG_LY);
}

} // namespace gb