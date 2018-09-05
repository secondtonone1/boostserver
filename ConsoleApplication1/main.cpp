// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#ifdef __linux__
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#endif //__linux__

#include "Server.h"

int main(void) {
	try {
		std::cout << "server start." << std::endl;

#ifdef __linux__
		struct sigaction sa;
		memset(&sa, 0, sizeof (sa));
		sa.sa_handler = SIG_IGN;
		if (sigaction(SIGPIPE, &sa, NULL) < 0)
		{
			LOG4CXX_WARN(logger_, "Error ignoring SIGPIPE!");
		}
#endif //__linux__


		// 建造服务对象
		boost::asio::io_service ios;
		// 具体的服务器地址与端口
		boost::asio::ip::tcp::endpoint endpotion(boost::asio::ip::tcp::v4(), 13695);
		// 构建Server实例
		BoostServer server(ios, endpotion);
		// 启动异步调用事件处理循环
		server.run();
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << std::endl;
	}
	std::cout << "server end." << std::endl;
	getchar();
	return 0;
}


