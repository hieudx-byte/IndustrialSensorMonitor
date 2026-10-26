#include "Sensor.h"

std::string sensorStatusToString(SensorStatus status) {
    switch (status) {
        case SensorStatus::NORMAL:
            return "NORMAL";
        case SensorStatus::WARNING:
            return "WARNING";
        case SensorStatus::CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

Sensor::Sensor(int id, std::string name, double currentTemperature,
               double warningThreshold, double criticalThreshold)
    : m_id(id),
      m_name(std::move(name)),
      m_currentTemperature(currentTemperature),
      m_warningThreshold(warningThreshold),
      m_criticalThreshold(criticalThreshold) {
    // Kiem tra du lieu hop le khi tao cam bien
    if (m_id <= 0) {
        throw std::invalid_argument("ID cam bien phai lon hon 0.");
    }
    if (m_name.empty()) {
        throw std::invalid_argument("Ten cam bien khong duoc de trong.");
    }
    if (!areThresholdsValid(m_warningThreshold, m_criticalThreshold)) {
        throw std::invalid_argument("Nguong WARNING phai nho hon nguong CRITICAL.");
    }
}

// Xac dinh trang thai dua vao nhiet do va 2 nguong da dat
SensorStatus Sensor::getStatus() const noexcept {
    if (m_currentTemperature >= m_criticalThreshold) {
        return SensorStatus::CRITICAL;
    }
    if (m_currentTemperature >= m_warningThreshold) {
        return SensorStatus::WARNING;
    }
    return SensorStatus::NORMAL;
}

std::string Sensor::getStatusString() const {
    return sensorStatusToString(getStatus());
}

void Sensor::setTemperature(double temperature) noexcept {
    m_currentTemperature = temperature;
}

void Sensor::setName(const std::string& name) {
    if (name.empty()) {
        throw std::invalid_argument("Ten cam bien khong duoc de trong.");
    }
    m_name = name;
}

bool Sensor::setThresholds(double warningThreshold, double criticalThreshold) {
    if (!areThresholdsValid(warningThreshold, criticalThreshold)) {
        return false;
    }
    m_warningThreshold = warningThreshold;
    m_criticalThreshold = criticalThreshold;
    return true;
}
