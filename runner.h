//
// Created by zbl on 2022/6/21.
//

#ifndef RISCV_PROJECT_RUNNER_H
#define RISCV_PROJECT_RUNNER_H

#include "order_class.h"

using std::cout;
using std::endl;

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
    //buffer[orderNum++] = order("Pause");
    buffer[orderNum++] = order("Pause");
    buffer[0].order_IF();buffer[0].order_ID();buffer[0].order_EXE();buffer[0].order_MEM();
    buffer[orderNum++] = order("Pause");
    buffer[1].order_IF();buffer[1].order_ID();buffer[1].order_EXE();
    buffer[orderNum++] = order("Pause");
    buffer[2].order_IF();buffer[2].order_ID();
    buffer[orderNum++] = order(next_pos);
    buffer[3].order_IF();
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

void reg_forwarding(){//buffer[0]在此时已经完成WB了;
    if(buffer[1].ins != "Pause" && buffer[1].need_WB){
        if(buffer[1].aimRd == buffer[3].rs1){
            buffer[3].rd1 = buffer[1].rd;
        }
        if(buffer[1].aimRd == buffer[3].rs2){
            buffer[3].rd2 = buffer[1].rd;
        }
    }
    if(buffer[2].ins != "Pause" && buffer[2].need_WB){
        if(buffer[2].aimRd == buffer[3].rs1){
            buffer[3].rd1 = buffer[2].rd;
        }
        if(buffer[2].aimRd == buffer[3].rs2){
            buffer[3].rd2 = buffer[2].rd;
        }
    }
}

int cnt = 0;

void run(){
    bool stop_flag = 0;
    init();
    bool insert_flag = 0;
    while(orderNum >= 4){
        insert_flag = 0; //表示尚未添加新的指令;
        buffer[0].order_WB();


/*
        if(buffer[0].type != order::nop){
            cout << ++cnt << " : ";
            std::cout << buffer[0].type << std::endl;
            cout << "reg test : ";
            for(int i = 0; i < 32; ++i){
                cout << reg[i] << " ";
            }
            cout << endl;
            cout << "mem test : ";
            for(int i = 0; i < 10000; ++i){
                cout << mem[i] << " ";
            }
            cout << endl;
        }
*/


        buffer[1].order_MEM();
        buffer[2].order_EXE();
        buffer[3].order_ID();
        reg_forwarding();//解决寄存器中的数据冲突;
        if(buffer[3].is_control() && !stop_flag){//如果是分支跳转指令,则在该指令后面插入一条NOP;
            buffer_insert(3, order("Pause"));
            insert_flag = 1;
        }
        if(buffer[3].is_load_and_store() && !stop_flag){//解决内存中的数据冲突;
            buffer_insert(3, order("Pause"));
            buffer_insert(3, order("Pause"));
            insert_flag = 1;
        }
        if(!insert_flag && !stop_flag && orderNum <= 4){
            if(mem[next_pos] == (int)0x0ff00513){
                stop_flag = 1;
            }
            else{
                order newOrder(next_pos);
                buffer_insert(orderNum - 1, newOrder);
                insert_flag = 1;
            }
        }
        buffer[4].order_IF();
        buffer_update();
    }
    for(int i = 0; i < orderNum; ++i){//最后几条指令顺序执行;
        buffer[i].order_IF();
        buffer[i].order_ID();
        buffer[i].order_EXE();
        buffer[i].order_MEM();
        buffer[i].order_WB();


/*
        if(buffer[i].type != order::nop){
            cout << ++cnt << " : ";
            std::cout << buffer[i].type << std::endl;
            cout << "reg test : ";
            for(int j = 0; j < 32; ++j){
                cout << reg[j] << " ";
            }
            cout << endl;
            cout << "mem test : ";
            for(int j = 0; j < 10000; ++j){
                cout << mem[j] << " ";
            }
            cout << endl;
        }
*/


    }
    printf("%u\n",reg[10] & 255u);
}

void run1(){
    while(true){
        if(mem[next_pos] == (int)0x0ff00513) break;
        order cur(next_pos);
        cur.order_IF();
        cur.order_ID();
        cur.order_EXE();
        cur.order_MEM();
        cur.order_WB();
        cout << ++cnt << " : ";
        std::cout << cur.type << std::endl;
        cout << "reg test : ";
        for(int i = 0; i < 32; ++i){
            cout << reg[i] << " ";
        }
        cout << endl;
        cout << "mem test : ";
        for(int i = 0; i < 10000; ++i){
            cout << mem[i] << " ";
        }
        cout << endl;
    }
    printf("%u\n",reg[10] & 255u);
}

#endif //RISCV_PROJECT_RUNNER_H
