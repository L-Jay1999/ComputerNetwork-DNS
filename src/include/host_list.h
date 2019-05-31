﻿#pragma once

#include <unordered_map>
#include <string>

enum HostState
{
	BANNED = 0,//����
	FIND,      //�ҵ�
	NOT_FIND   //δ�ҵ�
};

class HostList
{
public:
	HostList(const std::string &path = "hosts.txt");
	HostState get_host_state(const std::string &host_name) const;
	std::string get_ip_str(const std::string &host_name) const;

private:
	bool Load(const std::string &path);
	std::string host_path_;                                 //�ļ�����·��
	static const std::string banned_host_;                  //�������ַ���
	std::unordered_map<std::string, std::string> host_map_; // <host_name, ip_addr>
};