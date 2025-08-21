#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Fork {
    bool busy{false};
    std::mutex m;
    std::condition_variable cv;
public:
    void pick() {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]{ return !busy; });
        busy = true;
    }
    void drop() {
        {
            std::lock_guard<std::mutex> lock(m);
            busy = false;
        }
        cv.notify_one();
    }
};

class Philosopher {
    std::string name;
    std::shared_ptr<Fork> left;
    std::shared_ptr<Fork> right;
    bool reverse;
public:
    Philosopher(std::string n, std::shared_ptr<Fork> l, std::shared_ptr<Fork> r, bool rev = false)
        : name(std::move(n)), left(std::move(l)), right(std::move(r)), reverse(rev) {}
    void dine(int rounds) {
        for (int i = 0; i < rounds; ++i) {
            if (!reverse) { left->pick(); right->pick(); }
            else { right->pick(); left->pick(); }
            std::cout << name << " eating " << (i + 1) << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            left->drop();
            right->drop();
            std::cout << name << " thinking " << (i + 1) << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        std::cout << name << " finished\n";
    }
};

int main() {
    auto f1 = std::make_shared<Fork>();
    auto f2 = std::make_shared<Fork>();
    auto f3 = std::make_shared<Fork>();

    Philosopher diogenes("Diogenes", f1, f2, false);
    Philosopher plato("Plato", f2, f3, false);
    Philosopher aristotle("Aristotle", f3, f1, true);

    std::thread t1([&]{ diogenes.dine(5); });
    std::thread t2([&]{ plato.dine(5); });
    std::thread t3([&]{ aristotle.dine(5); });

    t1.join(); t2.join(); t3.join();
    return 0;
}