#ifndef PPU_H
#define PPU_H

#include <bitset>

#include "address-bus.hpp"

namespace gb {

class Gameboy;
class AddressBus;

class PPU {
  // Fixme these raw pointers should go
  AddressBus* bus;
  Gameboy* gameboy;

public:
  typedef std::bitset<2> color;

  typedef enum {
    LCD_DISPLAY_ENABLE      = 7,
    WINDOW_TILE_MAP_SELECT  = 6,
    WINDOW_DISPLAY_ENABLE   = 5,
    TILE_DATA_SELECT_MODE   = 4,
    BG_TILE_MAP_SELECT      = 3,
    SPRITE_SIZE             = 2,
    SPRITE_ENABLE           = 1,
    BG_WINDOW_ENABLE        = 0,
  } LCDC_BIT;

  typedef enum {
    STAT_UNUSED_BIT         = 7,
    LY_LYC_INTERRUPT_ENABLE = 6,
    MODE_2_INTERRUPT_ENABLE = 5,
    MODE_1_INTERRUPT_ENABLE = 4,
    MODE_0_INTERRUPT_ENABLE = 3,
    LY_EQUALS_LYC             = 2,
    PPU_MODE_MSB            = 1,
    PPU_MODE_LSB            = 0, // Todo pandocs does not actually say what is msb and what lsb, check fux
  } STAT_BIT;

  typedef enum {
    H_BLANK  = 0,
    V_BLANK  = 1,
    OAM_SCAN = 2,
    DRAWING = 3
  } PPU_MODE;

  struct Sprite {
    word yPos{};
    word xPos{};
    word tileNumber{};
    std::bitset<8> flags{};
  };

  static constexpr int WIDTH{160};
  static constexpr int HEIGHT{144};
  static constexpr int TOTAL_PIXELS{WIDTH*HEIGHT};
  static constexpr int TILE_WIDTH{8};
  static constexpr int TILES_IN_LINE{20};
  static constexpr int TILES_IN_COLUMN{18};
  static constexpr int TILEMAP_SIDE_SIZE{32};
  static constexpr int WORDS_PER_TILE_LINE{2};
  static constexpr int TILE_SIZE_IN_WORDS{TILE_WIDTH * 2};
  static constexpr int MAX_SPRITES_PER_LINE{10};
  static constexpr int MAX_SPRITE_HEIGHT{16};
  static constexpr int SPRITE_WIDTH{8};

  // For some reason, WX needs to be shifted by 7
  static constexpr int WX_SHIFT = 7;

private:
  // FIXME addresses
  static constexpr dword LCDCAddress = 0xFF40;
  static constexpr dword STATAddress = 0xFF41;
  // Controlled by the program, set LCD viewport position
  static constexpr dword SCYAddress  = 0xFF42;
  static constexpr dword SCXAddress  = 0xFF43;
  // LY indicates the current horizontal line
  // LY can hold any value from 0 to 153, with values
  // from 144 to 153 indicating the VBlank period.
  static constexpr dword LYAddress   = 0xFF44;
  static constexpr dword LYCAddress  = 0xFF45;
  static constexpr dword DMAAddress  = 0xFF46;
  // Palettes (background, objects)
  static constexpr dword BGPAddress   = 0xFF47;
  static constexpr dword OBP0Address  = 0xFF48;
  static constexpr dword OBP1Address  = 0xFF49;
  // Controlled by the program, set window position
  static constexpr dword WYAddress   = 0xFF4A;
  static constexpr dword WXAddress   = 0xFF4B;

  // Count how many machine clocks have been fired in this line.
  // This is needed to implement correct PPU timing.
  word currentLineClockCounter{0};

  // Only one STAT interrupt can be fired for each line.
  bool STATAlreadyRequestedThisLine{false};

  // Store data encoded in two-word-per-8-pixel format
  std::array<word, TILEMAP_SIDE_SIZE> backgroundLineBufferLsb{};
  std::array<word, TILEMAP_SIDE_SIZE> backgroundLineBufferMsb{};
  std::array<word, TILEMAP_SIDE_SIZE> windowLineBufferLsb{};
  std::array<word, TILEMAP_SIDE_SIZE> windowLineBufferMsb{};
  std::array<color, TILEMAP_SIDE_SIZE * TILE_WIDTH> backgroundLineBuffer{};
  std::array<color, TILEMAP_SIDE_SIZE * TILE_WIDTH> windowLineBuffer{};
  std::vector<Sprite> OAMLineBuffer{};

  // Write to registers ////////////////////////////////////////////////////////
  // LCD Control Register (LCDC : $FF40)
  void LCDC(LCDC_BIT flag, bool value);
  // LCD Status Register (STAT : $FF41)
  bool STAT(STAT_BIT flag) const;
  void STAT(STAT_BIT flag, bool value);
  // LY Register 0xFF44
  void LY(word value) const;
  // This writes to STAT and triggers interrupts
  // if needed
  void setPPUMode(PPU_MODE mode);
  // Only one STAT interrupt can be triggered
  void tryRequestSTATInterrupt();

  // Main loop logic ///////////////////////////////////////////////////////////
  void lineEndLogic(word ly);
  // Prepares which sprites need to be drawn
  void addSpriteToBufferIfNeeded(int spriteNumber);
  // drawCurrentLine
  void drawCurrentLine();
  // This prepares all the tile data needed to draw the (full 32 tile) current
  // line and stores it in 2-byte-per-8-pixel format in
  // backgroundLineBufferLsb/msb
  void prepareBackgroundLine();
  // Same but for window
  void prepareWindowLine();
  // TODO prepareXXLine are two functions which are extremely similar. They could
  //  probably be refactored in a way that code duplication is reduced.
  // This converts the buffer format to a more suitable one for the
  // current code structure and saves the result to
  // backgroundLineBuffer, windowLineBuffer
  void computeColorBuffers();
  // Flush background/window (first) to screen buffer; then,
  // overwrite sprites (with transparency).
  void flushLineToScreenBuffer() const;
  void flushSpritesToScreenBuffer();
  // Finally, reset OAM buffer.
  void resetOamBuffer();

  // Helper functions for drawing //////////////////////////////////////////////
  dword getTilemapBaseAddress(bool drawingWindow) const;
  int   getTilemapOffset(bool drawWindow, int tileX, int tileY) const;
  dword getTiledataBaseAddress() const;
  bool  isPositionInsideWindow(int x, int y) const;
  int   getSpriteHeight() const;

 public:
  // Constructor ///////////////////////////////////////////////////////////////
  PPU(Gameboy* gameboy, AddressBus* bus);
  //////////////////////////////////////////////////////////////////////////////

  // How many frames have been drawn
  unsigned long long frameCount{0};

  // To be called exactly once for each machine cycle
  void machineClock();

  // Apply palette to a color
  color applyPalette0(color input) const;
  color applyPalette1(color input) const;
  color applyPaletteBG(color input) const;

  // Debug stuff
  void printStatus()   const;
  void printTileData() const;
  void printTileMap()  const;

  // Register getters
  bool LCDC(LCDC_BIT flag) const;
  PPU_MODE getPPUMode()    const;
  word WY()                const;
  word WX()                const;
  word SCY()               const;
  word SCX()               const;
  word LY()                const;
};

} // namespace gb

#endif // PPU_H
