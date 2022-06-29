//
// Created by zbl on 2022/6/21.
//

#ifndef RISCV_PROJECT_RUNNER_H
#define RISCV_PROJECT_RUNNER_H

#include "order_class.h"
void de_bug(){
    std::cout << "reg_test : " << " ";
    for(int i = 0; i < 32; ++i){
        std::cout << reg[i] << " ";
    }
    std::cout << std::endl;
}

order buffer[200000]; //0-4为在流水线中正在执行的指令;
int orderNum = 0; //当前buffer中的指令总数;

void init(){//插入四个空指令,保证初始时有五个指令
    buffer[orderNum++] = order("Pause");
    buffer[orderNum++] = order("Pause");
    buffer[orderNum++] = order("Pause");
    buffer[orderNum++] = order("Pause");
    buffer[orderNum++] = order(next_pos);
}

void buffer_insert(int index, const order &aimOrder){//在index的后面插入一个aimOrder;
    orderNum++;
    for(int i = orderNum - 1; i > index + 1; --i){
        buffer[i] = buffer[i - 1];
    }
    buffer[index + 1] = aimOrder;
}

void buffer_update(){
    orderNum--;
    for(int i = 0; i < orderNum; ++i){
        buffer[i] = buffer[i + 1];
    }
}

void run(){
    /*
    init();
    bool insert_flag = 0;
    while(orderNum >= 5){
        insert_flag = 0; //表示尚未添加新的指令;
        buffer[0].order_WB();
        buffer[1].order_MEM();
        buffer[2].order_EXE();
        buffer[3].order_ID();
        if(buffer[3].is_control()){//如果是分支跳转指令,则在该指令后面插入一条NOP
            buffer_insert(3, order("Pause"));
            insert_flag = 1;
        }
        buffer[4].order_IF();
        if(!insert_flag){
            buffer_insert(orderNum - 1, order(next_pos));
            insert_flag = 1;
        }
        buffer_update();
    }
     */
    while(true){
        if(mem[next_pos] == (int)0x0ff00513) break;
        order cur(next_pos);
        cur.order_IF();
        cur.order_ID();
        cur.order_EXE();
        cur.order_MEM();
        cur.order_WB();
    }
    printf("%u\n",reg[10] & 255u);
}

#endif //RISCV_PROJECT_RUNNER_H
