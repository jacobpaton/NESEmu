# NESEmu
Emulator for the Nintendo Entertainment System (NES) written by Jacob Paton.

This is the first emulator I've written and has imperfections. There are several things on my todo list that I would like to tackle with this project. As I continue working on it, I'll update this README with the project status. I plan to clean up the main loop to be contained with a NES class to allow for the emulator to be used as a library for a suite of emulators (which I may or may not create in the future).

## Features
- [ ] Instruction fetching/decoding/execution framework
- [ ] Official instruction set support
- [ ] Unofficial instructions support
- [ ] PPU Rendering
- [ ] PPU Scrolling
- [ ] APU

## Building

In case anyone who comes across this wants to build it themselves, the process is as follows from the root project directory.

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
