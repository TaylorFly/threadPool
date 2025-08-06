#include <iostream>
#include "thread_pool.hpp"
#include <thread>

using u64 = unsigned long long;

u64 add(int l, int r) {
    u64 ret = 0;
    for (int i = l; i <= r; i++) {
        ret += i;
    }
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return ret;
}

int main() {
    {
        ThreadPool pool;
        pool.start(2);
        pool.setMode(PoolMode::DYNAMIC);
        std::future<u64> future1 = pool.submitTask(add, 1, 1000000);
        std::future<u64> future2 = pool.submitTask(add, 1, 1000000);
        std::future<u64> future3 = pool.submitTask(add, 1, 1000000);
        std::future<u64> future4 = pool.submitTask(add, 1, 1000000);
        std::future<u64> future5 = pool.submitTask(add, 1, 1000000);
        std::future<u64> future6 = pool.submitTask(add, 1, 1000000);

        std::cout << future1.get() << std::endl;
        std::cout << future2.get() << std::endl;
        std::cout << future3.get() << std::endl;
        std::cout << future4.get() << std::endl;
        std::cout << future5.get() << std::endl;
        std::cout << future6.get() << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}