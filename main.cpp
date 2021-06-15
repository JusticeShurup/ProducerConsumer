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
bool flag = true;
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

void runThread(std::queue<int>& queue) {
	std::unique_lock<std::mutex> lock_empty_data(mutex_empty_data);
	while (flag) {
		while (queue.empty())
			no_data_condition.wait(lock_empty_data);
		fib(queue.front());
		queue.pop();
		collection_full.notify_one();
	}
}

void addElem(std::queue<int>& queue) {
	int element;
	std::unique_lock<std::mutex> lock_full_data(mutex_full_data);
	std::ifstream input("input.txt");
	while (input >> element) {
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

	std::thread producer(&addElem, std::ref(queue));

	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++) 
		threads.push_back(std::thread(&runThread, std::ref(queue)));
	producer.join();

	for (int i = 0; i < 4; i++) 
		threads[i].join();


	return 0;
}