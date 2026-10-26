#include "Sensor.h"
#include "SensorManager.h"
#include "FileManager.h"
#include "AlertManager.h"
#include "SimulationEngine.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <cstdlib>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] Dong " << __LINE__ << ": " << msg << "\n"; \
            std::exit(1); \
        } \
    } while (0)

// 1. Test Sensor
void testStatusClassification() {
    Sensor s(1, "Test Sensor", 30.0, 50.0, 80.0);
    TEST_ASSERT(s.getStatus() == SensorStatus::NORMAL, "30.0 phai la NORMAL");
    s.setTemperature(65.0);
    TEST_ASSERT(s.getStatus() == SensorStatus::WARNING, "65.0 phai la WARNING");
    s.setTemperature(85.0);
    TEST_ASSERT(s.getStatus() == SensorStatus::CRITICAL, "85.0 phai la CRITICAL");

    s.setTemperature(50.0);
    TEST_ASSERT(s.getStatus() == SensorStatus::WARNING, "Gia tri tai dung nguong warning phai la WARNING");
    s.setTemperature(80.0);
    TEST_ASSERT(s.getStatus() == SensorStatus::CRITICAL, "Gia tri tai dung nguong critical phai la CRITICAL");

    bool caught = false;
    try { Sensor bad(2, "Bad", 10.0, 80.0, 50.0); } catch (...) { caught = true; }
    TEST_ASSERT(caught, "Constructor phai tu choi khi warning >= critical");

    Sensor cold(3, "Kho Lanh A", -25.0, -10.0, 0.0);
    TEST_ASSERT(cold.getStatus() == SensorStatus::NORMAL, "-25 C phai la NORMAL");
    cold.setTemperature(-5.0);
    TEST_ASSERT(cold.getStatus() == SensorStatus::WARNING, "-5 C phai la WARNING");
    cold.setTemperature(5.0);
    TEST_ASSERT(cold.getStatus() == SensorStatus::CRITICAL, "+5 C phai la CRITICAL");
}

// 2. Test SensorManager
void testManagerCrudAndSearch() {
    SensorManager mgr;
    TEST_ASSERT(mgr.addSensor(Sensor(1, "Boiler 1", 25.0, 50.0, 80.0)), "Them sensor 1 thanh cong");
    TEST_ASSERT(mgr.addSensor(Sensor(2, "Turbine Alpha", 30.0, 50.0, 80.0)), "Them sensor 2 thanh cong");
    TEST_ASSERT(!mgr.addSensor(Sensor(1, "Dup", 40.0, 60.0, 90.0)), "Tu choi trung ID 1");
    TEST_ASSERT(mgr.getSensorCount() == 2, "So luong phai la 2");

    // Tim theo ID
    auto s1 = mgr.getSensorById(1);
    TEST_ASSERT(s1.has_value() && s1->getName() == "Boiler 1", "Tim theo ID 1 dung");

    // Tim theo ten
    auto searchRes = mgr.findSensorsByName("boiler");
    TEST_ASSERT(searchRes.size() == 1 && searchRes[0].getId() == 1, "Tim theo ten 'boiler'");

    // Dashboard stats
    mgr.addSensor(Sensor(3, "Super Heater", 95.0, 50.0, 80.0));
    SensorDashboardStats stats = mgr.getDashboardStats();
    TEST_ASSERT(stats.total == 3, "Total phai la 3");
    TEST_ASSERT(stats.normalCount == 2, "Normal count phai la 2");
    TEST_ASSERT(stats.criticalCount == 1, "Critical count phai la 1");
    TEST_ASSERT(stats.maxTempSensor.has_value() && stats.maxTempSensor->getId() == 3, "Max temp sensor phai la ID 3");

    // Xoa
    TEST_ASSERT(mgr.removeSensor(1), "Xoa ID 1 thanh cong");
    TEST_ASSERT(!mgr.hasSensor(1), "ID 1 khong con ton tai");
}

// 3. Test FileManager
void testFileManagerResiliency() {
    std::string testPath = "build/test_sensors_temp.txt";
    SensorManager mgr;
    mgr.addSensor(Sensor(101, "Boiler Room 1", 72.5, 80.0, 95.0));
    mgr.addSensor(Sensor(102, "Freezer Room - A", -15.0, -10.0, 0.0));

    auto saveRes = FileManager::saveSensors(testPath, mgr);
    TEST_ASSERT(saveRes.success, "Luu sensor thanh cong");

    SensorManager loadedMgr;
    auto loadRes = FileManager::loadSensors(testPath, loadedMgr);
    TEST_ASSERT(loadRes.success, "Nap sensor thanh cong");
    TEST_ASSERT(loadedMgr.getSensorCount() == 2, "So luong nap vao phai la 2");

    // File khong ton tai
    auto missingRes = FileManager::loadSensors("non_existent_file_99999.txt", loadedMgr);
    TEST_ASSERT(!missingRes.success, "File thieu phai bao loi");
    TEST_ASSERT(loadedMgr.getSensorCount() == 2, "Du lieu cu phai giu nguyen khi nap file loi");

    // File rong
    std::string emptyPath = "build/empty_temp.txt";
    { std::ofstream out(emptyPath); }
    TEST_ASSERT(FileManager::loadSensors(emptyPath, loadedMgr).success, "Nap file rong thanh cong");
    TEST_ASSERT(loadedMgr.getSensorCount() == 0, "So luong sau khi nap file rong phai la 0");

    std::filesystem::remove(testPath);
    std::filesystem::remove(emptyPath);
}

