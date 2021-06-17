#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
std::atomic_bool flag = true;

std::mutex mutex_empty_data;
std::mutex mutex_full_data;
std::condition_variable no_data_condition;
std::condition_variable collection_full;
void fib(int number) {
	int a = 1, b = 1, c;
	if (number <= 2) {
		std::cout << b << std::endl;
		return;
	}
	for (int i = 3; i <= number; i++) {
		c = a + b;
		a = b; b = c;
	}
	std::cout << c  << std::endl;
}

void runThread(std::queue<int>& queue) {
	int number;
	while (flag) {
		{
			std::unique_lock<std::mutex> lock_empty_data(mutex_empty_data);
			while (queue.empty()) {
				no_data_condition.wait(lock_empty_data);
			}
			number = queue.front();
			queue.pop();
			collection_full.notify_one();
		}
		fib(number);
	}

}

void addElem(std::queue<int>& queue) {
	int element;
	std::ifstream input("input.txt");
	while (input >> element) {
		std::unique_lock<std::mutex> lock_full_data(mutex_full_data);
		while (queue.size() >= 100) {
			collection_full.wait(lock_full_data);
		}
		queue.push(element);
		no_data_condition.notify_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	input.close();
	flag = false;

}


int main() {
	std::queue <int> queue;
	std::vector<std::thread> threads;
	std::thread producer(&addElem, std::ref(queue));
	for (int i = 0; i < 4; i++) {
		threads.push_back(std::thread(&runThread, std::ref(queue)));
	}
	producer.join();	

	for (int i = 0; i < 4; i++) {
		threads[i].join();
	}

	return 0;
}