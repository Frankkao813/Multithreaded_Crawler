
#include "pch.h"
#include "winsocket.h"


#pragma comment(lib, "ws2_32.lib")



Socket::Socket(std::atomic<unsigned long long>& totalCounter, std::atomic<unsigned long long>& webpageCounter) : downloadedBytes(totalCounter), webpageDownloadedBytes(webpageCounter){
	// create this buffer once, then possibly reuse for multiple connections in Part 3 
	// either new char [INITIAL_BUF_SIZE] or malloc (INITIAL_BUF_SIZE) 
	allocatedSize = INITIAL_BUF_SIZE;
	buf = std::vector<char>(allocatedSize);

	this->curPos = 0;

	// intialize the socket
	this->sock = INVALID_SOCKET;

	// intialize the parseURL
	// this->urlSplit = urlSplit;


}

// urlSplit and DNS info
bool Socket::intializeInfo(parsedURL urlSplit, std::string IP) {
	this->urlSplit = urlSplit;
	this -> curPos = 0;
	this->IP = IP;

	// TODO: probably also need to resize the 
	resetBuffer();
	return true;
}



bool Socket::connectToServer(std::string info) {

	std::string ip_addr = urlSplit.host;
	int port = urlSplit.port;
	std::string path = urlSplit.path;
	std::string host = urlSplit.host;

	//printf("Entering the connect to server \n");

	// TODO: need to revise the request sent, it should not contain # and fragment
	std::string resource = urlSplit.resource; // everything starting from [/path], /(slash) included

	// we want to close and reopen the socket
	closeSocket();




	// open a TCP socket
	this -> sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket() generated error %d\n", WSAGetLastError());
		//WSACleanup(); // handled in main()
		return false;
	}

	// structure used in DNS lookups
	// struct hostent* remote;

	// structure for connecting to server
	struct sockaddr_in server {};




	// first assume that the string is an IP address
	DWORD IP = inet_addr(this->IP.c_str());
	double dnsTimeMs = 0;
	
	server.sin_addr.S_un.S_addr = IP;


	// setup the port # and protocol type
	server.sin_family = AF_INET;
	server.sin_port = htons(port);		// host-to-network flips the byte order

	// start to connect to the server, the default is 80
	auto connectStart = std::chrono::steady_clock::now();
	int connectRes = connect(sock, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
	auto connectEnd = std::chrono::steady_clock::now();

	double connectMs = std::chrono::duration<double, std::milli>(connectEnd - connectStart).count();
 

	if (connectRes == SOCKET_ERROR)
	{
		//printf("%8s Connection error: %d\n","", WSAGetLastError());
		return false;
	}



	//// TODO: modify this
	//if (info == "robots") {
	//	printf("%8s Connecting on %s...  took %.2f ms\n", "", info.c_str(), connectMs);
	//}
	//else if (info == "pages") {
	//	printf("%6s * Connecting on %s...  took %.2f ms\n", "", info.c_str(), connectMs);
	//}


	//printf("Successfully connected to %s (%s) on port %d\n", host.c_str(), inet_ntoa(server.sin_addr), htons(server.sin_port));
	return true;
}

void Socket::resetBuffer() {


	// 64KB: justification - total bytes crawled / number of pages crawled
	constexpr size_t RETAIN_LIMIT = 64 * 1024; 

	if (buf.capacity() > RETAIN_LIMIT) {
		std::vector<char>(INITIAL_BUF_SIZE).swap(buf);
	}
	else {
		buf.clear();
		buf.resize(INITIAL_BUF_SIZE);
	}

	curPos = 0;
	allocatedSize = buf.size();
}

