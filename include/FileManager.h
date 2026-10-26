#pragma once

#include "SensorManager.h"
#include <string>

// Ket qua xu ly file
struct FileOperationResult {
    bool success = false;
    size_t processedCount = 0;
    size_t skippedCount = 0;
    std::string message;
};

// Lop doc va ghi file du lieu cam bien
class FileManager {
public:
    // Luu danh sach cam bien ra file
    static FileOperationResult saveSensors(const std::string& filePath, const SensorManager& manager);

    // Doc danh sach cam bien tu file
    static FileOperationResult loadSensors(const std::string& filePath, SensorManager& manager);

private:
    static std::string trim(const std::string& str);
    static bool parseLine(const std::string& line, Sensor& outSensor);
};
