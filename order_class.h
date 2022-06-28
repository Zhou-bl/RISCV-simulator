//
// Created by zbl on 2022/6/28.
//

#ifndef RISCV_PROJECT_ORDER_CLASS_H
#define RISCV_PROJECT_ORDER_CLASS_H

//数据冒险类型:
//1.WB和ID之间的reg寄存器的数据冲突
//2.MEM和IF之间的mem内存的数据冲突

unsigned reg[32];
unsigned next_pos = 0;
unsigned mem[500000];

class order{
public:
    //约定:next_step=None表示尚未进行初始化
    //ins="Pause"表示插入暂停;Pause指令要执行完五个步骤,但是None指令不进行执行,退出后直接从流水中踢出;
    enum next_step{None, IF, ID, EXE, MEM, WB};//Node代表尚未进行初始化;
    std::string ins;//指令
    std::string func3, func7, opCode;
    next_step nxt;
    int pc, aimRd;
    bool need_MEM = false;
    bool need_WB = false;
    unsigned rd1, rd2, rd; //分别表示reg[rdx1],reg[rdx2],reg[rdx]的值;
    order():nxt(None), ins(""), pc(0){}

    order(int tmpPC){
        pc = tmpPC;
        nxt = IF;
    }

    order(const std::string &str){
        ins = str;
    }
    
    unsigned Get_Num(int l, int r){
        unsigned res = 0;
        for(int i = l; i <= r; ++i){
            res = (res << 1) + ins[i] - '0';
        }
        return res;
    }
    
    bool order_IF();
    
    bool order_ID();
    
    bool order_EXE();
    
    bool order_MEM();
    
    bool order_WB();
};

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


////分支跳转指令---------------------------------------------------------------------------------------------------------
////共 8 条-------------------------------------------------------------------------------------------------------------------
void JAL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 0);
    off = (off << 8) + get_imm(o.ins, 31 - 19,  31- 12);
    off = (off << 1) + get_imm(o.ins, 31 - 20, 31 - 20);
    off = (off << 10) + get_imm(o.ins, 1, 10);
    off <<= 1;
    o.rd = (o.pc + 1) << 2;
    o.need_WB = true;
    off = sign_extend(off, 21);
    nxt_pos = ((o.pc << 2) + off) >> 2;
}

void JALR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned t = (o.pc + 1) << 2, off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    nxt_pos = ((o.rd1 + off) & (~1)) >> 2;
    o.rd = t;
    o.need_WB = true;
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


////Load & Store指令-----------------------------------------------------------------------------------------------------
////共 8 条---------------------------------------------------------------------------------------------------------------------

////inner用于MEM操作:
void LB_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(o.rd1 + off, mem);//8位
    if(rans >> 7) rans |= 0xffffff00;
    o.rd = rans;
    o.need_WB = true;
}

void LH_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(o.rd1 + off, mem);//16位
    if(rans >> 15) rans |= 0xffff0000;
    o.rd = rans;
    o.need_WB = true;
}

void LW_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_four_char(o.rd1 + off, mem);//32位
    o.rd = rans;
    o.need_WB = true;
}

void LBU_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_one_char(o.rd1 + off, mem);//8位
    o.rd = rans;
    o.need_WB = true;
}

void LHU_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned off = get_imm(o.ins, 0, 11);
    if(off >> 11) off |= 0xfffff000;
    unsigned rans = read_two_char(o.rd1 + off, mem);//16位
    o.rd = rans;
    o.need_WB = true;
}

void SB_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned aimChar = o.rd2 & 0xff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_one_char(aimChar, o.rd1 + off, mem);
}

void SH_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned aimChar = o.rd2 & 0xffff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_two_char(aimChar, o.rd1 + off, mem);
}

void SW_inner(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned aimChar = o.rd2 & 0xffffffff;
    unsigned off = get_imm(o.ins, 31 - 31, 31 - 25);
    off <<= 5;
    off += get_imm(o.ins, 31 - 11, 31 - 7);
    if(off >> 11) off |= 0xfffff000;
    write_four_char(aimChar, o.rd1 + off, mem);
}


