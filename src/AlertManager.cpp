#include "AlertManager.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>

// Lay thoi gian hien tai theo dinh dang YYYY-MM-DD HH:MM:SS
std::string AlertManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm buf{};
#if defined(_WIN32)
    localtime_s(&buf, &in_time_t);
#else
    localtime_r(&in_time_t, &buf);
#endif
    std::ostringstream ss;
    ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Chi ghi nhan canh bao khi trang thai thuc su thay doi
bool AlertManager::checkAndRecordAlert(int sensorId, const std::string& sensorName,
                                      double temperature, SensorStatus previousStatus,
                                      SensorStatus currentStatus, const std::string& alertFilePath) {
    if (previousStatus == currentStatus) {
        return false; // Trang thai khong doi, bo qua
    }

    AlertRecord record;
    record.sensorId = sensorId;
    record.sensorName = sensorName;
    record.temperature = temperature;
    record.previousStatus = previousStatus;
    record.currentStatus = currentStatus;
    record.timestamp = getCurrentTimestamp();

    addAlert(record, alertFilePath);
    return true;
}

// Them canh bao vao vector va ghi noi tiep vao file neu co
void AlertManager::addAlert(const AlertRecord& alert, const std::string& alertFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_alerts.push_back(alert);

    if (!alertFilePath.empty()) {
        try {
            std::filesystem::path p(alertFilePath);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }

            bool needsHeader = !std::filesystem::exists(p) || (std::filesystem::file_size(p) == 0);
            std::ofstream outFile(alertFilePath, std::ios::out | std::ios::app);
            if (outFile.is_open()) {
                if (needsHeader) {
                    outFile << "# SensorID|Name|Temperature|PreviousStatus|CurrentStatus|Timestamp\n";
                }
                outFile << alert.sensorId << "|"
                        << alert.sensorName << "|"
                        << alert.temperature << "|"
                        << sensorStatusToString(alert.previousStatus) << "|"
                        << sensorStatusToString(alert.currentStatus) << "|"
                        << alert.timestamp << "\n";
            }
        } catch (...) {
            // Bo qua loi ghi file de khong lam dung chuong trinh
        }
    }
}

std::vector<AlertRecord> AlertManager::getAllAlerts() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alerts;
}

// Loc canh bao theo ID cam bien
std::vector<AlertRecord> AlertManager::getAlertsBySensorId(int sensorId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AlertRecord> filtered;
    for (const auto& alert : m_alerts) {
        if (alert.sensorId == sensorId) {
            filtered.push_back(alert);
        }
    }
    return filtered;
}

// Loc canh bao theo muc do trang thai
std::vector<AlertRecord> AlertManager::getAlertsByLevel(SensorStatus level) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<AlertRecord> filtered;
    for (const auto& alert : m_alerts) {
        if (alert.currentStatus == level) {
            filtered.push_back(alert);
        }
    }
    return filtered;
}

size_t AlertManager::getAlertCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_alerts.size();
}

void AlertManager::clearAlerts() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_alerts.clear();
}

// Luu toan bo canh bao ra file
bool AlertManager::saveAlertsToFile(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    try {
        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
        if (!outFile.is_open()) {
            return false;
        }

        outFile << "# SensorID|Name|Temperature|PreviousStatus|CurrentStatus|Timestamp\n";
        for (const auto& a : m_alerts) {
            outFile << a.sensorId << "|"
                    << a.sensorName << "|"
                    << a.temperature << "|"
                    << sensorStatusToString(a.previousStatus) << "|"
                    << sensorStatusToString(a.currentStatus) << "|"
                    << a.timestamp << "\n";
        }
        return true;
    } catch (...) {
        return false;
    }
}

// Doc lich su canh bao tu file
bool AlertManager::loadAlertsFromFile(const std::string& filePath) {
    try {
        if (!std::filesystem::exists(filePath)) {
            return false;
        }

        std::ifstream inFile(filePath);
        if (!inFile.is_open()) {
            return false;
        }

        std::vector<AlertRecord> loaded;
        std::string line;

        while (std::getline(inFile, line)) {
            auto start = std::find_if_not(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); });
            auto end = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char c) { return std::isspace(c); }).base();
            if (start >= end) continue;
            std::string trimmed(start, end);
            if (trimmed.front() == '#') continue;

            std::stringstream ss(trimmed);
            std::string token;
            std::vector<std::string> tokens;
            while (std::getline(ss, token, '|')) {
                auto s = std::find_if_not(token.begin(), token.end(), [](unsigned char c) { return std::isspace(c); });
                auto e = std::find_if_not(token.rbegin(), token.rend(), [](unsigned char c) { return std::isspace(c); }).base();
                tokens.push_back((s < e) ? std::string(s, e) : std::string());
            }

            if (tokens.size() == 6) {
                try {
                    AlertRecord record;
                    record.sensorId = std::stoi(tokens[0]);
                    record.sensorName = tokens[1];
                    record.temperature = std::stod(tokens[2]);

                    auto parseStatus = [](const std::string& st) {
                        if (st == "WARNING") return SensorStatus::WARNING;
                        if (st == "CRITICAL") return SensorStatus::CRITICAL;
                        return SensorStatus::NORMAL;
                    };

                    record.previousStatus = parseStatus(tokens[3]);
                    record.currentStatus = parseStatus(tokens[4]);
                    record.timestamp = tokens[5];
                    loaded.push_back(record);
                } catch (...) {
                    // Bo qua dong loi
                }
            }
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_alerts = std::move(loaded);
        return true;
    } catch (...) {
        return false;
    }
}

// In bang lich su canh bao
void AlertManager::printAlertTable(const std::vector<AlertRecord>& alerts, std::ostream& os) {
    if (alerts.empty()) {
        os << "Khong co lich su canh bao nao.\n";
        return;
    }

    os << "+---------------------+-----+---------------------------+------------+-------------------------+\n";
    os << "| " << std::left << std::setw(19) << "Thoi Gian"
       << " | " << std::left << std::setw(3) << "ID"
       << " | " << std::left << std::setw(25) << "Ten Cam Bien"
       << " | " << std::right << std::setw(10) << "Temp (C)"
       << " | " << std::left << std::setw(23) << "Chuyen Trang Thai"
       << " |\n";
    os << "+---------------------+-----+---------------------------+------------+-------------------------+\n";

    for (const auto& a : alerts) {
        std::string transition = sensorStatusToString(a.previousStatus) + " -> " + sensorStatusToString(a.currentStatus);
        os << "| " << std::left << std::setw(19) << a.timestamp
           << " | " << std::left << std::setw(3) << a.sensorId
           << " | " << std::left << std::setw(25) << (a.sensorName.length() > 25 ? a.sensorName.substr(0, 22) + "..." : a.sensorName)
           << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << a.temperature
           << " | " << std::left << std::setw(23) << transition
           << " |\n";
    }
    os << "+---------------------+-----+---------------------------+------------+-------------------------+\n";
    os << "Tong so canh bao: " << alerts.size() << "\n";
}
