# Displaytronic VS2812 Driver Board

[![CI](https://github.com/Qeteshpony/displaytronic/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/Qeteshpony/displaytronic/actions/workflows/ci.yml)

Replacement driver board and firmware for the Displaytronic VS2812 flipdot display (28×12 dots).

![3D Render](https://qeteshpony.github.io/displaytronic/3D/displaytronic-3D_top.png)

> **Status:** Work in progress – hardware not yet built or tested.

## Contents

| Path | Description |
|---|---|
| [`pcb/`](pcb/) | KiCad project of the driver board – see [Hardware Documentation](https://qeteshpony.github.io/displaytronic) |
| [`firmware/`](firmware/) | ATmega328P firmware (PlatformIO) – see [firmware README](firmware/README.md) |
| [`flipdot board/`](flipdot%20board/) | Reverse engineered schematics of the display itself |

Prebuilt firmware and production files are attached to each [release](https://github.com/Qeteshpony/displaytronic/releases). Changes are listed in the [changelog](CHANGELOG.md).

## License

- Hardware: [CERN-OHL-P-2.0](LICENSE-CERN-OHL-P-2.0)
- Firmware: [BSD-2-Clause](LICENSE-BSD-2-Clause)
