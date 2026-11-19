#pragma once

struct HTTPResponse {
	bool success = false;
	std::string header = "";
	std::string content = "";
};


class Socket {
	private:
		SOCKET  sock; //socket handle;
		//std::vector<char> buf; // current buffer;
		int allocatedSize; // bytes allocated for buf
		int INITIAL_BUF_SIZE = 8192;
		int THRESHOLD = 4096;
		parsedURL urlSplit;
		std::string IP; // the IP address resolved by the main function
		std::atomic<unsigned long long>& downloadedBytes; // downloaded bytes from the crawler
		std::atomic<unsigned long long>& webpageDownloadedBytes; // downloaded bytes, but from the webpages and not the robot

		bool Read(std::string mode);
		void closeSocket();
		void resetBuffer();

	public: 
		std::vector<char> buf; // current buffer;
		//Socket();
		explicit Socket(std::atomic<unsigned long long>& totalCounter, std::atomic<unsigned long long>& webpageCounter);

		~Socket();
		bool connectToServer(std::string info);
		HTTPResponse fetchResponse(std::string resource);
		bool intializeInfo(parsedURL urlSplit, std::string IP);
		int curPos; // current position in buffer

};
