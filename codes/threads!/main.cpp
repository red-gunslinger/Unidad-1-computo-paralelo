#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

class Summation {
    int id_;
    int total_{0};

public:
    explicit Summation(int id) : id_(id) {}

    void run() {
        std::random_device rd;
        auto seed = static_cast<unsigned>(
            rd() ^
            std::hash<std::thread::id>{}(std::this_thread::get_id()) ^
            static_cast<unsigned>(
                std::chrono::high_resolution_clock::now()
                    .time_since_epoch()
                    .count()));
        std::mt19937 gen(seed);
        std::uniform_int_distribution<int> dist(1, 1000);
        int s = 0;
        for (int i = 0; i < 100; ++i) s += dist(gen);
        total_ = s;
    }

    int id() const { return id_; }
    int total() const { return total_; }
};

int main() {
    constexpr int kThreads = 10;
    std::vector<Summation> tasks;
    tasks.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i) tasks.emplace_back(i + 1);

    std::mutex print_mtx;
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i] {
            tasks[i].run();
            std::lock_guard<std::mutex> lk(print_mtx);
            std::cout << "El thread #" << tasks[i].id()
                      << " sumo: " << tasks[i].total() << "\n";
        });
    }

    for (auto& t : threads) t.join();

    auto it = std::max_element(
        tasks.begin(), tasks.end(),
        [](const Summation& a, const Summation& b) { return a.total() < b.total(); });

    if (it != tasks.end()) {
        std::cout << "\nEl thread con mayor puntuacion fue el #"
                  << it->id() << " y sumo: " << it->total() << "\n";
    }
    return 0;
}

