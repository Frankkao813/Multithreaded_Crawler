// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#pragma once

#include <stdio.h>
#include <Windows.h>
#include <string> 
#include <iostream> // std::cout
#include <vector> // replacing the C-style charatcer array container
#include<chrono> // used in time calculation, like dns
#include <queue> // used when fetching url from the pool
#include <atomic> // used to calculate the number of elements in queue
#include <mutex> // to protect the resources that is not thread-safe (e.g. queue, set)
// some header to read the files
#include <fstream>
#include <set> // check uniqueness


#include "HTMLParserBase.h"
#include "utility.h"
#include "winsocket.h"
#include "Crawler.h"







#endif //PCH_H
