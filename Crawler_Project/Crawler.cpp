#include "pch.h"
#include "Crawler.h"

Crawler::Crawler() {
}



bool Crawler::ReadFilePointer(std::string filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	auto fileSize = file.tellg();
	// TODO: Opened URL-input-1M.txt with size 66,152,005 
	printf("Opened %s with %lld bytes\n", filename.c_str(), static_cast<long long>(fileSize));
	const auto length = static_cast<std::size_t>(fileSize);
	buffer = std::make_unique<char[]>(length + 1);

	file.seekg(0, std::ios::beg); // move from 0 bytes from the beginning

	if (!file.read(buffer.get(), static_cast<std::streamsize>(length))) {
		return false;
	}

	buffer[length] = '\0';
	if (length > 0) {
		urlChar.push(buffer.get());
	}

	for (size_t i = 0; i < length; i++) {
		if (buffer[i] == '\n') {
			buffer[i] = '\0';

			if (i > 0 && buffer[i - 1] == '\r') {
				buffer[i - 1] = '\0';
			}

			if (i + 1 < length) {
				urlChar.push(buffer.get() + i + 1);
			}
		}
	}

	numUrl = urlChar.size();
	std::cout << " successfully added " << numUrl << "urls into the queue.\n";


}


bool Crawler::ResolveAndCheckIP(parsedURL url_part, DWORD& ip, double& dnsTimeMs) {
	// host uniqueness

		// start to do the DNS
		// printf("%8s Doing DNS ...", "");
	std::string ip_addr = url_part.host;
	//std::string resolvedIP {};
	ip = inet_addr(ip_addr.c_str());


	if (ip == INADDR_NONE) {


		auto dnsStart = std::chrono::steady_clock::now();
		hostent* remote = gethostbyname(ip_addr.c_str());
		auto dnsEnd = std::chrono::steady_clock::now();

		dnsTimeMs = std::chrono::duration<double, std::milli>(dnsEnd - dnsStart).count();




		if (remote == nullptr) {
			//printf("failed with %d\n", WSAGetLastError());
			return false;
		}

		// record the successful DNS Lookup
		successDNSLookup++;


		memcpy(&ip, remote->h_addr, remote->h_length);
		//resolvedIP = inet_ntoa(addr);
		//printf(" done in %.2f ms, found %s\n", dnsTimeMs, resolvedIP.c_str());

	}




	//in_addr addr{};
	//addr.s_addr = IP;
	//std::string resolvedIP = inet_ntoa(addr);

	//printf(" done in %.2f ms, found %s\n", dnsTimeMs, resolvedIP.c_str());


	//printf("%8s Checking IP uniqueness... ", "");
	//printf("Host: %s -> IP: %s\n", url_part.host.c_str(), resolvedIP.c_str());
	{
		std::lock_guard<std::mutex> lock(seenIPMutex);
		auto resultIP = seenIPs.insert(ip);
		if (resultIP.second == false) {
			// duplicate host
			return false;
		}
		else {
			// success
			//std::cout << "unique IP found" << std::endl;
			uniqueIPs++;
		}

		return true;
	}
}

void Crawler::StatusCodeCountUpdate(std::string webpageCode) {
	if (webpageCode.empty()) {
		return;
	}

	int statusCode = stoi(webpageCode);
	switch (statusCode / 100) {
		case 2: http2xx++; break;
		case 3: http3xx++; break;
		case 4: http4xx++; break;
		case 5: http5xx++; break;
		default: others++; break;

	}

}
void Crawler::Run() {

	activeThreads++;
	Socket sk(downloadedBytes, webpageDownloadedBytes);

	while (true) {
		//std::string url;
		char* charPtr;
		{
			std::lock_guard<std::mutex> lock(queueMutex);
			//if (urls.empty()) {
			//	break;
			//}

			if (urlChar.empty()) {
				break;
			}

			// move one url from the queue
			// url = urls.front();
			charPtr = urlChar.front();
			//urls.pop();
			urlChar.pop();
			numUrl--;
			extractedUrl++;
		}

		// start to check uniqueness
		//parsedURL url_part = parse_url(url.data());
		parsedURL url_part = parse_url(charPtr);
		if (url_part.isValid == false) {
			printf("The url given is invalid \n");
			continue;
		}
		//	std::cout << "scheme: " << url_part.scheme << " | "
		//<< "isValid: " << std::boolalpha << url_part.isValid << " | "
		//<< "host: " << url_part.host << " | "
		//<< "port: " << url_part.port << " | "
		//<< "path: " << url_part.path << " | "
		//<< "query: " << url_part.query << " | "
		//<< "fragment: " << url_part.fragment << std::endl;

		{
			std::lock_guard<std::mutex> lock(seenHostMutex);

	
			auto result = seenHost.insert(url_part.host);
			if (result.second == false){
				// duplicate host
				continue;
			}
			else {
				// not duplicate host
				uniqueHosts++;
			}
		}

		// checking IP uniqueness
		DWORD ip{};
		double dnsTimeMs{};

		if (!ResolveAndCheckIP(url_part, ip, dnsTimeMs)) {
			// false: duplicate IP detected
			continue;
		}

		in_addr addr{};
		addr.s_addr = ip;
		std::string resolvedIP = inet_ntoa(addr);



		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		// try to connect to the socket
		// original place of socket

		sk.intializeInfo(url_part, resolvedIP);
		bool con1 = sk.connectToServer("robots");
		if (!con1) {
			//printf("Failed to connect to the server");
			continue;
		}
		

		HTTPResponse result_robot  = sk.fetchResponse("/robots.txt");
		if (!result_robot.success) {
			continue; // there are some problem when fetching the result
		}
		// verify status code
		std::string robotCode = parseHeader(result_robot.header);
		// printf("%8s Verifying Header... status code %s\n","", robotCode.c_str());
		if (robotCode.empty() || robotCode[0] != '4') {
			continue;
		}
		successRobotChecks++;


		// connection to pages
		bool con2 = sk.connectToServer("pages");
		if (!con2) {
			//printf("Failed to connect to the server");
			continue;
		}



		HTTPResponse result_webpage = sk.fetchResponse(url_part.resource);
		if (!result_webpage.success) {
			continue;
		}

		// the successfully crawled URL
		successCrawledUrl++;


		std::string webpageCode = parseHeader(result_webpage.header);
		//printf("%8s Verifying Header... status code %s\n", "", webpageCode.c_str());

		StatusCodeCountUpdate(webpageCode);
		if (webpageCode.empty() || webpageCode[0] != '2') {
			continue;
		}

		std::pair<double, std::pair<bool, bool>> parsedInfo = calc_links(result_webpage.content, url_part);
		int numLinks = parsedInfo.first;  //parsedInfo.second: {linkContainTamu, hostContainTamu}
		bool containTamuLink = parsedInfo.second.first;
		bool hostContainTamu = parsedInfo.second.second;



		if (containTamuLink) {
			containTamuLinkCnt++;

			if (!hostContainTamu) {
				hostOutsideTamuCnt++;
			}
		}




		parsedLinks += numLinks;
		//printf("%6s + Parsing page...  done in %.2f ms with %d links \n", "", parseTime, numLinks);


	}

	// the decrement
	activeThreads--;

}

