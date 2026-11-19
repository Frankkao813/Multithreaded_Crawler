### Intro
This project implements a multithreaded web crawler in C++ designed to download and process web pages concurrently, achieving higher throughput than a sequential crawler.

The main objective is to explore practical concepts in computer networking, multithreaded programming, thread synchronization, and concurrent system design. By distributing URL processing across multiple worker threads, the crawler can handle numerous HTTP requests concurrently and reduce the impact of network latency on overall crawling performance.

The project was originally developed as part of a computer networks programming assignment and is intended to be extended with automated testing, improved build configuration, and dependency management.

About How the porgram is structured, please see the report section: [View Project Report](Crawler_Project/report.pdf)

### Features:
- Multithreaded URL Processing: The number of threads can be configured to support concurrent URL processing. This can be used to conduct performance experiments.
-  Producer-Consumer Architecture: Implements an unbounded, thread-safe URL queue to coordinate work between threads. Worker threads receives URL from the queue and download the webpages and conduct the statistics
- Statistics and Performance Monitoring:　Runs a dedicated statistics thread to periodically report crawling progress.

### Tech Statck
- Language: C++17
- Networking: Windows Sockets API (Winsock2)
- Concurrency: Multithreading, synchronization primitives, and a shared producer-consumer queue
- Development Environment: Visual Studio 2022

### Estimated Improvement:
- HTTPS Support: Extend the existing HTTP client to support secure HTTPS connections using TLS, enabling the crawler to access a broader range of modern websites.
- Automated Testing: Introduce GoogleTest for unit and integration testing, covering URL parsing, HTTP response handling, thread synchronization, and network error scenarios. Integrate CTest and GitHub Actions to automate test execution.
- Standardized Build Environment: Adopt CMake as the build system and vcpkg for dependency management, improving build reproducibility, maintainability, and portability across development environments.