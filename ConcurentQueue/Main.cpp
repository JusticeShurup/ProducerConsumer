#include <iostream>
#include <tbb/concurrent_queue.h>
#include <thread>
#include <vector>
#include <fstream>
#include <functional>

void fibChislo(int number) {
	int a = 1, c = 1, b = 1;
	if (number <= 2) {
		std::cout << a << std::endl;
	}
	for (int i = 3; i <= number; i++) {
		c = a + b;
		a = b;
		b = c;
	}
	std::cout << c << std::endl;
}

class ThreadPool {
public:
	ThreadPool(int thr_numb){
		function = fibChislo;
		for (int i = 0; i < thr_numb; i++) {
			threads.push_back(std::thread(&ThreadPool::consumer, this));
		}
	}
	void start() {
		working = true;
	}
	void consumer() {
		while (working || !queue.empty()) {
			int number; 
			queue.try_pop(number);
			if (function != nullptr) {
				function(number);
			}
		}
	}

	void close() {
		working = false;
		for (int i = 0; i < threads.size(); i++) {
			threads[i].join();
		}
	}

	void push(int element) {
		queue.push(element);
	}

private:
	std::function<void (int)> function;
	tbb::concurrent_queue<int> queue;
	std::vector<std::thread> threads;
	bool working = false;
};

void producer(ThreadPool& pool) {
	std::fstream input("input.txt");
	int element;
	while (input  >> element)
	{
		pool.push(element);
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}
	input.close();
}

int main() {
	ThreadPool pool(4);
	std::thread producer(&producer, std::ref(pool));
	pool.start();
	producer.join();
	pool.close();
	return 0;
}