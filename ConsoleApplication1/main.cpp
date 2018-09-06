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
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>
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
			std::cout << "Error ignoring SIGPIPE!"<<std::endl;
		}
#endif //__linux__

		boost::property_tree::ptree pt,tag_setting;
		boost::property_tree::ini_parser::read_ini("../data/config.ini",pt);
		//tag_setting = pt.get_child("config");
		
		boost::asio::io_service ios;
		short sport = pt.get<short>("config.tcpport");
		//boost::asio::ip::address add;
		//add.from_string("127.0.0.1");
		
		//boost::asio::ip::tcp::endpoint endpotion(add, short(13695));
		
		boost::asio::ip::tcp::endpoint endpotion(boost::asio::ip::tcp::v4(),sport);
		BoostServer server(ios, endpotion);
		server.run();
	}
	catch (std::exception& _e) {
		std::cout << _e.what() << std::endl;
	}
	std::cout << "server end." << std::endl;
	getchar();
	return 0;
}


