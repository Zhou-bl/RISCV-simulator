#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include "runner.h"
#include "get_code.h"

using namespace std;

int main(){
    //freopen("testcases/tak.data", "r", stdin);
    //freopen("ans.txt", "w", stdout);
    read_into_mem(mem);
    run();
    /*
    std::cout << cpu_cycle << std::endl; //输出cpu_cycle来反映流水线效率;
    std::cout << "branch cnt : " << bht.branch_cnt << std::endl;
    std::cout << "right cnt : " << bht.right_cnt << std::endl;
    if(bht.branch_cnt){
        std::cout << 1.0 * bht.right_cnt / bht.branch_cnt * 100.0 << std::endl;
    }
     */
}