void Crawler::StopInfoPrint() {
	stopStat.store(true);
}

void Crawler::FinalPrint(double elapsedTime) {
	// We have to knwo about the total used to run the program
	int extractedUrlRate = extractedUrl.load() / elapsedTime;
	int dnslookupRate = uniqueHosts.load() / elapsedTime;
	int robotlookupRate = uniqueIPs.load() / elapsedTime;
	int successCrawledUrlRate = successCrawledUrl.load() / elapsedTime;
	int parsedLinksRate = parsedLinks.load() / elapsedTime;
	double webpageMB = webpageDownloadedBytes.load() / 1000000.0;
	double totalMB = downloadedBytes.load() / 1000000.0;
	printf("\n"); // The separation between the PrintStat line and FinalPrint line
	printf("Extracted %d URLs @ %d/s\n", extractedUrl.load(), extractedUrlRate);
	printf("Looked up %d DNS names @ %d/s\n", uniqueHosts.load(), dnslookupRate);
	printf("Attempted %d site robots @ %d/s\n", uniqueIPs.load(), robotlookupRate);
	printf("Crawled %d pages @ %d/s (%.2f MB)\n", successCrawledUrl.load(), successCrawledUrlRate, webpageMB);
	//printf("Total downloaded bytes: %.2f MB\n", totalMB);
	printf("Parsed %d links @ %d/s\n", parsedLinks.load(), parsedLinksRate);
	printf("HTTP Codes: 2xx= %d, 3xx= %d, 4xx= %d, 5xx= %d, other =%d\n",
		http2xx.load(), http3xx.load(), http4xx.load(), http5xx.load(), others.load());

	printf("Link Contain TAMU: %d, Host outside Tamu: %d \n", containTamuLinkCnt.load(), hostOutsideTamuCnt.load());

}

void Crawler::PrintStat() {
	int elapsedSeconds = 0;
	auto lastBytes = downloadedBytes.load();
	auto lastC = successCrawledUrl.load(); // What is this??


	auto lastTime = std::chrono::steady_clock::now();

	while (!stopStat.load()) { // executed when stopStat is false
		std::this_thread::sleep_for(std::chrono::seconds(2));
		elapsedSeconds += 2;
		if (stopStat.load()) {
			break;
		}

		auto currBytes = downloadedBytes.load();
		auto currentTime = std::chrono::steady_clock::now();
		auto currC = successCrawledUrl.load();

		double seconds = std::chrono::duration<double>(
			currentTime - lastTime
		).count();

		double mbps = (currBytes - lastBytes) * 8.0 / seconds / 1000000.0;
		double pps = (currC - lastC) / seconds;


		// 3 character positions for time, 5 for number of threads, 6 character position for Q, 7 for E, 6 for (H, D), 5 for (I, R, C), 4 for L
		printf("[%3d] %5d Q %6d E %7d H %6d D %6d I %5d R %5d C %5d L %4dK \n",
			elapsedSeconds,
			activeThreads.load(),
			numUrl.load(),
			extractedUrl.load(),
			uniqueHosts.load(),
			successDNSLookup.load(),
			uniqueIPs.load(),
			successRobotChecks.load(),
			currC,
			parsedLinks.load()/1000
		);

		printf("***  Crawling %.2f pps @    %.2f Mbps\n", pps, mbps);


		lastBytes = currBytes;
		lastTime = currentTime;
		lastC = currC;

	}

	

}
