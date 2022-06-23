//
// Created by zbl on 2022/6/21.
//

#ifndef RISCV_PROJECT_FUNCTION_LIB_H
#define RISCV_PROJECT_FUNCTION_LIB_H

unsigned get_imm(const std::string &ins, int l, int r){
    unsigned res = 0;
    for(int i = l; i <= r; ++i){
        res = (res << 1) + ins[i] - '0';
    }
    return res;
}

unsigned get_rd(const std::string &ins, int l, int r){
    unsigned res = 0;
    for(int i = l; i <= r; ++i){
        res = (res << 1) + ins[i] - '0';
    }
    return res;
}

unsigned read_one_char(int index, unsigned mem[]){//在index位读一个字节
    int memPos = index >> 2;
    int del = index - (memPos << 2);
    unsigned res = 0;
    if(del == 0){
        res = mem[memPos] & 0xff;
    }
    else if(del == 1){
        res = (mem[memPos] & 0xff00) >> 8;
    }
    else if(del == 2){
        res = (mem[memPos] & 0xff0000) >> 16;
    }
    else if(del == 3){
        res = (mem[memPos] & 0xff000000) >> 24;
    }
    return res;
}

unsigned read_two_char(int index, unsigned mem[]){
    unsigned res = read_one_char(index, mem);
    res += (read_one_char(index + 1, mem) << 8);
    return res;
}

unsigned read_four_char(int index, unsigned mem[]){
    unsigned res = read_two_char(index, mem);
    res += (read_two_char(index + 2, mem) << 16);
    return res;
}

void write_one_char(unsigned aimChar, int index, unsigned mem[]){
    int memPos = index >> 2;
    int del = index - (memPos << 2);
    if(del == 0){
        mem[memPos] &= 0xffffff00;
        mem[memPos] |= aimChar;
    }
    else if(del == 1){
        mem[memPos] &= 0xffff00ff;
        aimChar <<= 8;
        mem[memPos] |= aimChar;
    }
    else if(del == 2){
        mem[memPos] &= 0xff00ffff;
        aimChar <<= 16;
        mem[memPos] |= aimChar;
    }
    else if(del == 3){
        mem[memPos] &= 0xffffff;
        aimChar <<= 24;
        mem[memPos] |= aimChar;
    }
}

void write_two_char(unsigned aimChar, int index, unsigned mem[]){
    write_one_char(aimChar & 0xff, index, mem);//低位开始写
    write_one_char((aimChar & 0xff00) >> 8, index + 1, mem);//然后是高位
}

void write_four_char(unsigned aimChar, int index, unsigned mem[]){
    write_two_char(aimChar & 0xffff, index, mem);
    write_two_char((aimChar & 0xffff0000) >> 16, index + 2, mem);
}

inline unsigned sign_extend(const unsigned &x, const int &cur_len, const int &dest_len = 32) {
    unsigned t = -1; // 1111...
    t = (t >> (cur_len - 1) ) << (cur_len - 1); // 3 5
    if (x >> (cur_len - 1)) return x | t;
    else return x;
}





void LUI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned imm = get_imm(ins, 31 - 31, 31 - 12), rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = imm << 12;
    ++pos;
}

void AUIPC(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned imm = get_imm(ins, 31 - 31, 31 - 12), rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = (imm << 12) + (pos << 2);
    ++pos;
}

void JAL(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7), off = get_imm(ins, 0, 0);
    off = (off << 8) + get_imm(ins, 31 - 19,  31- 12);
    off = (off << 1) + get_imm(ins, 31 - 20, 31 - 20);
    off = (off << 10) + get_imm(ins, 1, 10);
    off <<= 1;
    reg[rdx] = (pos + 1) << 2;
    off = sign_extend(off, 21);
    pos = ((pos << 2) + off) >> 2;
}

void JALR(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned t = (pos + 1) << 2, off = get_imm(ins, 0, 11);
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    pos = ((reg[rdx1] + off) & (~1)) >> 2;
    reg[rdx] = t;
}

