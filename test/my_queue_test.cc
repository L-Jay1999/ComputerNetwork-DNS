#include "../src/include/my_queue.h"
#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>


std::mutex m;
MyQueue a;
void print(int i)
{
	std::lock_guard<std::mutex> guard(m);
	std::cout << std::this_thread::get_id() << "\t" << i << std::endl;
}

void push(MyQueue &q)
{
	for (long long i = 0; i < 100000000 / 2; i++)
		q.push_back(i);
}

void pop(MyQueue & q)
{
	int data;
	for (long long i = 0; i < 100000000 / 2; i++)
	{
		data = q.pop_front();
		// print(data);
	}
}

int main()
{
	
	auto start = std::chrono::system_clock::now();
#define T_TEST
#ifdef T_TEST
	std::thread p1(push, std::ref(a)), p2(push, std::ref(a)), q1(pop, std::ref(a)), q2(pop, std::ref(a));//, q3(pop, std::ref(a));//, q4(pop, std::ref(a));
	p1.join();
	p2.join();
	q1.join();
	q2.join();
	// q3.join();
	// q4.join();
	// 70.8 56.7 56.4 55.7 61.0 58.5 57.9 56.8 56.5 62.6 57.4 
#else
	long long data;
	for (long long i = 0; i < 100000000; i++)
	{
		a.push_back(i);
		data = a.pop_front();
	}
	// 59.4 60.2 59.9 61.8 60.36 59.8 59.9 65.3 59.4 60.0

#endif
	auto end = std::chrono::system_clock::now();
	std::cout << (end - start).count()/1000000.0 << std::endl;
	return 0;
}
