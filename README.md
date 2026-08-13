# thread-pool - a header only task pool with typed futures and arbitrary-signature task submission

## Design

- thread-pool implementation using modern C++ constructs
- accepts a callable of any signature via a variadic template `submit`, constrained with a C++20 concept
- returns `std::future<R>` (R deduced with `std::invoke_result_t`) so the caller can retrieve the task's result later
- drains pending tasks on destruction before workers exit

## build

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --clean-first
```

## usage

```cpp
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
```