//
// Created by zbl on 2022/6/21.
//

#ifndef RISCV_PROJECT_GET_CODE_H
#define RISCV_PROJECT_GET_CODE_H

#include <string>

int char16_to_int(char c){
    if(c >= '0' && c <= '9')
        return c - '0';
    else return c - 'A' + 10;
}

unsigned get_new_pos(const std::string &str){
    unsigned res = 0;
    for(int i = 1; i < str.length(); ++i){
        res = (res << 4) + char16_to_int(str[i]);
    }
    return res >> 2;
}

unsigned get_next_op(const std::string &str, int &pos){
    char op16[4][2];
    int curOp = 0;
    unsigned res = 0;
    for(; curOp < 4; ++pos){
        op16[curOp][0] = str[pos++];
        op16[curOp++][1] = str[pos++];
    }
    for(int i = 3; i >= 0; --i){
        res = (res << 4) + char16_to_int(op16[i][0]);
        res = (res << 4) + char16_to_int(op16[i][1]);
    }
    return res;
}

void read_into_mem(unsigned mem[]){
    std::string command;
    int mem_pos, str_pos;
    while(std::getline(std::cin, command)){
        if(command[0] == '@'){//新程序的开始标志
            mem_pos = get_new_pos(command);
            continue;
        }
        else{
            str_pos = 0;
            while(str_pos < command.length()){
                mem[mem_pos++] = get_next_op(command, str_pos);
            }
        }
    }
}

#endif //RISCV_PROJECT_GET_CODE_H
