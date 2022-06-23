#include "sort.hpp"

std::vector<int> my_merge_sort(int n, int l = 0){

    if(n == 1){
        std::vector<int> a;
        a.push_back(l);
        return a;
    }
    int mid = n - 1 >> 1;
    std::vector<int> lv = my_merge_sort(mid + 1, l);
    std::vector<int> rv = my_merge_sort(n - mid - 1, l + mid + 1);
    std::vector<int> res;
    int i = 0, j = 0;
    for(; i < lv.size() && j < rv.size();){
        if(query(lv[i], rv[j])){
            res.push_back(lv[i++]);
        }
        else{
            res.push_back(rv[j++]);
        }
    }
    while(i < lv.size()){
        res.push_back(lv[i++]);
    }
    while(j < rv.size()){
        res.push_back(rv[j++]);
    }
    return res;
}

std::vector<int> my_sort(int n){
    return my_merge_sort(n, 0);
}