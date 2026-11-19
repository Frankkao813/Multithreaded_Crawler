#pragma once


struct parsedURL {
	std::string scheme;
	std::string host;
	int port = 80;
	std::string path = "/";
	std::string query;
	std::string fragment; // everything belongs to the # segment
	std::string resource; // to be passed inside the HTTP request
	std::string cleanURL; // the full path without #, needed when we want to parse the number of links
	bool isValid;
};



int readHTMLFile(char* filename, char*& filebuf, int& filesize);
parsedURL parse_url(const char* url_c);
std::string parseHeader(std::string header);
std::pair<int, std::pair<bool, bool>>  calc_links(std::string& content, parsedURL& url_part); // calculate the number of links

bool containTamuLink(char* linkBuffer, int& nLinks);
bool hostContainTamuCheck(parsedURL& url_part);