////算术指令--------------------------------------------------------------------------------------------------------------
////共 21 条--------------------------------------------------------------------------------------------------------------------
void LUI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 31 - 31, 31 - 12);
    o.rd = imm << 12;
    o.need_WB = true;
}

void AUIPC(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 31 - 31, 31 - 12);
    o.rd = (imm << 12) + (o.pc << 2);
    o.need_WB = true;
}


void ADDI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);//12位
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = imm + o.rd1;
    o.need_WB = true;
}

void SLTI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = ((int)o.rd1 < (int)imm);
    o.need_WB = true;
}

void SLTIU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = (o.rd1 < imm);
    o.need_WB = true;
}

void XORI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 ^ imm;
    o.need_WB = true;
}

void ORI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 | imm;
    o.need_WB = true;
}

void ANDI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned imm = get_imm(o.ins, 0, 11);
    if(imm >> 11) imm |= 0xfffff000;
    o.rd = o.rd1 & imm;
    o.need_WB = true;
}

void SLLI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){////shamt[5] == 0?
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = o.rd1 << shamt;
    o.need_WB = true;
}

void SRLI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = o.rd1 >> shamt;
    o.need_WB = true;
}

void SRAI(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    unsigned shamt = get_imm(o.ins, 31 - 25, 31 - 20);
    o.rd = (int)o.rd1 >> shamt;
    o.need_WB = true;
}

void ADD(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 + o.rd2;
    o.need_WB = true;
}

void SUB(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 - o.rd2;
    o.need_WB = true;
}

void SLL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 << (o.rd2 & 0x1f);
    o.need_WB = true;
}

void SLT(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = (int)o.rd1 < (int)o.rd2 ? 1 : 0;
    o.need_WB = true;
}

void SLTU(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 < o.rd2 ? 1 : 0;
    o.need_WB = true;
}

void XOR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 ^ o.rd2;
    o.need_WB = true;
}

void SRL(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 >> (o.rd2 & 0x1f);
    o.need_WB = true;
}

void SRA(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = (int)o.rd1 >> (o.rd2 & 0x1f);
    o.need_WB = true;
}

void OR(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 | o.rd2;
    o.need_WB = true;
}

void AND(unsigned reg[], unsigned mem[], order &o, unsigned &nxt_pos){
    o.rd = o.rd1 & o.rd2;
    o.need_WB = true;
}




bool order::order_IF(){
    if(nxt != IF){//不合法的步骤;
        return false;
    }
    if(ins == "Pause"){//暂停步骤,视为成功执行;
        nxt = ID;
        next_pos++;
        return true;
    }
    std::string res;
    int tmp[32] = {0}, cnt = 0;
    unsigned in = mem[pc];
    while(in){
        tmp[cnt++] = in & 1;
        in >>= 1;
    }
    for(int i = 31; i >= 0; --i)
        res += tmp[i] + '0';
    ins = res;
    nxt = ID;
    next_pos++;
    return true;
}

bool order::order_ID(){//取出操作码,reg1,reg2,reg,func3,func7;
    if(nxt != ID){
        return false;
    }
    if(ins == "Pause"){
        nxt = EXE;
        return true;
    }
    for(int i = 31-6; i <= 31-0; ++i){
        opCode.push_back(ins[i]);
    }
    for(int i = 31-14; i <= 31-12; ++i){
        func3.push_back(ins[i]);
    }
    for(int i = 31-31; i <= 31-25; ++i){
        func7.push_back(ins[i]);
    }

    int rs1, rs2;
    rs1 = Get_Num(31 - 19, 31 - 15), rs2 = Get_Num(31 - 24, 31 - 20), aimRd = Get_Num(31 - 11, 31 - 7);
    rd1 = reg[rs1], rd2 = reg[rs2], rd = reg[aimRd];

    nxt = EXE;
    return true;
}

