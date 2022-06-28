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





void LUI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 31 - 31, 31 - 12);
    o.rd = imm << 12;
}

void AUIPC(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 31 - 31, 31 - 12);
    o.rd = (imm << 12) + (o.pc << 2);
}

void JAL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 0);
    off = (off << 8) + get_imm(o.ins, 31 - 19,  31- 12);
    off = (off << 1) + get_imm(o.ins, 31 - 20, 31 - 20);
    off = (off << 10) + get_imm(o.ins, 1, 10);
    off <<= 1;
    o.rd = (o.pc + 1) << 2;
    off = sign_extend(off, 21);
    nxt_pos = ((o.pc << 2) + off) >> 2;
}

void JALR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned t = (o.pc + 1) << 2, off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    nxt_pos = ((o.rd1 + off) & (~1)) >> 2;
    o.rd = t;
}

void BEQ(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if(o.rd1 == o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void BNE(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if(o.rd1 != o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void BLT(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if((int)o.rd1 < (int)o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void BGE(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if((int)o.rd1 >= (int)o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void BLTU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if(o.rd1 < o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void BGEU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    if(o.rd1 >= o.rd2){
        unsigned off = get_imm(o.ins, 0, 0);
        off = (off << 1) + get_imm(o.ins, 31 - 7, 31 - 7);
        off = (off << 6) + get_imm(o.ins, 1, 6);
        off = (off << 4) + get_imm(o.ins, 31 - 11, 31 - 8);
        off <<= 1;
        if(off >> 12) off |= 0xffffe000;
        nxt_pos = ((o.pc << 2) + off) >> 2;
    }
}

void LB(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(o.rd1 + off, mem);//8位
    if(rans >> 7) rans |= 0xffffff00;
    o.rd = rans;
}

void LH(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(o.rd1 + off, mem);//16位
    if(rans >> 15) rans |= 0xffff0000;
    o.rd = rans;
}

void LW(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_four_char(o.rd1 + off, mem);//32位
    o.rd = rans;
}

void LBU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(o.rd1 + off, mem);//8位
    //rans <<= 24;
    o.rd = rans;
}

void LHU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(o.rd1 + off, mem);//16位
    //rans <<= 16;
    o.rd = rans;
}

void SB(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    unsigned aimChar = o.rd2 & 0xff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_one_char(aimChar, o.rd1 + off, mem);
}

void SH(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    unsigned aimChar = o.rd2 & 0xffff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_two_char(aimChar, o.rd1 + off, mem);
}

void SW(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    unsigned aimChar = o.rd2 & 0xffffffff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_four_char(aimChar, o.rd1 + off, mem);
}

void ADDI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(o.ins, 0, 11);//12位
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = imm + o.rd1;
}

void SLTI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7), rdx1 = get_rd(o.ins, 31 - 19, 31 - 15);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = ((int)o.rd1 < (int)imm);
}

void SLTIU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7), rdx1 = get_rd(o.ins, 31 - 19, 31 - 15);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = (o.rd1 < imm);
}

void XORI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 ^ imm;
}

void ORI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 | imm;
}

void ANDI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 & imm;
}

void SLLI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){////shamt[5] == 0?
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = o.rd1 << shamt;
}

void SRLI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = o.rd1 >> shamt;
}

void SRAI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = (int)o.rd1 >> shamt;
}

void ADD(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 + o.rd2;
}

void SUB(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 - o.rd2;
}

void SLL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 << (o.rd2 & 0x1f);
}

void SLT(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = (int)o.rd1 < (int)o.rd2 ? 1 : 0;
}

void SLTU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 < o.rd2 ? 1 : 0;
}

void XOR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 ^ o.rd2;
}

void SRL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 >> (o.rd2 & 0x1f);
}

void SRA(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = (int)o.rd1 >> (o.rd2 & 0x1f);
}

void OR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 | o.rd2;
}

void AND(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    //unsigned rdx1 = get_rd(o.ins, 31 - 19, 31 - 15), rdx2 = get_rd(o.ins, 31 - 24, 31 - 20);
    //unsigned rdx = get_rd(o.ins, 31 - 11, 31 - 7);
    o.rd = o.rd1 & o.rd2;
}


#endif //RISCV_PROJECT_FUNCTION_LIB_H
