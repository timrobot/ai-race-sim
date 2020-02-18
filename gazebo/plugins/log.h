#pragma once

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

#define ANSI_COLOR_RED      std::string("\033[01;31m")
#define ANSI_COLOR_GREEN    std::string("\033[01;32m")
#define ANSI_COLOR_YELLOW   std::string("\033[01;33m")
#define ANSI_COLOR_BLUE     std::string("\033[01;34m")
#define ANSI_COLOR_MAGENTA  std::string("\033[01;35m")
#define ANSI_COLOR_CYAN     std::string("\033[01;36m")
#define ANSI_COLOR_RESET    std::string("\033[01;0m")

#define LOG_VERBOSE 0
#define LOG_DEBUG   1
#define LOG_INFO    2
#define LOG_WARN    3
#define LOG_ERROR   4

#define LOGV(TAG, ...) __linux_log_print(LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(TAG, ...) __linux_log_print(LOG_DEBUG  , TAG, __VA_ARGS__)
#define LOGI(TAG, ...) __linux_log_print(LOG_INFO   , TAG, __VA_ARGS__)
#define LOGW(TAG, ...) __linux_log_print(LOG_WARN   , TAG, __VA_ARGS__)
#define LOGE(TAG, ...) __linux_log_print(LOG_ERROR  , TAG, __VA_ARGS__)

inline void __linux_log_print(int LOGID, const char *tag, const char *fmt,
    ...) {
  std::string header;
  switch (LOGID) {
    case LOG_VERBOSE: header = ANSI_COLOR_BLUE    + "[V] "; break;
    case LOG_DEBUG:   header = ANSI_COLOR_CYAN    + "[D] "; break;
    case LOG_INFO:    header = ANSI_COLOR_GREEN   + "[I] "; break;
    case LOG_WARN:    header = ANSI_COLOR_YELLOW  + "[W] "; break;
    case LOG_ERROR:   header = ANSI_COLOR_MAGENTA + "[E] "; break;
  }

  uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>
    (std::chrono::system_clock::now().time_since_epoch()).count();
  uint64_t ms = now % 1000;
  std::time_t t = now / 1000;
  std::tm tm = *std::localtime(&t);
  std::stringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d::%H:%M:%S");
  header += ss.str();
  std::string msbuf = std::to_string((int)ms);
  header += std::string(".");
  if (msbuf.size() < 3) {
    header += std::string(3 - msbuf.size(), '0');
  }
  header += msbuf + std::string(22 - ss.str().size(), ' ');

  header += std::string("[") + std::string(tag) + std::string("]  ");
  std::cout << header;

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);

  std::cout << ANSI_COLOR_RESET << std::endl;
}
