class MOS6502 {
    public:
        MOS6502() {
            memory = malloc(this.memSize * sizeof(int8_t))
        }

    private:
        int memSize = 2048;
        int* memory;
}
