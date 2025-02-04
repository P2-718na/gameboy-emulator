#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <bitset>

#include "memory.hpp"

namespace gb {

class Graphics {

  typedef std::bitset<2> color;

  static constexpr int width_{160};
  static constexpr int height_{144};
  static constexpr int totalPixels_{width_*height_};
  static constexpr int tilesInLine_{20};
  static constexpr int tilesInColumn_{18};
  static constexpr int tilemapSideSize_{32};

  std::array< std::array<color, 144 >, 160> screenBuffer_{};
  // TODO very ugly
  word lineDotCounter_{0};

  struct sprite {
    //todo oam memory
  };

  // Graphics data encoded in the 2BPP format (explained above)
  // is stored in VRAM at addresses $8000-$97FF and is
  // usually referred to by so-called “Tile Numbers”.
  // As each tile takes up 16 bytes of memory, a
  // “Tile Number” is essentially just an index of a
  // 16-byte-block within this section of VRAM.

  // In order to set which tiles should be displayed in the Background / Window
  // grids, background maps are used. The VRAM sections $9800-$9BFF and
  // $9C00-$9FFF each contain one of these background maps.
  // A background map consists of 32x32 bytes representing tile numbers organized
  // row by row.
  // This means that the first byte in a background map is the Tile Number of
  // the Tile at the very top left
  // The byte after is the Tile Number of the Tile to the right of it and so
  // on. The 33rd byte would represent the Tile Number of the leftmost tile in the
  // second tile row.

  // The OAM (standing for “Object Attribute Memory”)
  // section of memory ranges from $FE00-$FE9F and contains
  // data used to display Sprites (also known as “Objects”)
  // on screen. Each sprite takes up 4 bytes in this section
  // of memory, allowing for a total of 40 sprites to be displayed
  // at any given time. Each entry is structured as follows:

  static constexpr dword LCDCAddress = 0xFF40;
  static constexpr dword STATAddress = 0xFF41;

  // Controlled by the program, set LCD viewport position
  static constexpr dword SCYAddress  = 0xFF42;
  static constexpr dword SCXAddress  = 0xFF43;

  // LY indicates the current horizontal line
  // LY can hold any value from 0 to 153, with values
  // from 144 to 153 indicating the VBlank period.
  static constexpr dword LYAddress   = 0xFF44;
  static constexpr dword LYCAddress  = 0xFF45; // todo The Game Boy constantly compares the value of the LYC and LY registers. When both values are identical, the “LYC=LY” flag in the STAT register is set, and (if enabled) a STAT interrupt is requested.

  static constexpr dword DMAAddress  = 0xFF46;

  // Palettes (background, objects)
  static constexpr dword BGBAddress   = 0xFF47;
  static constexpr dword OBP0Address  = 0xFF48;
  static constexpr dword OBP1Address  = 0xFF49;

  // Controlled by the program, set window position
  static constexpr dword WYAddress   = 0xFF4A;
  static constexpr dword WXAddress   = 0xFF4B;

  //  Todo same as for processor.hpp
  Memory* ram_;

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

  // Registers
  // LCD Control Register (LCDC : $FF40)
  bool LCDC(LCDCBit flag) const;
  void LCDC(LCDCBit flag, bool value);

  // LCD Status Register (STAT : $FF41)
  bool STAT(STATBit flag) const;
  void STAT(STATBit flag, bool value);

  void setPPUMode(PPUMode mode);

  void LY(word value) const;

  void lineEndLogic(word ly);

  void drawLine(bool drawWindow);
  void drawOneWholeFuckingLine();
  void flushDwordToBuffer(std::bitset<8> msb, std::bitset<8> lsb, int tileX);

  dword getTilemapBaseAddress(bool drawWindow) const;
  int getTilemapOffset(bool drawWindow, int tileX, int tileY) const;
  dword getTiledataBaseAddress(bool drawWindow) const;

 public:

  // Constructor ///////////////////////////////////////////////////////////////
  explicit Graphics(Memory* ram);
  //////////////////////////////////////////////////////////////////////////////

  int frameCount{0};

  void machineClock();

  PPUMode getPPUMode() const;

  void printStatus() const;
  void printBuffer() const;
  void printTileData() const;
  void printTileMap() const;

  word WY() const;
  word WX() const;
  word SCY() const;
  word SCX() const;
  word LY() const;

  bool isScreenOn() const;
};

} // namespace gb

#endif //GAMEBOY_H
