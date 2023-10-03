# Decimator

# Setup development environment

1. git clone https://github.com/electro-smith/libDaisy
2. git clone git clone https://github.com/electro-smith/DaisySP
3. brew install armmbed/formulae/arm-none-eabi-gcc

# Build

- `$(cd libDaisy && make)`
- `$(cd DaisySP && make)`
- replace `OBJECTS =` with `OBJECTS +=` in `libDaisy/core/Makefile` on line 259.
- in `libfmsymth/GNUmakefile`:
- copy `CPU`, `FPU`, `FLOAT_ABI` and `MCU` variables from `libDaisy/core/Makefile`
- comment the three lines inside `ifneq ($(findstring armv7,$(ARCH)),)`, and add two lines: `CFLAGS += $(MCU)` and `ASFLAGS += $(MCU)`.
- also, for debugging, inside `ifeq ($(DEBUG), 1)`, add `ASFLAGS += -g -ggdb`.
- `$(cd libfmsynth && make TOOLCHAIN_PREFIX=arm-none-eabi- ARCH=armv7 TUNE=cortex-m7)`

Explanation:

- The adopted `Decimator` Makefile which includes `libDaisy` works almost unmodified; the only
  change is allowing the Makefile to also compile `libfmsynth`'s object file (by making `OBJECTS` variable extendable).
- `libfmsynth` has ARM cross-compiling support with above command, but getting `ld` errors:
  "conflicting architecture profiles A/M", "failed to merge target specific data of file fmsynth.o",
  "PatchFemine.elf uses VFP register commands, fmsynth.o does not"

Looking at online discussions, `-mcpu`, `-mfpu` and `-mfloat-abi` are at fault and maybe must match.
https://stackoverflow.com/questions/9753749/arm-compilation-error-vfp-registers-used-by-executable-not-object-file

`libDaisy` uses:

```
CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
```

Looks like `libfmsynth` uses `NEON` aka Advanced SIMD implementation not available on Cortex M7.

So I modified the `libfmsynth` makefile to match (see `MCU` variable), and got a compiled version (30 Sept 2023) but not tested if it works on the hardware yet.

The files that I had to modify are also copied as `libDaisy-core-Makefile` and `libfmsynth-GNUmakefile`.

Read about [strategies for patching submodules](https://stackoverflow.com/a/57940839/324220).

(Unrelated note) For compiling for M1 mac targets:

- comment the line `CFLAGS += -march=native` in `libfmsynth/GNUmakefile` on line 60.

## Author

Combined into an Eurorack FM synth by Luka Ramishvili
libfmsynth code by Hans-Kristian Arntzen (Themeister)
libDaisy code by ElectroSmith
Decimator code by Ankoor Apte, Noise Engineering

## Description

Femine (FM synth) firmware for the Versio

## Usage

- If leds display OFF RED OFF RED, the FM synth failed to initialize.

### Source Code

- [Versio example](https://github.com/electro-smith/DaisyExamples/tree/master/versio/Decimator)
- [FM Synth](https://github.com/Themaister/libfmsynth)
- Initial code taken from `libfmsynth/lv2/fmsynth_lv2.cpp`

## Controls

All controls have some effect on the LEDs

| Control | Description               |
| ------- | ------------------------- |
| Knob 0  | Bitcrush factor (left)    |
| Knob 1  | Downsample factor (left)  |
| Knob 2  | Bitcrush factor (right)   |
| Knob 3  | Downsample factor (right) |

## Demo

TODO: Add demo
