// return value is a buffer
#include "pch.h"


int readHTMLFile(char* filename, char*& fileBuf, int& fileSize) {
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	// process errors
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with %d\n", GetLastError());
		return 0;
	}

	// get file size
	LARGE_INTEGER li;
	BOOL bRet = GetFileSizeEx(hFile, &li);
	// process errors
	if (bRet == 0)
	{
		printf("GetFileSizeEx error %d\n", GetLastError());
		return 0;
	}

	// read file into a buffer
	fileSize = (DWORD)li.QuadPart;			// assumes file size is below 2GB; otherwise, an __int64 is needed
	DWORD bytesRead;
	// allocate buffer
	fileBuf = new char[fileSize];
	// read into the buffer
	bRet = ReadFile(hFile, fileBuf, fileSize, &bytesRead, NULL);
	// process errors
	if (bRet == 0 || bytesRead != fileSize)
	{
		printf("ReadFile failed with %d\n", GetLastError());
		return 0;
	}

	// done with the file
	CloseHandle(hFile);

	return 1;

}





parsedURL parse_url(const char* url_c) {

	//component is like: scheme://host[:port][/path][?query][#fragment]
	std::string remaining = url_c;
	//printf("URL:  %s \n", remaining.c_str());
	//printf("%8s Parsing URL... ", "");
	parsedURL result;
	// try to break by the following sequence
	
	bool isValid = true;


	// extract the fragment
	std::size_t fragStart = remaining.find("#");
	if (fragStart == std::string::npos) {
		result.fragment = "";
	}
	else {
		result.fragment = remaining.substr(fragStart + 1);
		remaining.erase(fragStart);

	}

	
	result.cleanURL = remaining;


	// extract and validate scheme
	std::size_t schemeEnd = remaining.find("://");
	
	if (schemeEnd == std::string::npos) {
		// std::cout << "No scheme is found" << std::endl;
		result.isValid = false;
		return result;
	}
	else {
		//std::cout << "scheme is being removed" << std::endl;
		result.scheme = remaining.substr(0, schemeEnd);
		remaining.erase(0, schemeEnd + 3);
		if (result.scheme != "http") {
			printf("failed with invalid scheme \n");
			result.isValid = false;
			return result;
		}

	}



	// extract the query
	std::size_t queryStart = remaining.find("?");
	if (queryStart == std::string::npos) {
		result.query = "";
	}
	else {
		result.query = remaining.substr(queryStart + 1);
		remaining.erase(queryStart);
	}

	// extract the path
	std::size_t pathStart = remaining.find("/");

	if (pathStart == std::string::npos) {
		result.path = "/";
	}
	else {
		result.path = remaining.substr(pathStart + 1);
		remaining.erase(pathStart);
	}


	// extract the port
	std::size_t portStart = remaining.find(":");
	if (portStart == std::string::npos) {
		result.port = 80;
	}
	else { // we found the ":", but there might be chance that the port is invalid or not specified
		bool valid = true;
		std::string portText = remaining.substr(portStart + 1);
		int port = 0;
		remaining.erase(portStart);
		if (portText.empty()) {
			valid = false;
		}
		else {
			try { 
				size_t pos;
				// stoi: takes as many characters as possible to form a valid base-n
				port = std::stoi(portText, &pos);
				if (pos != portText.size()) { // e.g. 123abc -> The program only process 123, so reject it
					valid = false;
				}
				if (port < 1 || port > 65535) {
					valid = false;
				}
			}
			catch(...){ // e.g. 0
				valid = false;
			
			}

		}

		if (valid == false) {
			result.isValid = false;
			// printf("failed with invalid port \n");
			return result;

		}

		
		result.port = port;
	
	}

	// extract the rest
	if (remaining == "") {
		result.isValid = false;
		result.host = "";
	}
	else {
		result.host = remaining;
	}


	// resource
	result.resource = result.path;
	if (!result.query.empty()) {
		result.resource = result.path + "?" + result.query;
	}

	if (result.resource.empty() || result.resource[0] != '/') {
		result.resource.insert(0, "/");
	}

	result.isValid = true;
	
	//printf("host %s, port %d, request %s \n", result.host.c_str(), result.port, result.resource.c_str());

	return result;
}


std::string parseHeader(std::string header) {
	// check whether the first five character is HTTP/
	//printf("Verifying header...");
	//size_t httpLoc = header.find(' ')
	//	if (httpLoc != 0) {
	//		printf("")
	//	}
	//


	size_t firstSpace = header.find(' ');
	if (firstSpace == std::string::npos) {
		//printf("Invalid HTTP response");
		return "";
	}

	size_t secondSpace = header.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos) {
		//printf("Invalid HTTP reponse");
		return "";
	}

	// TODO: what if the status text is invalid
	std::string statusText = header.substr(
		firstSpace + 1,
		secondSpace - firstSpace - 1
	);

	return statusText;

}

// fetch the number of links by parsing the content
std::pair<int, std::pair<bool, bool>> calc_links(std::string& content, parsedURL& url_part) {

	std::vector<char> htmlBuffer(content.begin(), content.end());
	htmlBuffer.push_back('\0');

	HTMLParserBase parser;
	int fileSize = content.size();
	std::string base = url_part.cleanURL;
	// std::cout << "The base to be passed in is " << base << std::endl;

	std::vector<char> baseBuffer(base.begin(), base.end());
	baseBuffer.push_back('\0');


	auto parseStart = std::chrono::steady_clock::now();
	int nLinks = 0;
	char* linkBuffer = parser.Parse(htmlBuffer.data(), fileSize, baseBuffer.data(), (int)baseBuffer.size(), &nLinks);

	auto parseEnd = std::chrono::steady_clock::now();
	auto parseTimeMs = std::chrono::duration<double, std::milli>(parseEnd - parseStart).count();


	// additional function calculating whether it is tamu link
	bool linkContainTamu = containTamuLink(linkBuffer, nLinks);
	bool hostContainTamu = hostContainTamuCheck(url_part);
	
	return { nLinks, {linkContainTamu, hostContainTamu}};

}


bool containTamuLink(char* linkBuffer, int& nLinks) {
	// find until there is a tamu.link
	const char* current = linkBuffer;

	for (int i = 0; i < nLinks; i++) {
		//printf("%s\n", current);

		parsedURL currentURL = parse_url(current);
		std::string host = currentURL.host;
		//for (char& c : host) {
		//	c = static_cast<char>(
		//		std::tolower(static_cast<unsigned char>(c))
		//		);
		//}

		const std::string suffix = ".tamu.edu";

		if (host == "tamu.edu" || 
			(host.size() >= suffix.size() &&
				host.compare(host.size() - suffix.size(),
					suffix.size(), suffix) == 0)) {
			return true;
		}

		current += strlen(current) + 1;
	}

	return false;
}


bool hostContainTamuCheck(parsedURL& url_part) {
	std::string host = url_part.host;
	const std::string suffix = ".tamu.edu";


	//for (char& c : host) {
	//	c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	//}


	if (host == "tamu.edu" ||
		(host.size() >= suffix.size() &&
		host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0)) {
		return true;
	}

	return false;
}