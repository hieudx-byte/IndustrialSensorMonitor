#pragma once

#include "SensorManager.h"
#include "AlertManager.h"

#include <atomic>
#include <thread>
#include <string>

// Luong mo phong nhiet do bien doi ngau nhien
class SimulationEngine {
public:
    SimulationEngine(SensorManager& sensorManager, AlertManager& alertManager,
                     std::string alertFilePath = "data/alerts.txt");
    ~SimulationEngine();

    // Khong cho copy hoac move de an toan luong
    SimulationEngine(const SimulationEngine&) = delete;
    SimulationEngine& operator=(const SimulationEngine&) = delete;
    SimulationEngine(SimulationEngine&&) = delete;
    SimulationEngine& operator=(SimulationEngine&&) = delete;

    // Bat va tat luong mo phong
    bool start(int intervalMs = 1500);
    void stop();

    bool isRunning() const noexcept { return m_running.load(); }
    int getIntervalMs() const noexcept { return m_intervalMs; }

private:
    void runSimulation(int intervalMs);

    SensorManager& m_sensorManager;
    AlertManager& m_alertManager;
    std::string m_alertFilePath;

    std::atomic<bool> m_running{false}; // Co bao dung luong an toan
    std::thread m_workerThread;          // Luong chay ngam
    int m_intervalMs = 1500;
};
