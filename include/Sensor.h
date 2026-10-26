#pragma once

#include <string>
#include <stdexcept>

// Cac trang thai cua cam bien
enum class SensorStatus {
    NORMAL,    // Binh thuong
    WARNING,   // Canh bao
    CRITICAL   // Nguy hiem
};

// Chuyen enum trang thai sang chuoi de hien thi
std::string sensorStatusToString(SensorStatus status);

// Lop Sensor luu thong tin mot cam bien nhiet do
class Sensor {
public:
    Sensor(int id, std::string name, double currentTemperature,
           double warningThreshold, double criticalThreshold);

    // Getters
    int getId() const noexcept { return m_id; }
    const std::string& getName() const noexcept { return m_name; }
    double getCurrentTemperature() const noexcept { return m_currentTemperature; }
    double getWarningThreshold() const noexcept { return m_warningThreshold; }
    double getCriticalThreshold() const noexcept { return m_criticalThreshold; }

    // Tinh trang thai dua tren nhiet do hien tai va nguong
    SensorStatus getStatus() const noexcept;
    std::string getStatusString() const;

    // Setters
    void setTemperature(double temperature) noexcept;
    void setName(const std::string& name);
    bool setThresholds(double warningThreshold, double criticalThreshold);

    // Nguong canh bao phai nho hon nguong nguy hiem
    static bool areThresholdsValid(double warningThreshold, double criticalThreshold) noexcept {
        return warningThreshold < criticalThreshold;
    }

private:
    int m_id;
    std::string m_name;
    double m_currentTemperature;
    double m_warningThreshold;
    double m_criticalThreshold;
};
