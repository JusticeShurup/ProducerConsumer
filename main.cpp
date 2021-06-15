#include <iostream>
#include <fstream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mutex_empty_data;
std::mutex mutex_full_data;
std::condition_variable no_data_condition;
std::condition_variable collection_full;
void fib(int number) {
	std::chrono::milliseconds(10);
	int a = 1, b = 1, c;
	if (number <= 2) {
		std::cout << b << std::endl;
		return;
	}
	for (int i = 3; i <= number; i++) {
		c = a + b;
		a = b; b = c;
	}
	std::cout << c << std::endl;
}

void runThread(std::queue<int>& queue, std::queue<int>& all_elems) {
	std::unique_lock<std::mutex> lock_empty_data(mutex_empty_data);
	while (all_elems.size() > 0) {
		while (queue.empty())
			no_data_condition.wait(lock_empty_data);
		fib(queue.front());
		queue.pop();
		all_elems.pop();
		collection_full.notify_one();
	}
}

void addElem(std::queue<int>& queue, std::ifstream& input) {
	int element;
	std::unique_lock<std::mutex> lock_full_data(mutex_full_data);
	while (input >> element) {
		while (queue.size() >= 100) {
			collection_full.wait(lock_full_data);
		}
		queue.push(element);
		no_data_condition.notify_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void full_all_elems(std::queue<int>& all_elems) {
	std::ifstream input("input.txt");
	int element;
	while (input >> element)
		all_elems.push(element);
	input.close();
}

int main() {
	std::queue <int> queue;
	std::ifstream input("input.txt");

	std::queue <int> all_elems;
	full_all_elems(all_elems);

	std::thread producer(&addElem, std::ref(queue), std::ref(input));

	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++) 
		threads.push_back(std::thread(&runThread, std::ref(queue), std::ref(all_elems)));

	for (int i = 0; i < 4; i++) 
		threads[i].join();

	producer.join();

	input.close();
	return 0;
}