#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <random>

// Каждый поток обрабатывает свой диапазон [begin, end) векторов.
// Ничего не печатает, только пишет результат в sums[i].
void worker(const std::vector<std::vector<int>>& vectors,
            std::vector<long long>& sums,
            size_t begin, size_t end) {
    for (size_t i = begin; i < end; ++i) {
        long long s = 0;
        for (int x : vectors[i]) s += x;
        sums[i] = s;
    }
}

// Запускает numThreads потоков и возвращает массив сумм.
std::vector<long long> computeSums(const std::vector<std::vector<int>>& vectors,
                                   int numThreads) {
    std::vector<long long> sums(vectors.size(), 0);
    std::vector<std::thread> threads;
    size_t chunk = vectors.size() / numThreads;

    for (int t = 0; t < numThreads; ++t) {
        size_t begin = t * chunk;
        size_t end = (t == numThreads - 1) ? vectors.size() : begin + chunk;
        threads.emplace_back(worker,
                             std::cref(vectors),
                             std::ref(sums),
                             begin, end);
    }
    for (auto& th : threads) th.join();
    return sums;
}

int main() {
    const size_t N = 30000;
    const size_t M = 100;

    // 1. Заполняем данные одним потоком — ДО замера времени.
    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(-1000, 1000);
    std::vector<std::vector<int>> vectors(N, std::vector<int>(M));
    for (auto& v : vectors)
        for (int& x : v) x = dist(gen);

    std::string header = "Vectors: " + std::to_string(N)
                       + ", elements per vector: " + std::to_string(M) + "\n";
    std::cout << header << std::endl;

    // 2. Замеряем только параллельную часть. Используем микросекунды.
    for (int numThreads : {1, 2, 4}) {
        auto start = std::chrono::high_resolution_clock::now();
        auto sums = computeSums(vectors, numThreads);
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        long long total = 0;
        for (auto s : sums) total += s;

        std::string report = "Threads: " + std::to_string(numThreads)
                           + " | Time: " + std::to_string(us) + " us"
                           + " | Control sum: " + std::to_string(total) + "\n";
        std::cout << report;
    }
    return 0;
}
