#include "SensorManager.h"

#include <algorithm>
#include <iomanip>
#include <cctype>

namespace {
    std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }
}

// Them cam bien, kiem tra trung ID
bool SensorManager::addSensor(const Sensor& sensor) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [&sensor](const Sensor& s) { return s.getId() == sensor.getId(); });
    if (it != m_sensors.end()) {
        return false; // ID da ton tai
    }
    m_sensors.push_back(sensor);
    return true;
}

// Xoa cam bien theo ID
bool SensorManager::removeSensor(int id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [id](const Sensor& s) { return s.getId() == id; });
    if (it == m_sensors.end()) {
        return false;
    }
    m_sensors.erase(it);
    return true;
}

// Tim cam bien theo ID, tra ve optional ban sao
std::optional<Sensor> SensorManager::getSensorById(int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [id](const Sensor& s) { return s.getId() == id; });
    if (it == m_sensors.end()) {
        return std::nullopt;
    }
    return *it;
}

// Tim kiem theo ten hoac tu khoa chuoi con
std::vector<Sensor> SensorManager::findSensorsByName(const std::string& nameQuery) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Sensor> matches;
    std::string lowerQuery = toLower(nameQuery);

    for (const auto& sensor : m_sensors) {
        std::string lowerName = toLower(sensor.getName());
        if (lowerName.find(lowerQuery) != std::string::npos) {
            matches.push_back(sensor);
        }
    }
    return matches;
}

bool SensorManager::hasSensor(int id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::any_of(m_sensors.begin(), m_sensors.end(),
                       [id](const Sensor& s) { return s.getId() == id; });
}

bool SensorManager::updateTemperature(int id, double newTemperature) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [id](const Sensor& s) { return s.getId() == id; });
    if (it == m_sensors.end()) {
        return false;
    }
    it->setTemperature(newTemperature);
    return true;
}

bool SensorManager::updateSensorInfo(int id, const std::string& newName,
                                     double newWarningThreshold, double newCriticalThreshold) {
    if (newName.empty() || !Sensor::areThresholdsValid(newWarningThreshold, newCriticalThreshold)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [id](const Sensor& s) { return s.getId() == id; });
    if (it == m_sensors.end()) {
        return false;
    }

    it->setName(newName);
    it->setThresholds(newWarningThreshold, newCriticalThreshold);
    return true;
}

// Lay toan bo danh sach cam bien
std::vector<Sensor> SensorManager::getAllSensors() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sensors;
}

// Loc cam bien theo trang thai
std::vector<Sensor> SensorManager::getSensorsByStatus(SensorStatus status) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Sensor> filtered;
    for (const auto& sensor : m_sensors) {
        if (sensor.getStatus() == status) {
            filtered.push_back(sensor);
        }
    }
    return filtered;
}

// Sap xep cam bien theo nhiet do
std::vector<Sensor> SensorManager::getSensorsSortedByTemperature(SortOrder order) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<Sensor> sorted = m_sensors;
    if (order == SortOrder::ASCENDING) {
        std::sort(sorted.begin(), sorted.end(), [](const Sensor& a, const Sensor& b) {
            return a.getCurrentTemperature() < b.getCurrentTemperature();
        });
    } else {
        std::sort(sorted.begin(), sorted.end(), [](const Sensor& a, const Sensor& b) {
            return a.getCurrentTemperature() > b.getCurrentTemperature();
        });
    }
    return sorted;
}

// Tinh toan thong tin cho dashboard
SensorDashboardStats SensorManager::getDashboardStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    SensorDashboardStats stats;
    stats.total = m_sensors.size();

    if (m_sensors.empty()) {
        return stats;
    }

    const Sensor* maxSensor = &m_sensors[0];

    for (const auto& s : m_sensors) {
        switch (s.getStatus()) {
            case SensorStatus::NORMAL:
                stats.normalCount++;
                break;
            case SensorStatus::WARNING:
                stats.warningCount++;
                break;
            case SensorStatus::CRITICAL:
                stats.criticalCount++;
                break;
        }

        if (s.getCurrentTemperature() > maxSensor->getCurrentTemperature()) {
            maxSensor = &s;
        }
    }

    stats.maxTempSensor = *maxSensor;
    return stats;
}

size_t SensorManager::getSensorCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sensors.size();
}

void SensorManager::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sensors.clear();
}

// In danh sach cam bien dang bang
void SensorManager::printSensorTable(const std::vector<Sensor>& sensors, std::ostream& os) {
    if (sensors.empty()) {
        os << "Khong co cam bien nao de hien thi.\n";
        return;
    }

    os << "+-----+---------------------------+------------+------------+------------+----------+\n";
    os << "| " << std::left << std::setw(3) << "ID"
       << " | " << std::left << std::setw(25) << "Ten Cam Bien"
       << " | " << std::right << std::setw(10) << "Temp (C)"
       << " | " << std::right << std::setw(10) << "Warn (C)"
       << " | " << std::right << std::setw(10) << "Crit (C)"
       << " | " << std::left << std::setw(8) << "TrangThai"
       << " |\n";
    os << "+-----+---------------------------+------------+------------+------------+----------+\n";

    for (const auto& s : sensors) {
        os << "| " << std::left << std::setw(3) << s.getId()
           << " | " << std::left << std::setw(25) << (s.getName().length() > 25 ? s.getName().substr(0, 22) + "..." : s.getName())
           << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << s.getCurrentTemperature()
           << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << s.getWarningThreshold()
           << " | " << std::right << std::fixed << std::setprecision(2) << std::setw(10) << s.getCriticalThreshold()
           << " | " << std::left << std::setw(8) << s.getStatusString()
           << " |\n";
    }
    os << "+-----+---------------------------+------------+------------+------------+----------+\n";
    os << "Tong so: " << sensors.size() << " cam bien.\n";
}
