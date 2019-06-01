#include <io.h>
#include <thread>
#include <cctype>

#include "include/dns_packet.h"
#include "include/dns_receiver.h"
#include "include/dns_sender.h"
#include "include/my_socket.h"
#include "include/host_list.h"
#include "include/job_queue.h"
#include "include/log.h"

void recv_t(JobQueue *job_queue)
{
	Log::WriteLog(2, __s("Initializing Receiver..."));
	DNSReceiver recver(job_queue);
	Log::WriteLog(2, __s("Receiver start working..."));
	recver.Start();
}

void sender_t(JobQueue *job_queue, HostList *host_list, const std::string &address)
{
	Log::WriteLog(2, __s("Initializing Sender..."));
	DNSSender sender(job_queue, host_list, address);
	Log::WriteLog(2, __s("Sender start working..."));
	sender.Start();
}

void print_help_msg(const char *exe_path)
{
	std::cout << "Usage: " << exe_path << " [superior dns ip address] [-d/-dd] [host rule file]\n"
			  << "Option:\n"
			  << "superior dns ip address: set the superior dns ip addrss, default: 10.3.9.4.\n"
			  << "-d/-dd: set debug level(-d = 1, -dd = 2), default debug level(no debug info): 0.\n"
			  << "host rule file: use rules provided in the host rule file instead of default rules.\n";
}

void checkparameters(const int argc, const char *argv[], std::string &host_path, std::string &superior_server_addr)
{
	bool debug_flag = true;
	bool path_flag = true;
	bool ip_flag = true;
	if (argc > 1 && argc < 5) //argc == 1时说明命令行输入只有程序运行全路径名，不作处理，不等于1时如下
	{
		for (int i = 1; i < argc; i++)
		{
			std::string parameter(argv[i]);		 //argv[i]指向输入命令行的字符串（从第一串起）
			if (parameter == "-d" && debug_flag) //当为-d时
			{
				Log::InitLog(1);
				debug_flag = false;
			}
			else if (parameter == "-dd" && debug_flag) //当为-dd时
			{
				Log::InitLog(2);
				debug_flag = false;
			}
			else
			{
				if (_access(parameter.c_str(), 0) != -1 && path_flag) // 判断文件是否存在
				{
					host_path = parameter; // 修改配置文件名
					path_flag = false;
				}
				else //当前输入字符串为域名
				{
					char buffer[8];
					auto ip_test = inet_pton(AF_INET, parameter.c_str(), buffer); // 将域名转化为地址结构体并存于buffer
					if (ip_test == 1 && ip_flag)								  // 转化成功
					{
						superior_server_addr = parameter; // 修改使用的服务器名
						ip_flag = false;
					}
					else
						std::cerr << "error:" << parameter << std::endl;
				}
			}
		}
	}
	else
		print_help_msg(argv[0]);
}

int main(const int argc, const char *argv[])
{
	if (argc > 4) // 当命令语句超过3段时输入出现问题直接返回 命令语法最长为dnsrelay [-d | -dd] [dns-server-ipaddr] [filename]
		return 0;

	std::string host_path = "hosts.txt";		   // 默认配置文件名
	std::string superior_server_addr = "10.3.9.4"; // 设置默认使用服务器为本地局域网

	checkparameters(argc, argv, host_path, superior_server_addr); // 就命令行输入视情况对配置文件名和服务器进行修改

	// 日志输出当前使用配置文件与服务器
	Log::WriteLog(2, __s("host file path: ") + host_path);
	Log::WriteLog(2, __s("supervisor dns server address: ") + superior_server_addr);

	HostList host_list(host_path); // 加载配置文件
	JobQueue job_queue;

	std::thread r(recv_t, &job_queue); // 接收数据包的线程
	Sleep(100);

	constexpr int sender_num = 8;
	std::vector<std::thread> sender_vec;
	for (int i = 0; i < sender_num; i++)
		sender_vec.push_back(std::thread(sender_t, &job_queue, &host_list, superior_server_addr));

	for (auto &t : sender_vec)
		t.join();

	r.join();
	Log::CloseLog();
	return 0;
}