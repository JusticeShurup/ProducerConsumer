#include <iostream>
#include <Poco/ThreadPool.h>
#include <fstream>
#include <functional>
#include <queue>
#include <thread>
#include <atomic>

std::atomic<bool> working = true;

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

bool try_pop(std::queue<int>& queue) {
	if (queue.size() > 0)
		return 1;
	else
		return 0;
}

class Target 
	: public Poco::Runnable {
public:
	Target(int a, std::function<void(int)>& function) : _a(a), _function(function) {};

	void run() override {
		_function(_a);
	}

private:
	int _a;
	std::function<void(int)> _function;
};

void producer(std::queue<int>& queue) {
	std::fstream input("input.txt");
	int element;
	while (input >> element)
	{
		working = true;
		queue.push(element);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	working = false;
	input.close();
}

int main() {
	std::queue<int> queue;
	std::queue<Target> targets;
	std::thread producer(&producer, std::ref(queue));
	Poco::ThreadPool pool(4, 32);
	std::function<void(int)> fib;
	fib = &fibChislo;
	bool flag;
	Target* target;
	while (!queue.empty() || working) {
		flag = true;

		while ( true)
			if (try_pop(queue)) {
				target = new Target(queue.front(), fib);
				break;
			}
		try {
			pool.start(*target); 
		}
		catch (Poco::NoThreadAvailableException& e) {
			flag = false;
		}
		if (flag) {
			queue.pop();
		}
		
	}
	producer.join();
	pool.joinAll();

	return 0;
}