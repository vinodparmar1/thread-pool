
#include "threadpool.hpp"

#include <iostream>

int fun(int i, char c) {
    int j = 0;
    while(++j < 100) {
        std::cout << "from task1: j= " << j << ", c=" << c << "\n";
    }
    return i + j;
}

void fun1(std::string str) {
    for(int i =0; i < 100; ++i) {
        std::cout << "from task2: str = " << str << i << "\n";
    }
}
int main() {
    using namespace threadpool;
    Threadpool tp(2);
    auto ret = tp.submit(fun, 15, 'd');
    tp.submit(fun1, "hello");
    std::cout << "return value from task: " << ret.get() << "\n";
    std::thread(fun, 5, 'a').join();
    return 0;
}