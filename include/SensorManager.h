#pragma once

#include "Sensor.h"

#include <vector>
#include <optional>
#include <string>
#include <iostream>
#include <mutex>

enum class SortOrder {
    ASCENDING,   // Tang dan
    DESCENDING   // Giam dan
};

// So lieu thong ke cho dashboard
struct SensorDashboardStats {
    size_t total = 0;
    size_t normalCount = 0;
    size_t warningCount = 0;
    size_t criticalCount = 0;
    std::optional<Sensor> maxTempSensor;
};

// Quan ly danh sach cam bien va cac thao tac tim kiem, cap nhat
class SensorManager {
public:
    SensorManager() = default;

    // Them, xoa, tim kiem
    bool addSensor(const Sensor& sensor);
    bool removeSensor(int id);
    std::optional<Sensor> getSensorById(int id) const;
    std::vector<Sensor> findSensorsByName(const std::string& nameQuery) const;
    bool hasSensor(int id) const;

    // Cap nhat
    bool updateTemperature(int id, double newTemperature);
    bool updateSensorInfo(int id, const std::string& newName,
                          double newWarningThreshold, double newCriticalThreshold);

    // Truy van du lieu
    std::vector<Sensor> getAllSensors() const;
    std::vector<Sensor> getSensorsByStatus(SensorStatus status) const;
    std::vector<Sensor> getSensorsSortedByTemperature(SortOrder order) const;
    SensorDashboardStats getDashboardStats() const;
    size_t getSensorCount() const;
    void clear();

    // In bang ra man hinh
    static void printSensorTable(const std::vector<Sensor>& sensors, std::ostream& os = std::cout);

private:
    std::vector<Sensor> m_sensors;
    mutable std::mutex m_mutex; // Khoa bao ve du lieu khi chay da luong
};