// 4. Test AlertManager
void testAlertManagerFilters() {
    AlertManager alertMgr;
    std::string testAlertFile = "build/test_alerts_temp.txt";

    // Khong doi thi khong tao alert
    TEST_ASSERT(!alertMgr.checkAndRecordAlert(1, "S1", 25.0, SensorStatus::NORMAL, SensorStatus::NORMAL, testAlertFile),
                "Khong tao alert khi khong doi trang thai");

    // Chuyen trang thai
    TEST_ASSERT(alertMgr.checkAndRecordAlert(1, "S1", 55.0, SensorStatus::NORMAL, SensorStatus::WARNING, testAlertFile),
                "Tao alert khi NORMAL sang WARNING");
    TEST_ASSERT(alertMgr.checkAndRecordAlert(1, "S1", 85.0, SensorStatus::WARNING, SensorStatus::CRITICAL, testAlertFile),
                "Tao alert khi WARNING sang CRITICAL");
    TEST_ASSERT(alertMgr.checkAndRecordAlert(2, "S2", 60.0, SensorStatus::NORMAL, SensorStatus::WARNING, testAlertFile),
                "Tao alert cho sensor 2");

    TEST_ASSERT(alertMgr.getAlertCount() == 3, "Tong so alert phai la 3");

    // Loc theo ID
    auto alertsId1 = alertMgr.getAlertsBySensorId(1);
    TEST_ASSERT(alertsId1.size() == 2, "ID 1 phai co 2 alert");

    // Loc theo muc do
    auto critAlerts = alertMgr.getAlertsByLevel(SensorStatus::CRITICAL);
    TEST_ASSERT(critAlerts.size() == 1, "Phai co 1 alert CRITICAL");

    if (std::filesystem::exists(testAlertFile)) {
        std::filesystem::remove(testAlertFile);
    }
}

// 5. Test SimulationEngine
void testSimulationEngineLifecycle() {
    SensorManager mgr;
    mgr.addSensor(Sensor(1, "Boiler Sim", 50.0, 60.0, 80.0));
    mgr.addSensor(Sensor(2, "Turbine Sim", 45.0, 55.0, 75.0));

    AlertManager alertMgr;
    std::string testAlertFile = "build/test_sim_alerts_temp.txt";

    SimulationEngine engine(mgr, alertMgr, testAlertFile);
    TEST_ASSERT(!engine.isRunning(), "Engine khong chay ban dau");

    TEST_ASSERT(engine.start(100), "Start engine thanh cong");
    TEST_ASSERT(engine.isRunning(), "Engine dang chay");
    TEST_ASSERT(!engine.start(100), "Tu choi start trung lap");

    std::this_thread::sleep_for(std::chrono::milliseconds(350));

    engine.stop();
    TEST_ASSERT(!engine.isRunning(), "Engine da dung");
    engine.stop();

    TEST_ASSERT(mgr.getSensorCount() == 2, "Sensor van nguyen ven");

    if (std::filesystem::exists(testAlertFile)) {
        std::filesystem::remove(testAlertFile);
    }
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "     RUNNING COMPREHENSIVE INDUSTRIAL SENSOR TESTS      \n";
    std::cout << "========================================================\n";

    std::cout << "[SUITE 1/5] Running Sensor Unit Tests...\n";
    testStatusClassification();
    std::cout << "  -> PASSED!\n\n";

    std::cout << "[SUITE 2/5] Running SensorManager Unit Tests...\n";
    testManagerCrudAndSearch();
    std::cout << "  -> PASSED!\n\n";

    std::cout << "[SUITE 3/5] Running FileManager Unit Tests...\n";
    testFileManagerResiliency();
    std::cout << "  -> PASSED!\n\n";

    std::cout << "[SUITE 4/5] Running AlertManager Unit Tests...\n";
    testAlertManagerFilters();
    std::cout << "  -> PASSED!\n\n";

    std::cout << "[SUITE 5/5] Running SimulationEngine Unit Tests...\n";
    testSimulationEngineLifecycle();
    std::cout << "  -> PASSED!\n\n";

    std::cout << "========================================================\n";
    std::cout << "  ALL 5 TEST SUITES PASSED CLEANLY (100% SUCCESS RATE)  \n";
    std::cout << "========================================================\n";
    return 0;
}