void BEQ(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(reg[rdx1] == reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void BNE(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(reg[rdx1] != reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void BLT(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if((int)reg[rdx1] < (int)reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void BGE(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if((int)reg[rdx1] >= (int)reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void BLTU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(reg[rdx1] < reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void BGEU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx2 = get_rd(ins, 31 - 24, 31 - 20), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(reg[rdx1] >= reg[rdx2]){
        unsigned off = get_imm(ins, 0, 0);
        off = (off << 1) + get_imm(ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(ins, 1, 6);
        off = (off << 4) + get_imm(ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        pos = ((pos << 2) + off) >> 2;
    }
    else{
        ++pos;
    }
}

void LB(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(reg[rdx1] + off, mem);//8位
    if(rans >> 7) rans |= 0xffffff00;
    reg[rdx] = rans;
    ++pos;
}

void LH(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(reg[rdx1] + off, mem);//16位
    if(rans >> 15) rans |= 0xffff0000;
    reg[rdx] = rans;
    ++pos;
}

void LW(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_four_char(reg[rdx1] + off, mem);//32位
    reg[rdx] = rans;
    ++pos;
}

void LBU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(reg[rdx1] + off, mem);//8位
    //rans <<= 24;
    reg[rdx] = rans;
    ++pos;
}

void LHU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(reg[rdx1] + off, mem);//16位
    //rans <<= 16;
    reg[rdx] = rans;
    ++pos;
}

void SB(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned aimChar = reg[rdx2] & 0xff;
    unsigned off = get_imm(ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_one_char(aimChar, reg[rdx1] + off, mem);
    ++pos;
}

void SH(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned aimChar = reg[rdx2] & 0xffff;
    unsigned off = get_imm(ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_two_char(aimChar, reg[rdx1] + off, mem);
    ++pos;
}

void SW(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned aimChar = reg[rdx2] & 0xffffffff;
    unsigned off = get_imm(ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_four_char(aimChar, reg[rdx1] + off, mem);
    ++pos;
}

void ADDI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(ins, 0, 11);//12位
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = imm + reg[rdx1];
    ++pos;
}

void SLTI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned imm = get_imm(ins, 0, 11);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = ((int)reg[rdx1] < (int)imm);
    ++pos;
}

void SLTIU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned imm = get_imm(ins, 0, 11);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7), rdx1 = get_rd(ins, 31 - 19, 31 - 15);
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = (reg[rdx1] < imm);
    ++pos;
}

void XORI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = reg[rdx1] ^ imm;
    ++pos;
}

void ORI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = reg[rdx1] | imm;
    ++pos;
}

void ANDI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    reg[rdx] = reg[rdx1] & imm;
    ++pos;
}

void SLLI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){////shamt[5] == 0?
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(ins, 31 - 25, 31 - 20);
    reg[rdx] = reg[rdx1] << shamt;
    ++pos;
}

void SRLI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(ins, 31 - 25, 31 - 20);
    reg[rdx] = reg[rdx1] >> shamt;
    ++pos;
}

void SRAI(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx = get_rd(ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(ins, 31 - 25, 31 - 20);
    reg[rdx] = (int)reg[rdx1] >> shamt;
    ++pos;
}

void ADD(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] + reg[rdx2];
    ++pos;
}

void SUB(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] - reg[rdx2];
    ++pos;
}

void SLL(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] << (reg[rdx2] & 0x1f);
    ++pos;
}

void SLT(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = (int)reg[rdx1] < (int)reg[rdx2] ? 1 : 0;
    ++pos;
}

void SLTU(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] < reg[rdx2] ? 1 : 0;
    ++pos;
}

void XOR(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] ^ reg[rdx2];
    ++pos;
}

void SRL(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] >> (reg[rdx2] & 0x1f);
    ++pos;
}

void SRA(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = (int)reg[rdx1] >> (reg[rdx2] & 0x1f);
    ++pos;
}

void OR(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] | reg[rdx2];
    ++pos;
}

void AND(const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    unsigned rdx1 = get_rd(ins, 31 - 19, 31 - 15), rdx2 = get_rd(ins, 31 - 24, 31 - 20);
    unsigned rdx = get_rd(ins, 31 - 11, 31 - 7);
    reg[rdx] = reg[rdx1] & reg[rdx2];
    ++pos;
}


#endif //RISCV_PROJECT_FUNCTION_LIB_H
