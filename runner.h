//
// Created by zbl on 2022/6/21.
//

#ifndef RISCV_PROJECT_RUNNER_H
#define RISCV_PROJECT_RUNNER_H

#include "function_lib.h"

std::string get_ins(unsigned in){
    std::string res;
    int tmp[32] = {0}, cnt = 0;
    while(in){
        tmp[cnt++] = in & 1;
        in >>= 1;
    }
    for(int i = 31; i >= 0; --i)
        res += tmp[i] + '0';
    return res;
}

void decode(const std::string &ins, std::string &opCode, std::string &func3, std::string &func7){
    //opCode:31-6 -> 31-0
    //func3:31-14 -> 31-12
    //func7:31-0 -> 31-25
    for(int i = 31-6; i <= 31-0; ++i){
        opCode.push_back(ins[i]);
    }
    for(int i = 31-14; i <= 31-12; ++i){
        func3.push_back(ins[i]);
    }
    for(int i = 31-31; i <= 31-25; ++i){
        func7.push_back(ins[i]);
    }
}

void executer(const std::string &opCode, const std::string &func3, const std::string &func7, const std::string &ins, unsigned reg[], unsigned mem[], unsigned &pos){
    if(opCode == "0110111"){//LUI
        LUI(ins, reg, mem, pos);
    }
    else if(opCode == "0010111"){//AUIPC
        AUIPC(ins, reg, mem, pos);
    }
    else if(opCode == "1101111"){//JAL
        JAL(ins, reg, mem, pos);
    }
    else if(opCode == "1100111" && func3 == "000"){//JALR
        JALR(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "000"){//BEQ
        BEQ(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "001"){//BNE
        BNE(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "100"){//BLT
        BLT(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "101"){//BGE
        BGE(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "110") {//BLTU
        BLTU(ins, reg, mem, pos);
    }
    else if(opCode == "1100011" && func3 == "111"){//BGEU
        BGEU(ins, reg, mem, pos);
    }
    else if(opCode == "0000011" && func3 == "000"){//LB
        LB(ins, reg, mem, pos);
    }
    else if(opCode == "0000011" && func3 == "001"){//LH
        LH(ins, reg, mem, pos);
    }
    else if(opCode == "0000011" && func3 == "010"){//LW
        LW(ins, reg, mem, pos);
    }
    else if(opCode == "0000011" && func3 == "100"){//LBU
        LBU(ins, reg, mem, pos);
    }
    else if(opCode == "0000011" && func3 == "101"){//LHU
        LHU(ins, reg, mem, pos);
    }
    else if(opCode == "0100011" && func3 == "000"){//SB
        SB(ins, reg, mem, pos);
    }
    else if(opCode == "0100011" && func3 == "001"){//SH
        SH(ins, reg, mem, pos);
    }
    else if(opCode == "0100011" && func3 == "010"){//SW
        SW(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "000"){//ADDI
        ADDI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "010"){//SLTI
        SLTI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "011"){//SLTIU
        SLTIU(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "100"){//XORI
        XORI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "110"){//ORI
        ORI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "111"){//ANDI
        ANDI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "001" && func7 == "0000000"){//SLLI
        SLLI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "101" && func7 == "0000000"){//SRLI
        SRLI(ins, reg, mem, pos);
    }
    else if(opCode == "0010011" && func3 == "101" && func7 == "0100000"){//SRAI
        SRAI(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "000" && func7 == "0000000"){//ADD
        ADD(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "000" && func7 == "0100000"){//SUB
        SUB(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "001" && func7 == "0000000"){//SLL
        SLL(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "010" && func7 == "0000000"){//SLT
        SLT(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "011" && func7 == "0000000"){//SLTU
        SLTU(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "100" && func7 == "0000000"){//XOR
        XOR(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "101" && func7 == "0000000"){//SRL
        SRL(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "101" && func7 == "0100000"){//SRA
        SRA(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "110" && func7 == "0000000"){//OR
        OR(ins, reg, mem, pos);
    }
    else if(opCode == "0110011" && func3 == "111" && func7 == "0000000"){//AND
        AND(ins, reg, mem, pos);
    }
}


void run(unsigned pos, unsigned reg[], unsigned mem[]){
    int cnt = 0;
    while(true){
        //std::cout << ++cnt << " : " << std::endl;

        if(mem[pos] == (int)0x0ff00513) break;
        std::string ins = get_ins(mem[pos]);//取32位01字符串指令;
        std::string opCode, func3, func7;
        decode(ins, opCode, func3, func7);//对字符串指令解析,取出opcode,func3,func7;
        executer(opCode, func3, func7, ins, reg, mem, pos);//执行指令
        //std::cout << " " << reg[get_rd(ins, 31 - 19, 31 - 15)] << std::endl;
        reg[0] = 0;


        /*
        if(cnt >= 1 && cnt <= 5000){

        for(int i = 0; i < 31; ++i){
            std::cout << i << " : " << reg[i] << " ";
        }



        for(int i = 0; i <= 3000; ++i){
            std::cout << mem[i] << " ";
        }
        std::cout << std::endl;



        std::cout << pos << std::endl;

        }
         */


        }

    printf("%u\n",reg[10] & 255u);
}

#endif //RISCV_PROJECT_RUNNER_H
