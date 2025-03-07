#ifndef PPU_H
#define PPU_H

#include <bitset>

#include <functional>
#include "address-bus.hpp"

namespace gb {
class Gameboy;
class AddressBus;
// Todo rename this with PPU or somehting


class PPU {
  // Fixme these raw pointers should go
  AddressBus* bus;
  Gameboy* gameboy;

public:
  typedef std::bitset<2> color;

  typedef enum {
    LCD_Display_Enable      = 7,
    Window_Tile_Map_Select  = 6,
    Window_Display_Enable   = 5,
    Tile_Data_Select_Mode   = 4,
    BG_Tile_Map_Select      = 3,
    Sprite_Size             = 2,
    Sprite_Enable           = 1,
    BG_Window_Enable        = 0,
  } LCDCBit;

  typedef enum {
    STAT_Unused_Bit         = 7,
    LY_LYC_Interrupt_Enable = 6,
    Mode_2_Interrupt_Enable = 5,
    Mode_1_Interrupt_Enable = 4,
    Mode_0_Interrupt_Enable = 3,
    LY_LYC_Flag             = 2,
    PPU_Mode_msb            = 1,
    PPU_Mode_lsb            = 0, //Todo pandocs does not actually say what is msb and what lsb, check fux
  } STATBit;

  typedef enum {
    HBlank  = 0,
    VBlank  = 1,
    OAMScan = 2,
    Drawing = 3
  } PPUMode;

  struct Sprite {
    word yPos{};
    word xPos{};
    word tileNumber{};
    std::bitset<8> flags{};
  };

  static constexpr int width_{160};
  static constexpr int height_{144};
  static constexpr int totalPixels_{width_*height_};
  static constexpr int tilesInLine_{20};
  static constexpr int tilesInColumn_{18};
  static constexpr int tilemapSideSize_{32};
private:

  // TODO very ugly
  word lineClockCounter{0};
  bool STATAlreadyRequestedThisLine{false};

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

  // Store data encoded in two-word per 8-pixel format
  std::array<word, tilemapSideSize_> backgroundLineBufferLsb{};
  std::array<word, tilemapSideSize_> backgroundLineBufferMsb{};
  std::array<word, tilemapSideSize_> windowLineBufferLsb{};
  std::array<word, tilemapSideSize_> windowLineBufferMsb{};
  std::array<color, tilemapSideSize_ * 8> backgroundLineBuffer{};
  std::array<color, tilemapSideSize_ * 8> windowLineBuffer{};
  std::vector<Sprite> oamLineBuffer{};

  // Registers
  // LCD Control Register (LCDC : $FF40)

  void LCDC(LCDCBit flag, bool value);

  // LCD Status Register (STAT : $FF41)
  bool STAT(STATBit flag) const;
  void STAT(STATBit flag, bool value);

  void setPPUMode(PPUMode mode);

  void LY(word value) const;

  color OBP0Palette(color input);
  color OBP1Palette(color input);
  color BGPalette(color input);

  void lineEndLogic(word ly);

  void drawCurrentLine();
  // Only one STAT interrupt can be triggered
  void requestSTATInterruptIfPossible();

  dword getTilemapBaseAddress(bool drawWindow) const;
  int getTilemapOffset(bool drawWindow, int tileX, int tileY) const;
  dword getTiledataBaseAddress() const;
  bool isInsideWindow(int x, int y) const;
  void computeBackgroundLine();
  void computeWindowLine();
  void changeBufferFormatToColorArray();
  void flushLineToScreenBuffer();
  void flushSpritesToScreenBuffer();
  void setScreenPixel(int x, int y, color value);
  void resetOamBuffer();
  void addSpriteToBufferIfNeeded(int spriteNumber);
  int getSpriteHeight() const;

 public:

    // Constructor ////////////////////////////Gameboy///////////////////////////////////
  PPU(Gameboy* gameboy, AddressBus* bus);
  //////////////////////////////////////////////////////////////////////////////

  int frameCount{0};

  void machineClock();

  PPUMode getPPUMode() const;

  void printStatus() const;
  void printTileData() const;
  void printTileMap() const;

  bool LCDC(LCDCBit flag) const;
  word WY() const;
  word WX() const;
  word SCY() const;
  word SCX() const;
  word LY() const;
};

} // namespace gb

#endif // PPU_H