HTTPResponse Socket::fetchResponse(std::string resource) {


	HTTPResponse res;


	std::string request =
		"GET " + resource + " HTTP/1.0\r\n"
		"User-Agent: myTAMUCrawler\r\n"
		"Host: " + urlSplit.host + "\r\n"
		"\r\n";
	


	//std::cout << "The request sent is ... \n" << request << std::endl;

	int iResult = send(sock, request.c_str(), static_cast<int>(request.size()), 0);
	if (iResult == SOCKET_ERROR) {
		//wprintf(L"send failed with error: %d\n", WSAGetLastError());
		closesocket(sock);
		//WSACleanup();
		return res;
	}




	bool readRes = Read(resource); 
	if (readRes == false) {// There were some problem in the read function
		return res;
	}

	std::string response(
		this -> buf.data(),
		this -> curPos
	);

	//std::cout << response << std::endl;

	// split the return value by \r\n
	size_t pos = response.find("\r\n\r\n");

	if (pos != std::string::npos) {
		res.header = response.substr(0, pos);
		res.content = response.substr(pos + 4);
		res.success = true;

		//std::cout << "header size: " << header.size() << std::endl;
		//std::cout << "HEADER START\n";
		//std::cout << header;
		//std::cout << "\nHEADER END\n";
	}


	return res;

}

Socket::~Socket() {

	// close the socket to this server; open again for the next one
	closeSocket();
}

void Socket::closeSocket() {
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}


// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select
bool Socket::Read(std::string mode)
{
	// reset the buffer if reading the resource
	if (mode == urlSplit.resource) {
		resetBuffer();
	}



	const int PAGE_LIMIT = 2 * 1024 * 1024; // 2 MB
	const int ROBOTS_LIMIT = 16 * 1024;       // 16 KB
	const double TIME_LIMIT = 10.0;
	int maxSize = (mode == "/robots.txt") ? ROBOTS_LIMIT : PAGE_LIMIT;

	int ret;


	// set timeout to 10 seconds 
	
	auto readStart = std::chrono::steady_clock::now();
	while (true)
	{
		// check the elapsed time first
		auto currTime = std::chrono::steady_clock::now();
		auto elapsedTimeSec = std::chrono::duration<double>(currTime - readStart).count();
		double remaining = TIME_LIMIT - elapsedTimeSec;
		if (remaining <= 0.0) {
			//printf("%8s Loading... failed with slow download\n", "");
			closeSocket();
			return false;
		}
		
		// select() modifies the parameters you pass to it, 
		// you must reinsert sock into fd_set each time you call select()
		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(sock, &readSet);

		timeval timeout{};
		timeout.tv_sec = static_cast<long>(remaining);
		timeout.tv_usec = static_cast<long>(
			(remaining - timeout.tv_sec) * 1000000.0
		);

		// wait to see if socket has any data (see MSDN) 
		// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-select?redirectedfrom=MSDN
		if ((ret = select(0, &readSet, nullptr, nullptr, &timeout)) > 0)
		{
			// new data available; now read the next segment 
			int bytes = recv(sock, buf.data() + curPos, allocatedSize - curPos, 0);
			//std::cout << "received " << bytes << std::endl;
			if (bytes > 0) {

				downloadedBytes += bytes;
				if (mode != "/robots.txt") {
					webpageDownloadedBytes += bytes;
				}

				if (curPos + bytes > maxSize) {
					//printf("%8s Loading... failed with exceeding max\n", "");
					closeSocket();
					return false;
				}

				curPos += bytes;
				if (allocatedSize - curPos < THRESHOLD) {
					// dynamic buffer resizing using API in vector class
					allocatedSize *= 2;
					buf.resize(allocatedSize);

				}
			}
			else if (bytes == 0) {
				//printf("%s \n", "The server has closed successfully");
				auto readEnd = std::chrono::steady_clock::now();
				auto readTimeMs = std::chrono::duration<double, std::milli>(readEnd - readStart).count();

				// check if we have received HTTP headers
				if (curPos < 5 || std::string(buf.data(), 5) != "HTTP/") {
					//printf("%8s Loading... failed with non-HTTP header (does not begin with HTTP/)\n", "");
					return false;
				}

				//printf("%8s Loading... done in %.2f ms with %d bytes \n", "", readTimeMs, curPos);
				return true;

			}
			else {
				// There is error
				int errorCode = WSAGetLastError();
				//printf("%8s Loading... failed with %d on recv\n", "", errorCode);
				return false;
			}


			// TODO: check currPos to see if there it exceeds the maximum limit. If so, we close the connection and return false
			


		}
		else if (ret == 0) {
			// report timeout
			//printf("%8s The select function failed because of timeout \n", "");
			break;
		}
		else {
			// There are some problems with the system
			int errorCode = WSAGetLastError();
			//printf("The select function failed because of %d \n", errorCode);
			return false;
		}

	}
	return false;
}
