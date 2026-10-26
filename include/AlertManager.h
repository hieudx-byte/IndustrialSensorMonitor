#pragma once

#include "Sensor.h"

#include <string>
#include <vector>
#include <mutex>
#include <iostream>

// Thong tin mot ban ghi canh bao
struct AlertRecord {
    int sensorId = 0;
    std::string sensorName;
    double temperature = 0.0;
    SensorStatus previousStatus = SensorStatus::NORMAL;
    SensorStatus currentStatus = SensorStatus::NORMAL;
    std::string timestamp;
};

// Quan ly lich su canh bao cua cac cam bien
class AlertManager {
public:
    AlertManager() = default;

    // Ghi nhan canh bao khi co su thay doi trang thai
    bool checkAndRecordAlert(int sensorId, const std::string& sensorName,
                             double temperature, SensorStatus previousStatus,
                             SensorStatus currentStatus, const std::string& alertFilePath = "");

    void addAlert(const AlertRecord& alert, const std::string& alertFilePath = "");

    // Lay danh sach va loc canh bao
    std::vector<AlertRecord> getAllAlerts() const;
    std::vector<AlertRecord> getAlertsBySensorId(int sensorId) const;
    std::vector<AlertRecord> getAlertsByLevel(SensorStatus level) const;
    size_t getAlertCount() const;
    void clearAlerts();

    // Doc va ghi file lich su canh bao
    bool saveAlertsToFile(const std::string& filePath) const;
    bool loadAlertsFromFile(const std::string& filePath);

    // In bang lich su ra man hinh
    static void printAlertTable(const std::vector<AlertRecord>& alerts, std::ostream& os = std::cout);

    // Lay moc thoi gian hien tai
    static std::string getCurrentTimestamp();

private:
    std::vector<AlertRecord> m_alerts;
    mutable std::mutex m_mutex; // Khoa mutex bao ve danh sach canh bao
};
