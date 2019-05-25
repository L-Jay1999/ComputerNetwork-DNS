#include <thread>
#include <mutex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <string>
#include <atomic>

#include "log.h"
static const std::string log_dir = "./";
static const std::string log_ext = ".log";
static std::string log_path;
static bool is_init = false;
static std::mutex mtx_;
static std::ofstream log_ofs;
static int dbg_level = 2;
static int log_id = 0;

void Log::InitLog(const int debug_level)
{
	std::stringstream ss;
	std::time_t current_time = std::time(nullptr);
	std::tm time_pack;
	auto error_code = localtime_s(&time_pack, &current_time);

	ss << std::put_time(&time_pack, "%d-%H%M%S");
	std::string log_name = ss.str();

	log_path = log_dir + log_name + log_ext;

	log_ofs.open(log_path);
	if (log_ofs)
	{
		dbg_level = debug_level;
		is_init = true;
	}
	else
	{
		is_init = false;
	}
}

void Log::WriteLog(const int level, const std::string &log)
{
	std::lock_guard<std::mutex> lock(mtx_);
	if (!is_init)
	{
		InitLog(2);
	}
	if (level <= dbg_level)
	{
		static std::stringstream ss;
		std::string timestamp, current_thread_id, log_id_str;
		std::time_t current_time = std::time(nullptr);
		std::tm time_pack;
		auto error_code = localtime_s(&time_pack, &current_time);

		log_id_str = std::to_string(log_id++) + "\t";

		ss << std::put_time(&time_pack, "%H:%M:%S") << " ";
		timestamp = ss.str();
		ss.clear();
		ss.str(std::string());

		ss << "tid: " << std::this_thread::get_id() << "\t";
		current_thread_id = ss.str();
		ss.clear();
		ss.str(std::string());

		std::cout << log_id_str << timestamp << current_thread_id << log << std::endl;
		log_ofs << log_id_str << timestamp << current_thread_id << log << std::endl;
	}
}

void Log::CloseLog()
{
	std::lock_guard<std::mutex> lock(mtx_);
	if (log_ofs)
		log_ofs.close();
}