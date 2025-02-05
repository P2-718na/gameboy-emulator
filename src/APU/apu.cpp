#include "apu.hpp"
#include <bitset>
#include <cassert>
#include <iostream>
#include <ostream>

#include "gbapu.hpp"
#include "memory.hpp"
#include <SFML/Audio.hpp>
// todo decouple sfml from this ^^

constexpr unsigned SAMPLERATE = 48000;
constexpr double CYCLES_PER_SAMPLE = 4194304.0 / SAMPLERATE;
constexpr double CYCLES_PER_FRAME = 4194304.0 / 59.7;

using namespace gbapu;
using Clock = std::chrono::steady_clock;

namespace gb {

static constexpr uint8_t HOLD = 0x00;
struct DemoCommand {

  uint8_t reg;
  uint8_t value;
};

static DemoCommand const DEMO_ENVELOPE[] = {
  {Apu::REG_NR52, 0x80}, {Apu::REG_NR50, 0x77}, {Apu::REG_NR51, 0x11},
  {Apu::REG_NR12, 0xF7}, {Apu::REG_NR14, 0x87}, {HOLD, 120},
  {Apu::REG_NR12, 0xF6}, {Apu::REG_NR14, 0x87}, {HOLD, 90},
  {Apu::REG_NR12, 0xF5}, {Apu::REG_NR14, 0x87}, {HOLD, 70},
  {Apu::REG_NR12, 0xF4}, {Apu::REG_NR14, 0x87}, {HOLD, 60},
  {Apu::REG_NR12, 0xF3}, {Apu::REG_NR14, 0x87}, {HOLD, 50},
  {Apu::REG_NR12, 0xF2}, {Apu::REG_NR14, 0x87}, {HOLD, 40},
  {Apu::REG_NR12, 0xF1}, {Apu::REG_NR14, 0x87}, {HOLD, 30},
  {Apu::REG_NR14, 0x87}, {HOLD, 10},
  {Apu::REG_NR14, 0x87}, {HOLD, 10},
  {Apu::REG_NR14, 0x87}, {HOLD, 10},
  {Apu::REG_NR14, 0x87}, {HOLD, 10}
};

struct Demo {
  const char *name;
  DemoCommand const *sequence;
  size_t sequenceLen;
};

#define demoStruct(name, seq) { name, seq, sizeof(seq) / sizeof(DemoCommand) }

static Demo const DEMO_TABLE[] = {
  demoStruct("envelope", DEMO_ENVELOPE)
};



APU::APU() {
  sf::SoundBuffer buffer;

  Apu apu(SAMPLERATE, SAMPLERATE / 10);
  //apu.setVolume(0.6f);

  constexpr size_t samplesPerFrame = (CYCLES_PER_FRAME / CYCLES_PER_SAMPLE) + 1;
  auto frameBuf = std::make_unique<float[]>(samplesPerFrame * 2);


  //auto timeStart = std::chrono::steady_clock::now();

  Clock::duration minTime(Clock::duration::max()), maxTime(0);
  Clock::duration elapsed(0);
  size_t frameCounter = 0;

  auto &demo = DEMO_TABLE[0];

  apu.reset();
  apu.clearSamples();

  uint32_t cycles = 0;
  for (size_t j = 0; j != demo.sequenceLen; ++j) {
    auto cmd = demo.sequence[j];
    if (cmd.reg == HOLD) {
      for (size_t frames = cmd.value; frames--; ) {
        auto now = Clock::now();
        apu.step(CYCLES_PER_FRAME - cycles);
        auto frameTime = Clock::now() - now;
        if (frameTime < minTime) {
          minTime = frameTime;
        }
        if (frameTime > maxTime) {
          maxTime = frameTime;
        }
        ++frameCounter;
        elapsed += frameTime;

        apu.endFrame();
        size_t samples = apu.availableSamples();

        apu.readSamples(frameBuf.get(), samples);

        std::vector<sf::Int16> frameBuff(samplesPerFrame*2);
        // step through each element of integer array, and copy into float array as float
        for(int i = 0; i < samplesPerFrame*2; ++i) {
          //adding 1.0 to the values, multiply by 32767.0, convert to unsigned integer, then subtract 32767.
          frameBuff[i] = (sf::Int16)((frameBuf.get()[i+samples])*32767.0);
          //printf("%i, %f\n", frameBuff[i], (1. + frameBuf.get()[i]));
        }

        buffer.loadFromSamples(&frameBuff[0], samplesPerFrame, 2, SAMPLERATE);



        sf::Sound sound(buffer);
        sound.play();
        //wav.write(frameBuf.get(), samples);

      }

    } else {
      cycles += 12;
      apu.writeRegister(static_cast<Apu::Reg>(cmd.reg), cmd.value);
    }



    cycles = 0;

  }

  std::cout << "Time elapsed: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
            << " ms" << std::endl;
  std::cout << "Minimum: "
            << std::chrono::duration_cast<std::chrono::microseconds>(minTime).count()
            << " us" << std::endl;
  std::cout << "Maximum: "
            << std::chrono::duration_cast<std::chrono::microseconds>(maxTime).count()
            << " us" << std::endl;
  std::cout << "Average: "
            << std::chrono::duration_cast<std::chrono::microseconds>(elapsed / frameCounter).count()
            << " us" << std::endl;

}

} // namespace gb