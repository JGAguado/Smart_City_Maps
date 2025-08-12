#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// String utility functions
String repeat(const char* str, int count);
void printProgressBar(int progress, int total, int width = 50);
String formatBytes(size_t bytes);
String formatUptime(unsigned long milliseconds);

#endif // UTILS_H
