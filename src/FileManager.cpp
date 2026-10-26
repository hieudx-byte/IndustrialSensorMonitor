#include "FileManager.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

// Cat khoang trang o hai dau chuoi
std::string FileManager::trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

// Parse dong du lieu theo dinh dang: ID|Name|CurrentTemp|WarningThreshold|CriticalThreshold
bool FileManager::parseLine(const std::string& line, Sensor& outSensor) {
    std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed.front() == '#') {
        return false;
    }

    std::stringstream ss(trimmed);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, '|')) {
        tokens.push_back(trim(token));
    }

    if (tokens.size() != 5) {
        return false; // Sai so luong truong
    }

    try {
        // Doc ID
        size_t idIdx = 0;
        int id = std::stoi(tokens[0], &idIdx);
        if (idIdx != tokens[0].length() || id <= 0) {
            return false;
        }

        // Doc ten
        std::string name = tokens[1];
        if (name.empty()) {
            return false;
        }

        // Doc nhiet do
        size_t tempIdx = 0;
        double currentTemp = std::stod(tokens[2], &tempIdx);
        if (tempIdx != tokens[2].length()) {
            return false;
        }

        // Doc nguong canh bao
        size_t warnIdx = 0;
        double warningThreshold = std::stod(tokens[3], &warnIdx);
        if (warnIdx != tokens[3].length()) {
            return false;
        }

        // Doc nguong nguy hiem
        size_t critIdx = 0;
        double criticalThreshold = std::stod(tokens[4], &critIdx);
        if (critIdx != tokens[4].length()) {
            return false;
        }

        // Kiem tra logic nguong
        if (!Sensor::areThresholdsValid(warningThreshold, criticalThreshold)) {
            return false;
        }

        outSensor = Sensor(id, name, currentTemp, warningThreshold, criticalThreshold);
        return true;
    } catch (...) {
        return false; // Loi ep kieu du lieu
    }
}

// Luu danh sach cam bien ra file
FileOperationResult FileManager::saveSensors(const std::string& filePath, const SensorManager& manager) {
    FileOperationResult result;
    try {
        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) {
            result.success = false;
            result.message = "Khong the mo file de ghi: " + filePath;
            return result;
        }

        auto sensors = manager.getAllSensors();
        outFile << "# ID|Name|CurrentTemperature|WarningThreshold|CriticalThreshold\n";
        for (const auto& sensor : sensors) {
            outFile << sensor.getId() << "|"
                    << sensor.getName() << "|"
                    << sensor.getCurrentTemperature() << "|"
                    << sensor.getWarningThreshold() << "|"
                    << sensor.getCriticalThreshold() << "\n";
        }

        outFile.flush();
        result.success = true;
        result.processedCount = sensors.size();
        result.message = "Da luu thanh cong " + std::to_string(sensors.size()) + " cam bien vao " + filePath;
    } catch (const std::exception& e) {
        result.success = false;
        result.message = std::string("Loi khi ghi file: ") + e.what();
    }
    return result;
}

// Doc danh sach cam bien tu file
// Doc vao vector tam truoc, doc thanh cong moi cap nhat vao SensorManager
FileOperationResult FileManager::loadSensors(const std::string& filePath, SensorManager& manager) {
    FileOperationResult result;
    try {
        std::filesystem::path p(filePath);
        if (!std::filesystem::exists(p)) {
            result.success = false;
            result.message = "File khong ton tai: " + filePath;
            return result;
        }

        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            result.success = false;
            result.message = "Khong the mo file de doc: " + filePath;
            return result;
        }

        // Xu ly file rong
        if (std::filesystem::file_size(p) == 0) {
            manager.clear();
            result.success = true;
            result.processedCount = 0;
            result.skippedCount = 0;
            result.message = "File rong, danh sach cam bien duoc lam sach.";
            return result;
        }

        std::vector<Sensor> loadedSensors;
        std::string line;
        size_t skipped = 0;

        while (std::getline(inFile, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty() || trimmed.front() == '#') {
                continue;
            }

            Sensor sensor(1, "temp", 0, 10, 20);
            if (parseLine(trimmed, sensor)) {
                // Kiem tra trung ID trong file
                bool duplicate = std::any_of(loadedSensors.begin(), loadedSensors.end(),
                                             [&sensor](const Sensor& s) { return s.getId() == sensor.getId(); });
                if (!duplicate) {
                    loadedSensors.push_back(sensor);
                } else {
                    skipped++;
                }
            } else {
                skipped++;
            }
        }

        // Doc file xong moi gan vao manager
        manager.clear();
        for (const auto& s : loadedSensors) {
            manager.addSensor(s);
        }

        result.success = true;
        result.processedCount = loadedSensors.size();
        result.skippedCount = skipped;
        result.message = "Da nap thanh cong " + std::to_string(loadedSensors.size()) +
                         " cam bien" + (skipped > 0 ? " (bo qua " + std::to_string(skipped) + " dong loi/trung)" : "");
    } catch (const std::exception& e) {
        result.success = false;
        result.message = std::string("Loi khi doc file: ") + e.what();
    }
    return result;
}
