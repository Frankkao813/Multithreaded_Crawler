// HW1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
// HTMLParserTest.cpp
// Example program showing how to use the parser
// CSCE 463 sample code
//
//#include "stdafx.h"

int main(int argc, char** argv)
{

	// thread count
	
	if (argc != 3) {
		printf("Usage: hw1.exe [num_threads] [filename].txt");
		return -1;
	}

	// In this assignment, we are able to support more than 1 thread

	std::string num_threads = argv[1];
	std::string filename = argv[2];



	if (filename.size() < 4 || (filename.size() > 4 &&  filename.substr(filename.size() - 4) != ".txt")) {
		printf("Please provide a valid text file.\n");
		return -1;
	}


	// intialize the 
	WSADATA wsaData{};

	// Initialize WinSock; once per program run
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		printf("WSAStartup error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}


	int threadCnt = std::stoi(argv[1]); // turn the thread count into integer

	{
		Crawler crawler;
		if (!crawler.ReadFilePointer(filename)) {
			return -1;
		}

		auto timeStart = std::chrono::steady_clock::now();

		// create the thread for statistics
		std::thread statsThread(&Crawler::PrintStat, &crawler);

		std::vector<std::thread> workers;
		// create N crawling threads
		for (int i = 0; i < stoi(num_threads); i++) {
			workers.emplace_back(&Crawler::Run, &crawler);
		}


		for (auto& worker : workers) {
			worker.join();
		}

		crawler.StopInfoPrint();
		statsThread.join();

		auto timeEnd = std::chrono::steady_clock::now();
		double elapsedTime = std::chrono::duration<double>(timeEnd - timeStart).count();
		
		crawler.FinalPrint(elapsedTime);

	}



	// all workers has finished at this point





	WSACleanup();

	return 0;
}




