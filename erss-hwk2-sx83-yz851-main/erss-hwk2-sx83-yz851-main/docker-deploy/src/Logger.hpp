#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <vector>
#include <string>
#include <cstdio>
using namespace std;

extern std::ofstream file;
extern pthread_mutex_t thread_mutex;
std::string formatCurrentTime();

void writeLog(int id, string msg);

#endif