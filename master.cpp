#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include "runner.h"
#include "get_code.h"

using namespace std;
unsigned reg[32];
unsigned mem[500000];

char int_to_char16(int x){
    return x < 10 ? x + '0' : x - 10 + 'a';
}

int main(){
    //freopen("testcases/magic.data", "r", stdin);
    //freopen("master.txt", "w", stdout);
    read_into_mem(mem);
    run(0, reg, mem);
    //test();
}