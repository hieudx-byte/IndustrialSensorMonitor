#include "SensorManager.h"
#include "AlertManager.h"
#include "FileManager.h"
#include "SimulationEngine.h"
#include "ConsoleUI.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>

namespace {

// Duong dan file du lieu
const std::string SENSORS_DATA_PATH = "data/sensors.txt";
const std::string ALERTS_DATA_PATH = "data/alerts.txt";

// Macro kiem tra dieu kien test
#define RUN_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] Dong " << __LINE__ << ": " << msg << "\n"; \
            return 1; \
        } \
    } while (0)

// Ham chay tat ca cac bai test don vi
int runAllUnitTests() {
    std::cout << "========================================================\n";
    std::cout << "  RUNNING INDUSTRIAL SENSOR MONITORING SYSTEM TESTS     \n";
    std::cout << "========================================================\n";

    // 1. Test Sensor
    std::cout << "[SUITE 1/5] Sensor Classification & Boundary Tests...\n";
    {
        Sensor s(1, "Test Sensor", 30.0, 50.0, 80.0);
        RUN_ASSERT(s.getStatus() == SensorStatus::NORMAL, "30.0 phai la NORMAL");
        s.setTemperature(65.0);
        RUN_ASSERT(s.getStatus() == SensorStatus::WARNING, "65.0 phai la WARNING");
        s.setTemperature(85.0);
        RUN_ASSERT(s.getStatus() == SensorStatus::CRITICAL, "85.0 phai la CRITICAL");

        // Gia tri tai dung nguong
        s.setTemperature(50.0);
        RUN_ASSERT(s.getStatus() == SensorStatus::WARNING, "Gia tri tai nguong warning phai la WARNING");
        s.setTemperature(80.0);
        RUN_ASSERT(s.getStatus() == SensorStatus::CRITICAL, "Gia tri tai nguong critical phai la CRITICAL");

        // Nguong khong hop le
        bool caught = false;
        try { Sensor bad(2, "Bad", 10.0, 80.0, 50.0); } catch (...) { caught = true; }
        RUN_ASSERT(caught, "Constructor phai tu choi khi warning >= critical");

        // Nhiet do am
        Sensor cold(3, "Kho Lanh A", -25.0, -10.0, 0.0);
        RUN_ASSERT(cold.getStatus() == SensorStatus::NORMAL, "-25 C phai la NORMAL");
        cold.setTemperature(-5.0);
        RUN_ASSERT(cold.getStatus() == SensorStatus::WARNING, "-5 C phai la WARNING");
        cold.setTemperature(5.0);
        RUN_ASSERT(cold.getStatus() == SensorStatus::CRITICAL, "+5 C phai la CRITICAL");
    }
    std::cout << "  -> PASSED!\n\n";

    // 2. Test SensorManager
    std::cout << "[SUITE 2/5] SensorManager CRUD, Search by Name & Dashboard Stats...\n";
    {
        SensorManager mgr;
        RUN_ASSERT(mgr.addSensor(Sensor(1, "Reactor Boiler 1", 20.0, 50.0, 80.0)), "Them cam bien 1");
        RUN_ASSERT(mgr.addSensor(Sensor(2, "Coolant Pump Alpha", 60.0, 50.0, 80.0)), "Them cam bien 2");
        RUN_ASSERT(!mgr.addSensor(Sensor(1, "Duplicate ID", 30.0, 50.0, 80.0)), "Tu choi ID trung");
        RUN_ASSERT(mgr.getSensorCount() == 2, "Tong so phai la 2");

        // Tim kiem theo ten
        auto searchBoiler = mgr.findSensorsByName("boiler");
        RUN_ASSERT(searchBoiler.size() == 1 && searchBoiler[0].getId() == 1, "Tim theo ten 'boiler'");
        auto searchAlpha = mgr.findSensorsByName("ALPHA");
        RUN_ASSERT(searchAlpha.size() == 1 && searchAlpha[0].getId() == 2, "Tim khong phan biet hoa thuong");

        // Thong ke dashboard
        mgr.addSensor(Sensor(3, "Turbine High Temp", 95.0, 50.0, 80.0));
        SensorDashboardStats stats = mgr.getDashboardStats();
        RUN_ASSERT(stats.total == 3, "Total phai la 3");
        RUN_ASSERT(stats.normalCount == 1, "Normal count phai la 1");
        RUN_ASSERT(stats.warningCount == 1, "Warning count phai la 1");
        RUN_ASSERT(stats.criticalCount == 1, "Critical count phai la 1");
        RUN_ASSERT(stats.maxTempSensor.has_value() && stats.maxTempSensor->getId() == 3, "Max temp sensor la ID 3");

        // Sap xep
        auto asc = mgr.getSensorsSortedByTemperature(SortOrder::ASCENDING);
        RUN_ASSERT(asc.front().getCurrentTemperature() <= asc.back().getCurrentTemperature(), "Sap xep tang dan");

        // Xoa
        RUN_ASSERT(mgr.removeSensor(1), "Xoa ID 1");
        RUN_ASSERT(!mgr.hasSensor(1), "ID 1 khong con ton tai");
    }
    std::cout << "  -> PASSED!\n\n";

    // 3. Test FileManager
    std::cout << "[SUITE 3/5] FileManager Persistence, Empty File & Resiliency Tests...\n";
    {
        std::string testPath = "build/test_sensors_io.txt";
        SensorManager mgr;
        mgr.addSensor(Sensor(101, "Boiler Room 1", 72.5, 80.0, 95.0));
        mgr.addSensor(Sensor(102, "Freezer Room - A", -15.0, -10.0, 0.0));

        RUN_ASSERT(FileManager::saveSensors(testPath, mgr).success, "Luu file");

        SensorManager loaded;
        RUN_ASSERT(FileManager::loadSensors(testPath, loaded).success, "Nap file");
        RUN_ASSERT(loaded.getSensorCount() == 2, "So luong nap vao phai la 2");

        // File khong ton tai thi giu nguyen du lieu cu
        RUN_ASSERT(!FileManager::loadSensors("file_not_found_9999.txt", loaded).success, "File khong ton tai");
        RUN_ASSERT(loaded.getSensorCount() == 2, "Du lieu cu phai giu nguyen");

        // File rong
        std::string emptyPath = "build/empty_file.txt";
        { std::ofstream out(emptyPath); }
        RUN_ASSERT(FileManager::loadSensors(emptyPath, loaded).success, "Nap file rong");
        RUN_ASSERT(loaded.getSensorCount() == 0, "So luong phai la 0");

        std::filesystem::remove(testPath);
        std::filesystem::remove(emptyPath);
    }
    std::cout << "  -> PASSED!\n\n";

    // 4. Test AlertManager
    std::cout << "[SUITE 4/5] Alert Transitions & Filter by ID/Level Tests...\n";
    {
        AlertManager alertMgr;
        std::string alertTestFile = "build/test_alert_event.txt";

        // Khong doi trang thai thi khong tao canh bao
        RUN_ASSERT(!alertMgr.checkAndRecordAlert(1, "S1", 20.0, SensorStatus::NORMAL, SensorStatus::NORMAL, alertTestFile),
                   "Khong tao alert khi khong doi trang thai");

        // Chuyen trang thai
        RUN_ASSERT(alertMgr.checkAndRecordAlert(1, "S1", 55.0, SensorStatus::NORMAL, SensorStatus::WARNING, alertTestFile),
                   "Tao alert khi sang WARNING");
        RUN_ASSERT(alertMgr.checkAndRecordAlert(1, "S1", 85.0, SensorStatus::WARNING, SensorStatus::CRITICAL, alertTestFile),
                   "Tao alert khi sang CRITICAL");
        RUN_ASSERT(alertMgr.checkAndRecordAlert(2, "S2", 60.0, SensorStatus::NORMAL, SensorStatus::WARNING, alertTestFile),
                   "Tao alert cho sensor 2");

        RUN_ASSERT(alertMgr.getAlertCount() == 3, "Tong so alert phai la 3");

        // Loc alert theo ID
        auto alertsId1 = alertMgr.getAlertsBySensorId(1);
        RUN_ASSERT(alertsId1.size() == 2, "Sensor 1 co 2 alert");

        // Loc alert theo muc do
        auto critAlerts = alertMgr.getAlertsByLevel(SensorStatus::CRITICAL);
        RUN_ASSERT(critAlerts.size() == 1, "Co 1 alert CRITICAL");

        if (std::filesystem::exists(alertTestFile)) {
            std::filesystem::remove(alertTestFile);
        }
    }
    std::cout << "  -> PASSED!\n\n";

    // 5. Test SimulationEngine
    std::cout << "[SUITE 5/5] Multithreaded Simulation Engine Lifecycle Tests...\n";
    {
        AlertManager alertMgr;
        SensorManager simMgr;
        simMgr.addSensor(Sensor(1, "Boiler Sim", 30.0, 50.0, 80.0));
        simMgr.addSensor(Sensor(2, "Turbine Sim", 40.0, 60.0, 90.0));

        std::string alertTestFile = "build/test_sim_alerts.txt";
        SimulationEngine engine(simMgr, alertMgr, alertTestFile);

        RUN_ASSERT(engine.start(100), "Start mo phong");
        RUN_ASSERT(engine.isRunning(), "Trang thai running");
        RUN_ASSERT(!engine.start(100), "Tu choi start trung");

        std::this_thread::sleep_for(std::chrono::milliseconds(350));

        engine.stop();
        RUN_ASSERT(!engine.isRunning(), "Mo phong da dung");
        engine.stop();

        RUN_ASSERT(simMgr.getSensorCount() == 2, "So luong cam bien sau mo phong van dung");

        if (std::filesystem::exists(alertTestFile)) {
            std::filesystem::remove(alertTestFile);
        }
    }
    std::cout << "  -> PASSED!\n\n";

    std::cout << "========================================================\n";
    std::cout << "  ALL 5 TEST SUITES PASSED CLEANLY (100% SUCCESS RATE)  \n";
    std::cout << "========================================================\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    // Chay test neu co tham so --test
    if (argc > 1 && std::string(argv[1]) == "--test") {
        return runAllUnitTests();
    }

    SensorManager manager;
    AlertManager alertManager;
    SimulationEngine simEngine(manager, alertManager, ALERTS_DATA_PATH);

    std::cout << "========================================================================\n";
    std::cout << "         INDUSTRIAL SENSOR MONITORING SYSTEM (C++17)                    \n";
    std::cout << "========================================================================\n";

    // Tu dong nap du lieu khi khoi dong neu file ton tai
    if (std::filesystem::exists(SENSORS_DATA_PATH)) {
        auto res = FileManager::loadSensors(SENSORS_DATA_PATH, manager);
        if (res.success) {
            std::cout << "[KHOI DONG] " << res.message << "\n";
        }
    }
    if (std::filesystem::exists(ALERTS_DATA_PATH)) {
        if (alertManager.loadAlertsFromFile(ALERTS_DATA_PATH)) {
            std::cout << "[KHOI DONG] Da nap " << alertManager.getAlertCount()
                      << " ban ghi canh bao tu " << ALERTS_DATA_PATH << "\n";
        }
    }

    bool running = true;
    while (running) {
        ConsoleUI::displayDashboard(manager, simEngine.isRunning());
        ConsoleUI::displayMenu();

        int choice = ConsoleUI::readInt("Chon chuc nang (0-13): ");

        switch (choice) {
            case 1:
                ConsoleUI::handleDisplayAll(manager);
                break;
            case 2:
                ConsoleUI::handleAddSensor(manager);
                break;
            case 3:
                ConsoleUI::handleUpdateTemperature(manager, alertManager, ALERTS_DATA_PATH);
                break;
            case 4:
                ConsoleUI::handleEditSensor(manager);
                break;
            case 5:
                ConsoleUI::handleDeleteSensor(manager);
                break;
            case 6:
                ConsoleUI::handleFindSensor(manager);
                break;
            case 7:
                ConsoleUI::handleFilterByStatus(manager);
                break;
            case 8:
                ConsoleUI::handleSortByTemperature(manager);
                break;
            case 9:
                ConsoleUI::handleViewAlertHistory(alertManager);
                break;
            case 10:
                ConsoleUI::handleStartSimulation(simEngine);
                break;
            case 11:
                ConsoleUI::handleStopSimulation(simEngine);
                break;
            case 12:
                ConsoleUI::handleSaveData(manager, alertManager, SENSORS_DATA_PATH, ALERTS_DATA_PATH);
                break;
            case 13:
                ConsoleUI::handleLoadData(manager, alertManager, SENSORS_DATA_PATH, ALERTS_DATA_PATH);
                break;
            case 0: {
                std::cout << "\n[HE THONG] Dang dung mo phong (neu dang chay)...\n";
                simEngine.stop();

                std::cout << "[HE THONG] Luu du lieu truoc khi thoat...\n";
                FileManager::saveSensors(SENSORS_DATA_PATH, manager);
                alertManager.saveAlertsToFile(ALERTS_DATA_PATH);

                std::cout << "[HE THONG] Da luu xong. Tam biet!\n";
                running = false;
                break;
            }
            default:
                std::cout << "[LOI] Tuy chon khong hop le. Vui long chon tu 0 den 13.\n";
                break;
        }
    }

    return 0;
}
