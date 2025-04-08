#include "ppu.hpp"
#include "address-bus.hpp"
#include "doctest.h"
#include "gameboy.hpp"
#include "types.hpp"

using namespace gb;

TEST_CASE("PPU Basic Operations") {
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };  // Create with empty ROM
  AddressBus bus{ &gameboy };
  PPU ppu{ &gameboy, &bus };

  SUBCASE("Initial State") {
    // Check initial PPU mode
    CHECK_EQ(ppu.getPPUMode(), PPU::PPU_MODE::OAM_SCAN);

    // Check initial frame count
    CHECK_EQ(ppu.frameCount, 0);

    // Check initial LCD control register flags
    CHECK_FALSE(ppu.LCDC(PPU::LCDC_BIT::LCD_DISPLAY_ENABLE));
    CHECK_FALSE(ppu.LCDC(PPU::LCDC_BIT::WINDOW_DISPLAY_ENABLE));
    CHECK_FALSE(ppu.LCDC(PPU::LCDC_BIT::SPRITE_ENABLE));
  }
}

TEST_CASE("PPU Scroll Registers") {
  Gameboy gameboy{std::vector<word>(0x8000, 0) };
  AddressBus bus{ &gameboy };
  PPU ppu{ &gameboy, &bus };

  SUBCASE("Scroll Registers Initial Values") {
    CHECK_EQ(ppu.SCX(), 0);  // Scroll X should start at 0
    CHECK_EQ(ppu.SCY(), 0);  // Scroll Y should start at 0
    CHECK_EQ(ppu.WX(), 0);   // Window X should start at 0
    CHECK_EQ(ppu.WY(), 0);   // Window Y should start at 0
  }

  SUBCASE("Line Counter") {
    CHECK_EQ(ppu.LY(), 0);  // Line counter should start at 0
  }
}

TEST_CASE("PPU Machine Clock") {
  Gameboy gameboy{ std::vector<word>(0x8000, 0) };
  AddressBus bus{ &gameboy };
  PPU ppu{ &gameboy, &bus };

  SUBCASE("Mode Transitions") {
    // Start in OAM scan mode
    CHECK_EQ(ppu.getPPUMode(), PPU::PPU_MODE::OAM_SCAN);

    // Run enough cycles to transition through modes
    for (int i = 0; i != 20; ++i) {  // OAM scan takes 20 machine cycles
      ppu.machineClock();
    }
    CHECK_EQ(ppu.getPPUMode(), PPU::PPU_MODE::DRAWING);

    for (int i = 0; i != 73; ++i) {  // Drawing takes at most 73 M-cycles
      ppu.machineClock();
    }
    CHECK_EQ(ppu.getPPUMode(), PPU::PPU_MODE::H_BLANK);

    // After all the lines have been drawn, we switch do VBLANK.
    // Each line takes 114 M-cycles
    for (int i = 0; i != 21 + 114 * 143; ++i) {
      ppu.machineClock();
    }
    CHECK_EQ(ppu.getPPUMode(), PPU::PPU_MODE::V_BLANK);
  }
}

// TODO More PPU testing should be done by using test ROMs