#include <thread>


#include "include/dns_packet.h"
#include "include/dns_receiver.h"
#include "include/dns_sender.h"
#include "include/my_socket.h"
#include "include/host_list.h"
#include "include/job_queue.h"
#include "include/my_map.h"

void recv_t(JobQueue *job_queue)
{
	DNSReceiver recver(job_queue);
	recver.Start();
}

void sender_t(JobQueue *job_queue, HostList *host_list, MyMap *my_map)
{
	DNSSender sender(job_queue, host_list, my_map);
	sender.Start();
}

int checkparameters(const int argc, const char* argv[])
{
	if (argc != 1)
	{
		for (int i = 1; i < argc; i++)
		{
			std::string parameter(argv[i]);
			if (parameter == "-d")
			{

			}
			else if (parameter == "-dd")
			{

			}
			else
			{

			}
		}
	}
}


int main(const int argc, const char* argv[])
{
	checkparameters(argc, argv);
	HostList host_list();
	JobQueue job_queue;
	MyMap my_map;

	std::thread	r(recv_t, &job_queue);
	Sleep(100);
	void (*so)(JobQueue *, HostList *, MyMap *, const std::string &) = sender_t;
	
	constexpr int sender_num = 4;
	std::vector<std::thread> sender_vec;
	for (int i = 0; i < sender_num; i++)
	{
		sender_vec.push_back(std::thread(so, &job_queue, &host_list, &my_map, "114.114.114.114"));
		Sleep(100);
	}
	
	
	// std::thread s(so, &job_queue, &host_list, &my_map);
	// std::thread s1(so, &job_queue, &host_list, &my_map);
	
	for (auto &t : sender_vec)
	{
		t.join();
	}
	
	// s.join();
	// s1.join();
	r.join();

    return 0;
}


