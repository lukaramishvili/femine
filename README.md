# Femine (FM Synth)

# Compilation, source, development machine setup

see [./COMPILING.md](./COMPILING.md)

## Author

Combined into an Eurorack FM synth by Luka Ramishvili
libfmsynth code by Hans-Kristian Arntzen (Themeister)
libDaisy code by ElectroSmith
Decimator code by Ankoor Apte, Noise Engineering

## Description

FM synth firmware for the Versio

## Usage

- If leds display OFF RED OFF RED, the FM synth failed to initialize.

## Controls

All controls have some effect on the LEDs

| Control | Description               |
| ------- | ------------------------- |
| Knob 0  | Bitcrush factor (left)    |
| Knob 1  | Downsample factor (left)  |
| Knob 2  | Bitcrush factor (right)   |
| Knob 3  | Downsample factor (right) |

| Pin Name | Pin Location | Function        | Comment                          |
| -------- | ------------ | --------------- | -------------------------------- |
| CV_1     | C5           | Coarse Tuning   | Sets the pitch of the oscillator |
| CV_5     | C6           | 1V/Octave Input | Musical pitch tracking input     |

## Demo

TODO: Add demo
