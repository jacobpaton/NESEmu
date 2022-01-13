# NESEmu
Emulator for the Nintendo Entertainment System (NES) written by Jacob Paton.

This is the first emulator I've written and has many imperfections. There are several things on my todo list that I would like to tackle with this project. As I continue working on it, I'll update this README with the project status.

## Features
- [x] Instruction fetching/decoding/execution framework
- [x] Official instruction set support
- [ ] Full address mode support
- [ ] Unofficial instructions support
- [ ] PPU Rendering
- [ ] PPU Scrolling
- [ ] APU
- [ ] Mappers

## Building

In case anyone who comes across this wants to build it themselves, the process is standard cmake as follows from the root project directory.

```
mkdir build
cd build
cmake ..
make
./NESEmu
```

## Helpful Resources
- https://wiki.nesdev.org/
- https://wiki.nesdev.org/w/index.php/Emulator_tests
- http://users.telenet.be/kim1-6502/6502/proman.html#221
- https://yizhang82.dev/nes-emu-cpu
- https://austinmorlan.com/posts/nes_rendering_overview/
