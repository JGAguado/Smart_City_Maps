#include "utils.h"
#include "serial_config.h"  // Must be included first

String repeat(const char* str, int count) {
    String result = "";
    for (int i = 0; i < count; i++) {
        result += str;
    }
    return result;
}

void printProgressBar(int progress, int total, int width) {
    int pos = (progress * width) / total;
    Serial.print("[");
    for (int i = 0; i < width; i++) {
        if (i < pos) Serial.print("=");
        else if (i == pos) Serial.print(">");
        else Serial.print(" ");
    }
    Serial.printf("] %d%% (%d/%d)\n", (progress * 100) / total, progress, total);
}

String formatBytes(size_t bytes) {
    if (bytes < 1024) {
        return String(bytes) + " B";
    } else if (bytes < 1024 * 1024) {
        return String(bytes / 1024.0, 1) + " KB";
    } else {
        return String(bytes / (1024.0 * 1024.0), 1) + " MB";
    }
}

String formatUptime(unsigned long milliseconds) {
    unsigned long seconds = milliseconds / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;
    
    if (days > 0) {
        return String(days) + "d " + String(hours % 24) + "h " + String(minutes % 60) + "m";
    } else if (hours > 0) {
        return String(hours) + "h " + String(minutes % 60) + "m " + String(seconds % 60) + "s";
    } else if (minutes > 0) {
        return String(minutes) + "m " + String(seconds % 60) + "s";
    } else {
        return String(seconds) + "s";
    }
}
