### running the project
- Use crtl + shift + B to build the project in debug and release mode
- revise the path at the second line of test_debug.bat and test_release.bat
- build the prject: Ctrl + Shift + B
- In the terminal, enter the HW1/ project to run the code in two modes
	- debug: ./test_debug.bat
	- release: ./test_release.bat


### Architecture
utility.h/utility.cpp
- int readHTMLFile(char* filename, char*& fileBuf, int& fileSize): read the .html file and return the content. 
- parsedURL parse_url(char* url_c): Parse the url into different fields
- std::string parseHeader(std::string header): Parse the 3-digit status code

winsocket.h/winsocket.cpp
- bool Read(void): fetch the response from the server
- bool connectToServer(void): forming request and then connect to server
- bool fetchResponse(std::string& header, std::string& content): forming request and then use Read() to get the header and content

HW1.cpp: the program entry point

### Note: Info for Reference
- Rebuild the project: Ctrl + Shift + B
- accessing the terminal: Crtl + ` (backtick)
- test.bat contains the testcases used to test this program
- When encountering the compiler refuses to include html parser, disable whole program optimization for the project.
- When building Visual Studio cpp project, there will be two locations having x64 files.
One is the same directory as the HW1/, one is inside HW1/. The inside one is the intermediate file, and the same direcotry one contains the final executable


### Some Lessons Learned
1. When fetching the robot.txt and the page content, make sure to clean the buffer, or the program may mistakenly
identify the status code of robot as the page. This can be identified by adding

The wrong buffer looks like this: [404 robot response][200 page response]

2. In HW1-2, in order to test the IP uniqueness, one has to pull the code from the connect function, so there is heavy refactor.

3. (*) Think of some urls that can cover the particular testcase. It is especially important when there is no autograder available. In HW1-2, I need to change the
   the unique IP checking mechanism and the robots page limit in order to continue to the next.

4. Include order matter in pch.h. If the header file is included before custom class, some error might occur.
For instance, inlcuding std::set after Crawler.h will cause problem.

5. Concurrency definitely matters in HW1-3. Wrapping function inside another class makes the main function very clean
