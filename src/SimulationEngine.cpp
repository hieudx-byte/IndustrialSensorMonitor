#include "SimulationEngine.h"

#include <random>
#include <chrono>

SimulationEngine::SimulationEngine(SensorManager& sensorManager, AlertManager& alertManager,
                                   std::string alertFilePath)
    : m_sensorManager(sensorManager),
      m_alertManager(alertManager),
      m_alertFilePath(std::move(alertFilePath)) {}

// Destructor tu dong dung luong neu van dang chay
SimulationEngine::~SimulationEngine() {
    stop();
}

// Bat luong mo phong
bool SimulationEngine::start(int intervalMs) {
    if (m_running.load()) {
        return false; // Dang chay roi, khong tao them luong moi
    }

    m_intervalMs = intervalMs;
    m_running.store(true);
    m_workerThread = std::thread(&SimulationEngine::runSimulation, this, intervalMs);
    return true;
}

// Dung luong va cho luong ket thuc
void SimulationEngine::stop() {
    if (m_running.exchange(false)) {
        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
    }
}

// Vong lap mo phong cua luong worker
void SimulationEngine::runSimulation(int intervalMs) {
    std::mt19937 rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> dist(-2.5, 2.5);

    while (m_running.load()) {
        // Lay ban sao danh sach cam bien (khong giu khoa mutex lau)
        std::vector<Sensor> currentSensors = m_sensorManager.getAllSensors();

        for (const auto& sensor : currentSensors) {
            if (!m_running.load()) {
                break;
            }

            int id = sensor.getId();
            double oldTemp = sensor.getCurrentTemperature();
            SensorStatus oldStatus = sensor.getStatus();

            double delta = dist(rng);
            double newTemp = oldTemp + delta;

            // Lam tron 2 chu so thap phan
            newTemp = std::round(newTemp * 100.0) / 100.0;

            // Cap nhat nhiet do vao manager
            if (m_sensorManager.updateTemperature(id, newTemp)) {
                auto updatedSensorOpt = m_sensorManager.getSensorById(id);
                if (updatedSensorOpt) {
                    SensorStatus newStatus = updatedSensorOpt->getStatus();
                    // Chi tao canh bao khi trang thai thay doi
                    if (oldStatus != newStatus) {
                        m_alertManager.checkAndRecordAlert(
                            id,
                            updatedSensorOpt->getName(),
                            newTemp,
                            oldStatus,
                            newStatus,
                            m_alertFilePath
                        );
                    }
                }
            }
        }

        // Chia nho thoi gian ngu 50ms de khi goi stop() thi luong dung ngay lap tuc
        int elapsed = 0;
        const int sliceMs = 50;
        while (m_running.load() && elapsed < intervalMs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sliceMs));
            elapsed += sliceMs;
        }
    }
}
