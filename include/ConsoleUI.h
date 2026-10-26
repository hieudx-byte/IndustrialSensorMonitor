#pragma once

#include "SensorManager.h"
#include "AlertManager.h"
#include "FileManager.h"
#include "SimulationEngine.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <string>
#include <cctype>

// Lop ConsoleUI quan ly giao dien menu va nhap lieu
class ConsoleUI {
public:
    static void clearInputBuffer() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Nhap so nguyen co kiem tra hop le
    static int readInt(const std::string& prompt) {
        int value = 0;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                clearInputBuffer();
                return value;
            }
            std::cout << "[LOI] Vui long nhap mot so nguyen.\n";
            clearInputBuffer();
        }
    }

    // Nhap so thuc co kiem tra hop le
    static double readDouble(const std::string& prompt) {
        double value = 0.0;
        while (true) {
            std::cout << prompt;
            if (std::cin >> value) {
                clearInputBuffer();
                return value;
            }
            std::cout << "[LOI] Vui long nhap mot so thuc.\n";
            clearInputBuffer();
        }
    }

    // Nhap chuoi khong rong va khong chua dau '|'
    static std::string readNonEmptyString(const std::string& prompt) {
        std::string value;
        while (true) {
            std::cout << prompt;
            std::getline(std::cin, value);
            size_t start = value.find_first_not_of(" \t\r\n");
            size_t end = value.find_last_not_of(" \t\r\n");
            if (start != std::string::npos && end != std::string::npos) {
                std::string trimmed = value.substr(start, end - start + 1);
                if (!trimmed.empty() && trimmed.find('|') == std::string::npos) {
                    return trimmed;
                }
                if (trimmed.find('|') != std::string::npos) {
                    std::cout << "[LOI] Ten khong duoc chua ky tu '|'. Vui long nhap lai.\n";
                    continue;
                }
            }
            std::cout << "[LOI] Khong duoc de trong. Vui long nhap lai.\n";
        }
    }

    // Xac nhan hanh dong (y/n)
    static bool confirmAction(const std::string& prompt) {
        while (true) {
            std::cout << prompt;
            std::string answer;
            std::getline(std::cin, answer);
            if (!answer.empty()) {
                char c = static_cast<char>(std::tolower(static_cast<unsigned char>(answer[0])));
                if (c == 'y') return true;
                if (c == 'n') return false;
            }
            std::cout << "Vui long nhap 'y' (Dong y) hoac 'n' (Huy bo).\n";
        }
    }

    static void printHeader(const std::string& title) {
        std::cout << "\n======================================================\n";
        std::cout << "  " << title << "\n";
        std::cout << "======================================================\n";
    }

    // Bang thong ke tong quan o dau menu
    static void displayDashboard(const SensorManager& sensorMgr, bool isSimRunning) {
        SensorDashboardStats stats = sensorMgr.getDashboardStats();

        std::cout << "\n========================================================================\n";
        std::cout << "                INDUSTRIAL SENSOR MONITORING DASHBOARD                  \n";
        std::cout << "========================================================================\n";
        std::cout << "  Tong so cam bien: " << std::setw(2) << stats.total
                  << " | NORMAL: " << std::setw(2) << stats.normalCount
                  << " | WARNING: " << std::setw(2) << stats.warningCount
                  << " | CRITICAL: " << std::setw(2) << stats.criticalCount << "\n";

        if (stats.maxTempSensor.has_value()) {
            std::cout << "  Nhiet do cao nhat: " << std::fixed << std::setprecision(2)
                      << stats.maxTempSensor->getCurrentTemperature() << " C (Cam bien ["
                      << stats.maxTempSensor->getId() << "] '"
                      << stats.maxTempSensor->getName() << "')\n";
        } else {
            std::cout << "  Nhiet do cao nhat: Khong co du lieu cam bien.\n";
        }

        std::cout << "  Trang thai mo phong : ["
                  << (isSimRunning ? "DANG CHAY (Luong ngam)" : "DANG DUNG") << "]\n";
        std::cout << "========================================================================\n";
    }

    static void displayMenu() {
        std::cout << " 1.  Hien thi toan bo cam bien\n";
        std::cout << " 2.  Them cam bien moi\n";
        std::cout << " 3.  Cap nhat nhiet do (Thu cong)\n";
        std::cout << " 4.  Chinh sua thong tin & nguong canh bao\n";
        std::cout << " 5.  Xoa cam bien (Co xac nhan)\n";
        std::cout << " 6.  Tim kiem cam bien (Theo ID hoac theo Ten)\n";
        std::cout << " 7.  Loc cam bien theo trang thai (NORMAL/WARNING/CRITICAL)\n";
        std::cout << " 8.  Sap xep cam bien theo nhiet do\n";
        std::cout << " 9.  Xem va loc lich su canh bao (Alert Log)\n";
        std::cout << " 10. Bat mo phong nhiet do thoi gian thuc\n";
        std::cout << " 11. Tat mo phong nhiet do thoi gian thuc\n";
        std::cout << " 12. Luu du lieu ra file\n";
        std::cout << " 13. Nap du lieu tu file\n";
        std::cout << " 0.  Luu du lieu va Thoat\n";
        std::cout << "------------------------------------------------------------------------\n";
    }

    static void handleDisplayAll(const SensorManager& sensorMgr) {
        printHeader("DANH SACH TOAN BO CAM BIEN DANG QUAN LY");
        auto sensors = sensorMgr.getAllSensors();
        SensorManager::printSensorTable(sensors);
    }

    static void handleAddSensor(SensorManager& sensorMgr) {
        printHeader("THEM CAM BIEN MOI");
        int id = readInt("Nhap ma ID cam bien (so nguyen duong > 0): ");
        if (id <= 0) {
            std::cout << "[LOI] ID phai lon hon 0.\n";
            return;
        }

        if (sensorMgr.hasSensor(id)) {
            std::cout << "[LOI] Cam bien voi ID " << id << " da ton tai! Tu choi them trung lap.\n";
            return;
        }

        std::string name = readNonEmptyString("Nhap ten cam bien: ");
        double temp = readDouble("Nhap nhiet do hien tai (C): ");
        double warning = readDouble("Nhap nguong CANH BAO (WARNING C): ");
        double critical = readDouble("Nhap nguong NGUY HIEM (CRITICAL C): ");

        if (!Sensor::areThresholdsValid(warning, critical)) {
            std::cout << "[LOI] Nguong WARNING (" << warning
                      << ") phai nho hon nguong CRITICAL (" << critical << ").\n";
            return;
        }

        try {
            Sensor s(id, name, temp, warning, critical);
            if (sensorMgr.addSensor(s)) {
                std::cout << "[THANH CONG] Da them cam bien moi. Trang thai ban dau: " << s.getStatusString() << "\n";
            } else {
                std::cout << "[LOI] Khong the them cam bien do trung lap ma ID.\n";
            }
        } catch (const std::exception& e) {
            std::cout << "[LOI] " << e.what() << "\n";
        }
    }

    static void handleUpdateTemperature(SensorManager& sensorMgr, AlertManager& alertMgr, const std::string& alertPath) {
        printHeader("CAP NHAT NHIET DO CAM BIEN (THU CONG)");
        int id = readInt("Nhap ID cam bien can cap nhat: ");
        auto sensorOpt = sensorMgr.getSensorById(id);
        if (!sensorOpt) {
            std::cout << "[LOI] Khong tim thay cam bien voi ID " << id << "!\n";
            return;
        }

        SensorStatus prevStatus = sensorOpt->getStatus();
        std::cout << "Cam bien hien tai: [" << sensorOpt->getId() << "] " << sensorOpt->getName()
                  << " | Nhiet do: " << sensorOpt->getCurrentTemperature()
                  << " C | Trang thai: " << sensorOpt->getStatusString() << "\n";

        double newTemp = readDouble("Nhap nhiet do moi (C): ");
        if (sensorMgr.updateTemperature(id, newTemp)) {
            auto updated = sensorMgr.getSensorById(id);
            if (updated) {
                SensorStatus newStatus = updated->getStatus();
                std::cout << "[THANH CONG] Da cap nhat nhiet do. Trang thai moi: " << updated->getStatusString() << "\n";
                // Ghi nhan canh bao neu co doi trang thai
                if (prevStatus != newStatus) {
                    alertMgr.checkAndRecordAlert(id, updated->getName(), newTemp,
                                                 prevStatus, newStatus, alertPath);
                    std::cout << "[GHI NHAN CANH BAO] Chuyen trang thai tu " << sensorStatusToString(prevStatus)
                              << " -> " << sensorStatusToString(newStatus) << " da duoc luu vao lich su.\n";
                }
            }
        } else {
            std::cout << "[LOI] Cap nhat nhiet do that bai.\n";
        }
    }

    static void handleEditSensor(SensorManager& sensorMgr) {
        printHeader("CHINH SUA THONG TIN & NGUONG CAM BIEN");
        int id = readInt("Nhap ID cam bien can sua: ");
        auto sensorOpt = sensorMgr.getSensorById(id);
        if (!sensorOpt) {
            std::cout << "[LOI] Khong tim thay cam bien voi ID " << id << "!\n";
            return;
        }

        std::cout << "Dang sua cam bien: [" << sensorOpt->getId() << "] " << sensorOpt->getName() << "\n";
        std::string newName = readNonEmptyString("Nhap ten moi cho cam bien: ");
        double newWarning = readDouble("Nhap nguong CANH BAO moi (WARNING C): ");
        double newCritical = readDouble("Nhap nguong NGUY HIEM moi (CRITICAL C): ");

        if (!Sensor::areThresholdsValid(newWarning, newCritical)) {
            std::cout << "[LOI] Nguong WARNING (" << newWarning
                      << ") phai nho hon nguong CRITICAL (" << newCritical << ").\n";
            return;
        }

        if (sensorMgr.updateSensorInfo(id, newName, newWarning, newCritical)) {
            std::cout << "[THANH CONG] Thong tin cam bien da duoc cap nhat thanh cong.\n";
        } else {
            std::cout << "[LOI] Cap nhat thong tin cam bien that bai.\n";
        }
    }

    static void handleDeleteSensor(SensorManager& sensorMgr) {
        printHeader("XOA CAM BIEN (CO XAC NHAN)");
        int id = readInt("Nhap ID cam bien muon xoa: ");
        auto sensorOpt = sensorMgr.getSensorById(id);
        if (!sensorOpt) {
            std::cout << "[LOI] Khong tim thay cam bien voi ID " << id << ".\n";
            return;
        }

        std::string promptMsg = "Ban co CHAC CHAN muon xoa cam bien [" + std::to_string(id) +
                                "] '" + sensorOpt->getName() + "' khong? (y/n): ";
        if (!confirmAction(promptMsg)) {
            std::cout << "[THONG BAO] Da huy thao tac xoa cam bien.\n";
            return;
        }

        if (sensorMgr.removeSensor(id)) {
            std::cout << "[THANH CONG] Cam bien ID " << id << " da duoc xoa.\n";
        } else {
            std::cout << "[LOI] Xoa cam bien that bai.\n";
        }
    }

    static void handleFindSensor(const SensorManager& sensorMgr) {
        printHeader("TIM KIEM CAM BIEN");
        std::cout << "Chon phuong thuc tim kiem:\n";
        std::cout << " 1. Tim kiem theo ID\n";
        std::cout << " 2. Tim kiem theo Ten\n";
        int choice = readInt("Nhap lua chon (1-2): ");

        if (choice == 1) {
            int id = readInt("Nhap ID can tim: ");
            auto sensorOpt = sensorMgr.getSensorById(id);
            if (sensorOpt) {
                std::cout << "[TIM THAY] Thong tin chi tiet:\n";
                SensorManager::printSensorTable({*sensorOpt});
            } else {
                std::cout << "[THONG BAO] Khong tim thay cam bien ID " << id << ".\n";
            }
        } else if (choice == 2) {
            std::string query = readNonEmptyString("Nhap ten hoac tu khoa: ");
            auto results = sensorMgr.findSensorsByName(query);
            if (!results.empty()) {
                std::cout << "[TIM THAY] Danh sach cam bien phu hop:\n";
                SensorManager::printSensorTable(results);
            } else {
                std::cout << "[THONG BAO] Khong co cam bien nao co ten chua tu khoa '" << query << "'.\n";
            }
        } else {
            std::cout << "[LOI] Lua chon khong hop le.\n";
        }
    }

    static void handleFilterByStatus(const SensorManager& sensorMgr) {
        printHeader("LOC CAM BIEN THEO TRANG THAI");
        std::cout << "Chon trang thai:\n";
        std::cout << " 1. NORMAL\n";
        std::cout << " 2. WARNING\n";
        std::cout << " 3. CRITICAL\n";
        int choice = readInt("Nhap lua chon (1-3): ");

        SensorStatus targetStatus;
        if (choice == 1) {
            targetStatus = SensorStatus::NORMAL;
        } else if (choice == 2) {
            targetStatus = SensorStatus::WARNING;
        } else if (choice == 3) {
            targetStatus = SensorStatus::CRITICAL;
        } else {
            std::cout << "[LOI] Trang thai khong hop le.\n";
            return;
        }

        auto filtered = sensorMgr.getSensorsByStatus(targetStatus);
        std::cout << "\nDanh sach cam bien trang thai [" << sensorStatusToString(targetStatus) << "]:\n";
        SensorManager::printSensorTable(filtered);
    }

    static void handleSortByTemperature(const SensorManager& sensorMgr) {
        printHeader("SAP XEP CAM BIEN THEO NHIET DO");
        std::cout << "Chon chieu sap xep:\n";
        std::cout << " 1. Tang dan\n";
        std::cout << " 2. Giam dan\n";
        int choice = readInt("Nhap lua chon (1-2): ");

        SortOrder order = (choice == 2) ? SortOrder::DESCENDING : SortOrder::ASCENDING;
        auto sorted = sensorMgr.getSensorsSortedByTemperature(order);
        std::cout << "\nDanh sach sau khi sap xep:\n";
        SensorManager::printSensorTable(sorted);
    }

    static void handleViewAlertHistory(const AlertManager& alertMgr) {
        printHeader("LICH SU CANH BAO");
        std::cout << "Chon che do xem:\n";
        std::cout << " 1. Xem toan bo lich su\n";
        std::cout << " 2. Loc theo ID cam bien\n";
        std::cout << " 3. Loc theo muc do canh bao (WARNING / CRITICAL / NORMAL)\n";
        int choice = readInt("Nhap lua chon (1-3): ");

        if (choice == 1) {
            auto alerts = alertMgr.getAllAlerts();
            AlertManager::printAlertTable(alerts);
        } else if (choice == 2) {
            int id = readInt("Nhap ID cam bien: ");
            auto alerts = alertMgr.getAlertsBySensorId(id);
            std::cout << "\nLich su canh bao cua ID " << id << ":\n";
            AlertManager::printAlertTable(alerts);
        } else if (choice == 3) {
            std::cout << "Chon muc do:\n 1. NORMAL\n 2. WARNING\n 3. CRITICAL\n";
            int lvlChoice = readInt("Nhap muc do (1-3): ");
            SensorStatus lvl = (lvlChoice == 2) ? SensorStatus::WARNING :
                               (lvlChoice == 3) ? SensorStatus::CRITICAL : SensorStatus::NORMAL;
            auto alerts = alertMgr.getAlertsByLevel(lvl);
            std::cout << "\nLich su canh bao muc do [" << sensorStatusToString(lvl) << "]:\n";
            AlertManager::printAlertTable(alerts);
        } else {
            std::cout << "[LOI] Lua chon khong hop le.\n";
        }
    }

    static void handleStartSimulation(SimulationEngine& engine) {
        printHeader("BAT MO PHONG NHIET DO");
        if (engine.isRunning()) {
            std::cout << "[THONG BAO] Luong mo phong da dang chay.\n";
            return;
        }
        int interval = readInt("Nhap chu ky mo phong (ms, mac dinh 1500, min 200): ");
        if (interval < 200) {
            interval = 200;
            std::cout << "[THONG BAO] Chinh ve nguong toi thieu: 200ms\n";
        }

        if (engine.start(interval)) {
            std::cout << "[THANH CONG] Da bat mo phong voi chu ky " << interval << "ms.\n";
        } else {
            std::cout << "[LOI] Bat mo phong that bai.\n";
        }
    }

    static void handleStopSimulation(SimulationEngine& engine) {
        printHeader("TAT MO PHONG NHIET DO");
        if (!engine.isRunning()) {
            std::cout << "[THONG BAO] Luong mo phong hien khong chay.\n";
            return;
        }
        engine.stop();
        std::cout << "[THANH CONG] Da tat mo phong.\n";
    }

    static void handleSaveData(const SensorManager& sensorMgr, const AlertManager& alertMgr,
                               const std::string& sensorPath, const std::string& alertPath) {
        printHeader("LUU DU LIEU RA FILE");
        auto resSensors = FileManager::saveSensors(sensorPath, sensorMgr);
        if (resSensors.success) {
            std::cout << "[THANH CONG] " << resSensors.message << "\n";
        } else {
            std::cout << "[LOI] " << resSensors.message << "\n";
        }

        if (alertMgr.saveAlertsToFile(alertPath)) {
            std::cout << "[THANH CONG] Da luu " << alertMgr.getAlertCount()
                      << " ban ghi canh bao vao " << alertPath << "\n";
        } else {
            std::cout << "[LOI] Khong the luu lich su canh bao.\n";
        }
    }

    static void handleLoadData(SensorManager& sensorMgr, AlertManager& alertMgr,
                               const std::string& sensorPath, const std::string& alertPath) {
        printHeader("NAP DU LIEU TU FILE");
        auto resSensors = FileManager::loadSensors(sensorPath, sensorMgr);
        if (resSensors.success) {
            std::cout << "[THANH CONG] " << resSensors.message << "\n";
        } else {
            std::cout << "[LOI] " << resSensors.message << "\n";
        }

        if (alertMgr.loadAlertsFromFile(alertPath)) {
            std::cout << "[THANH CONG] Da nap " << alertMgr.getAlertCount()
                      << " ban ghi canh bao tu " << alertPath << "\n";
        } else {
            std::cout << "[THONG BAO] Khong co ban ghi canh bao nao tu " << alertPath << "\n";
        }
    }
};