bool order::order_EXE(){
    if(nxt != EXE){
        return false;
    }
    if(ins == "Pause"){
        nxt = MEM;
        return true;
    }
    if(opCode == "0110111"){//LUI
        LUI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010111"){//AUIPC
        AUIPC(reg, mem, *this, next_pos);
    }
    else if(opCode == "1101111"){//JAL
        JAL(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100111" && func3 == "000"){//JALR
        JALR(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "000"){//BEQ
        BEQ(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "001"){//BNE
        BNE(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "100"){//BLT
        BLT(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "101"){//BGE
        BGE(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "110") {//BLTU
        BLTU(reg, mem, *this, next_pos);
    }
    else if(opCode == "1100011" && func3 == "111"){//BGEU
        BGEU(reg, mem, *this, next_pos);
    }
    else if(opCode == "0000011" && func3 == "000"){//LB
        //LB(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0000011" && func3 == "001"){//LH
        //LH(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0000011" && func3 == "010"){//LW
        //LW(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0000011" && func3 == "100"){//LBU
        //LBU(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0000011" && func3 == "101"){//LHU
        //LHU(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0100011" && func3 == "000"){//SB
        //SB(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0100011" && func3 == "001"){//SH
        //SH(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0100011" && func3 == "010"){//SW
        //SW(reg, mem, *this, next_pos);
        need_MEM = true;
    }
    else if(opCode == "0010011" && func3 == "000"){//ADDI
        ADDI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "010"){//SLTI
        SLTI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "011"){//SLTIU
        SLTIU(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "100"){//XORI
        XORI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "110"){//ORI
        ORI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "111"){//ANDI
        ANDI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "001" && func7 == "0000000"){//SLLI
        SLLI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "101" && func7 == "0000000"){//SRLI
        SRLI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0010011" && func3 == "101" && func7 == "0100000"){//SRAI
        SRAI(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "000" && func7 == "0000000"){//ADD
        ADD(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "000" && func7 == "0100000"){//SUB
        SUB(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "001" && func7 == "0000000"){//SLL
        SLL(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "010" && func7 == "0000000"){//SLT
        SLT(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "011" && func7 == "0000000"){//SLTU
        SLTU(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "100" && func7 == "0000000"){//XOR
        XOR(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "101" && func7 == "0000000"){//SRL
        SRL(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "101" && func7 == "0100000"){//SRA
        SRA(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "110" && func7 == "0000000"){//OR
        OR(reg, mem, *this, next_pos);
    }
    else if(opCode == "0110011" && func3 == "111" && func7 == "0000000"){//AND
        AND(reg, mem, *this, next_pos);
    }

    nxt = MEM;
    return true;
}

bool order::order_MEM(){
    if(nxt != MEM){
        return false;
    }
    if(ins == "Pause"){
        nxt = WB;
        return true;
    }
    if(!need_MEM){
        nxt = WB;
        return true;
    }

    if(opCode == "0000011" && func3 == "000"){//LB
        LB_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0000011" && func3 == "001"){//LH
        LH_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0000011" && func3 == "010"){//LW
        LW_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0000011" && func3 == "100"){//LBU
        LBU_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0000011" && func3 == "101"){//LHU
        LHU_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0100011" && func3 == "000"){//SB
        SB_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0100011" && func3 == "001"){//SH
        SH_inner(reg, mem, *this, next_pos);
    }
    else if(opCode == "0100011" && func3 == "010"){//SW
        SW_inner(reg, mem, *this, next_pos);
    }
    nxt = WB;
    return true;
}

bool order::order_WB(){
    if(nxt != WB){
        return false;
    }
    if(ins == "Pause"){
        return true;
    }
    if(!need_WB){
        return true;
    }
    reg[aimRd] = rd;
    return true;
}

#endif //RISCV_PROJECT_ORDER_CLASS_H
