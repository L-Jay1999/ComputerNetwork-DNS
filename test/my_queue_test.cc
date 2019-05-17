#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>

#include "include/my_queue.h"

std::mutex m;
MyQeque a;
void print(int i)
{
	std::lock_guard<std::mutex> guard(m);
	std::cout << std::this_thread::get_id() << "\t" << i << std::endl;
}

void push(MyQeque &q)
{
	for (int i = 0; i < 100; i++)
		q.push_back(i);
}

void pop(MyQeque & q)
{
	int data;
	for (int i = 0; i < 100 / 2; i++)
	{
		data = q.pop_front();
		print(data);
	}
}

int main()
{
	auto start = std::chrono::system_clock::now();
	std::thread p(push, std::ref(a)), q1(pop, std::ref(a)), q2(pop, std::ref(a));
	p.join();
	q1.join();
	q2.join();
	auto end = std::chrono::system_clock::now();
	std::cout << (end - start).count() << std::endl;
	return 0;
}
