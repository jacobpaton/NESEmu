#include <iostream>
#include "nes.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "ROM file must be given as argument" << std::endl;
        return -1;
    }

    NES nes(argv[1]);
    nes.run();

    return 0;
}
