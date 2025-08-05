
#include "Result.hpp"
#include "threadPool.hpp"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
using ULL = unsigned long long;
class MyTask final : public Task {
public:
    MyTask(int l, int r): l_(l), r_(r) {}
    ~MyTask() override = default;

    Any run() override {
        ULL ans = 0;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        for (int i = l_; i <= r_; i++) {
            ans += i;
        }
        return ans;
    }

private:
    int l_, r_;
};

int main() {
#if 1
    {
        ThreadPool pool;
        pool.setMode(PoolMode::MODE_CACHED);
        pool.start(3);

        Result r1 = pool.submitTask(std::make_shared<MyTask>(1, 100000000));
        Result r2 = pool.submitTask(std::make_shared<MyTask>(100000001, 200000000));
        Result r3 = pool.submitTask(std::make_shared<MyTask>(200000001, 300000000));
        // ULL s1 = r1.get().cast<ULL>();
        // ULL s2 = r2.get().cast<ULL>();
        // ULL s3 = r3.get().cast<ULL>();
        // std::cout << s1 + s2 + s3 << "\n";
    }
#else
    {
        ThreadPool pool;
        pool.start(4);
        Result r1 = pool.submitTask(std::make_shared<MyTask>(1, 100000));
        ULL s1 = r1.get().cast<ULL>();
        std::cout << s1 << "\n";
    }
#endif

    std::cout << "put any char to exit\n";
    getchar();
    return 0;
}