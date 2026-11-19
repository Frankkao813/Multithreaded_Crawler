#pragma once
class Crawler {

private:
	//std::queue<std::string> urls; // TODO: queue of pointers
	std::atomic<int> activeThreads{ 0 }; // the number before Q
	std::atomic<int> numUrl{ 0 }; // Q: current size of the pending queue
	std::atomic<int> extractedUrl{ 0 }; // E: number of extracted URLs from the queue

	std::atomic<int> uniqueHosts{ 0 }; // H: number of URLs that have passed host uniqueness
	std::atomic<int> successDNSLookup{ 0 }; // D: number of successful DNS lookup
	std::atomic<int> uniqueIPs{ 0 }; // I: number of URLs that have passed IP uniqueness
	std::atomic<int> successRobotChecks{ 0 }; // R: number of URLs that have passed robot checks
	std::atomic<int> successCrawledUrl{ 0 }; // C: number of successfully crawled URLs
	std::atomic<int> parsedLinks{ 0 }; // L

	std::atomic<int> containTamuLinkCnt{ 0 };
	std::atomic<int>  hostOutsideTamuCnt{ 0 };


	// http status codes
	std::atomic<int> http2xx{ 0 };
	std::atomic<int> http3xx{ 0 };
	std::atomic<int> http4xx{ 0 };
	std::atomic<int> http5xx{ 0 };
	std::atomic<int> others{ 0 };


	std::atomic<unsigned long long> downloadedBytes{ 0 };
	std::atomic<unsigned long long> webpageDownloadedBytes{ 0 };
	

	std::set<std::string> seenHost;  
	std::set<DWORD> seenIPs;

	// mutex for the queue
	std::mutex queueMutex;
	std::mutex seenHostMutex;
	std::mutex seenIPMutex;

	// for statistics thread
	std::atomic<bool> stopStat{false};
	void StatusCodeCountUpdate(std::string webpageCode);

	// queue of pointer trial
	std::unique_ptr<char[]> buffer;
	std::queue<char*> urlChar;


public:
	//bool ReadFile(std::string filePath); // called by main
	bool ReadFilePointer(std::string filename); // trial: the alternative method to store...
	void Run();
	void PrintStat();
	void StopInfoPrint();
	bool ResolveAndCheckIP(parsedURL url_part, DWORD& ip, double& dnsTimeMs);
	void FinalPrint(double elapsedTime); // The print statement after all threads leaves successfully
	Crawler();
};