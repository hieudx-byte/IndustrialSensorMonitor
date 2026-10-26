# TÀI LIỆU KIẾN TRÚC & GHI CHÚ KỸ THUẬT DỰ ÁN
## INDUSTRIAL SENSOR MONITORING SYSTEM (C++17)

Tài liệu ghi chú kiến trúc hệ thống, luồng dữ liệu, giải thích thiết kế đa luồng và các câu hỏi kỹ thuật trọng tâm khi trình bày dự án.

---

## MỤC LỤC
1. [Tổng quan kiến trúc dự án (Architecture Overview)](#1-tổng-quan-kiến-trúc-dự-án)
2. [Vai trò và trách nhiệm của từng Class](#2-vai-trò-và-trách-nhiệm-của-từng-class)
3. [Luồng chạy chính từ `main()` đến các Class nghiệp vụ](#3-luồng-chạy-chính-từ-main-đến-các-class-nghiệp-vụ)
4. [Luồng thêm mới một cảm biến (Add Sensor Flow)](#4-luồng-thêm-mới-một-cảm-biến)
5. [Luồng lưu và nạp dữ liệu an toàn (Fault-Tolerant File I/O Flow)](#5-luồng-lưu-và-nạp-dữ-liệu-an-toàn)
6. [Luồng phát sinh và ghi nhận cảnh báo (Alert Event Flow)](#6-luồng-phát-sinh-và-ghi-nhận-cảnh-báo)
7. [Luồng hoạt động và dừng luồng an toàn (Multithreading & Concurrency Flow)](#7-luồng-hoạt-động-và-dừng-luồng-an-toàn)
8. [Các kỹ thuật C++ hiện đại (C++17) quan trọng đã sử dụng](#8-các-kỹ-thuật-c17-quan-trọng-đã-sử-dụng)
9. [Bộ câu hỏi phỏng vấn thực tế và câu trả lời mẫu](#9-bộ-câu-hỏi-phỏng-vấn-thực-tế-và-câu-trả-lời-mẫu)
10. [Gợi ý cách tự giới thiệu dự án ấn tượng với nhà tuyển dụng](#10-gợi-ý-cách-tự-giới-thiệu-dự-án-ấn-tượng-với-nhà-tuyển-dụng)

---

## 1. TỔNG QUAN KIẾN TRÚC DỰ ÁN

Dự án được xây dựng theo mô hình **Layered Architecture** (Kiến trúc phân tầng) kết hợp nguyên lý **Single Responsibility Principle (SRP)** trong SOLID:

```
+-----------------------------------------------------------------------+
|                             ConsoleUI                                 |
|   (Bảng điều khiển Dashboard, Menu, Nhập liệu an toàn, Hiển thị bảng) |
+------------------------------------+----------------------------------+
                                     |
                                     v
+------------------------------------+----------------------------------+
|                   Business Logic & Storage Layer                      |
|  - SensorManager    : Quản lý danh mục cảm biến in-memory             |
|  - AlertManager     : Quản lý lịch sử sự kiện cảnh báo                |
|  - SimulationEngine : Luồng ngầm mô phỏng nhiệt độ thời gian thực     |
|  - FileManager      : Đọc / ghi file có khả năng chịu lỗi             |
+------------------------------------+----------------------------------+
                                     |
                                     v
+------------------------------------+----------------------------------+
|                         Domain Entity (Sensor)                        |
|   (Mã ID, Tên, Nhiệt độ, Ngưỡng cảnh báo, Tính toán trạng thái)       |
+-----------------------------------------------------------------------+
```

### Tại sao lại chia như vậy?
- **Tách biệt giao diện và nghiệp vụ:** `SensorManager` không chứa lệnh `std::cin` hay `std::cout`. Khi cần nâng cấp lên GUI (Qt, Web, v.v.), ta chỉ cần thay thế tầng UI mà giữ nguyên 100% logic nghiệp vụ lõi.
- **Dễ viết Unit Test:** Từng class nghiệp vụ có thể được kiểm thử độc lập mà không cần giả lập luồng nhập từ bàn phím.

---

## 2. VAI TRÒ VÀ TRÁCH NHIỆM CỦA TỪNG CLASS

### 1. `Sensor` ([Sensor.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/Sensor.h) / [Sensor.cpp](file:///C:/CppProjects/IndustrialSensorMonitor/src/Sensor.cpp))
- **Trách nhiệm:** Đại diện cho một thực thể cảm biến công nghiệp duy nhất.
- **Ràng buộc toàn vẹn (Invariants):**
  - $ID > 0$.
  - Tên cảm biến không được để trống.
  - Ngưỡng cảnh báo bắt buộc phải nhỏ hơn ngưỡng nguy hiểm ($T_{\text{warn}} < T_{\text{crit}}$).
- **Tính toán trạng thái:**
  - `NORMAL`: $T < T_{\text{warn}}$
  - `WARNING`: $T_{\text{warn}} \le T < T_{\text{crit}}$
  - `CRITICAL`: $T \ge T_{\text{crit}}$

### 2. `SensorManager` ([SensorManager.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/SensorManager.h) / [SensorManager.cpp](file:///C:/CppProjects/IndustrialSensorMonitor/src/SensorManager.cpp))
- **Trách nhiệm:** Quản lý tập hợp danh sách cảm biến (`std::vector<Sensor>`) trong bộ nhớ RAM.
- **Tính năng:**
  - Thêm cảm biến (chặn trùng ID).
  - Xóa cảm biến theo ID.
  - Cập nhật nhiệt độ và ngưỡng.
  - Tìm kiếm theo ID hoặc tìm kiếm chuỗi con theo Tên (không phân biệt hoa thường).
  - Lọc theo trạng thái và sắp xếp theo nhiệt độ.
  - Tính toán số liệu thống kê Dashboard thời gian thực (`getDashboardStats()`).
- **An toàn đa luồng:** Dùng `mutable std::mutex m_mutex` bảo vệ container khi nhiều luồng truy cập cùng lúc.

### 3. `AlertManager` ([AlertManager.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/AlertManager.h) / [AlertManager.cpp](file:///C:/CppProjects/IndustrialSensorMonitor/src/AlertManager.cpp))
- **Trách nhiệm:** Quản lý lịch sử cảnh báo khi có sự biến động trạng thái.
- **Quy tắc quan trọng:** Chỉ ghi nhận khi `previousStatus != currentStatus` (chặn cảnh báo trùng lặp).
- **Tính năng:** Lọc cảnh báo theo ID cảm biến, lọc theo mức độ (`WARNING`, `CRITICAL`), ghi nối tiếp ra file `data/alerts.txt`.

### 4. `FileManager` ([FileManager.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/FileManager.h) / [FileManager.cpp](file:///C:/CppProjects/IndustrialSensorMonitor/src/FileManager.cpp))
- **Trách nhiệm:** Đọc và ghi dữ liệu ra đĩa an toàn.
- **Định dạng:** Sử dụng ký tự pipe `|` (`ID|Name|Temperature|WarningThreshold|CriticalThreshold`) để hỗ trợ tên cảm biến có chứa khoảng trắng.
- **Cơ chế chịu lỗi:** Đọc dữ liệu vào một vector tạm trước (Two-phase commit). Nếu file hỏng hoặc thiếu, dữ liệu in-memory cũ được giữ nguyên 100%.

### 5. `SimulationEngine` ([SimulationEngine.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/SimulationEngine.h) / [SimulationEngine.cpp](file:///C:/CppProjects/IndustrialSensorMonitor/src/SimulationEngine.cpp))
- **Trách nhiệm:** Chạy một `std::thread` ngầm liên tục tạo độ lệch nhiệt độ ngẫu nhiên từ $-2.5^\circ\text{C}$ đến $+2.5^\circ\text{C}$ cho các cảm biến.
- **Đồng bộ:** Dùng `std::atomic<bool> m_running` để dừng luồng, chia nhỏ sleep thành các lát 50ms để dừng tức thời, RAII destructor tự động join luồng.

### 6. `ConsoleUI` ([ConsoleUI.h](file:///C:/CppProjects/IndustrialSensorMonitor/include/ConsoleUI.h))
- **Trách nhiệm:** Trình bày bảng điều khiển Dashboard, hiển thị menu, in bảng ASCII, và đọc dữ liệu người dùng an toàn (chống tràn bộ đệm cin khi nhập chữ thay vì số).

---

## 3. LUỒNG CHẠY CHÍNH TỪ `main()` ĐẾN CÁC CLASS NGHIỆP VỤ

1. **Khởi động (`main()`):**
   - Khởi tạo `SensorManager`, `AlertManager`, và `SimulationEngine`.
   - Kiểm tra và tự động nạp dữ liệu từ `data/sensors.txt` và `data/alerts.txt` nếu file tồn tại.
2. **Vòng lặp tương tác (While Loop):**
   - Gọi `ConsoleUI::displayDashboard()`: Đọc số liệu thời gian thực (tổng số cảm biến, số lượng theo từng trạng thái, cảm biến có nhiệt độ cao nhất) và hiển thị lên màn hình.
   - Gọi `ConsoleUI::displayMenu()`: In danh mục 14 chức năng (0 đến 13).
   - Nhập lựa chọn người dùng qua `ConsoleUI::readInt()`.
   - Điều hướng (dispatch) đến hàm xử lý tương ứng trong `ConsoleUI`.
3. **Thoát chương trình (Lựa chọn 0):**
   - Dừng luồng mô phỏng ngầm nếu đang chạy (`simEngine.stop()`).
   - Tự động lưu toàn bộ dữ liệu cảm biến và nhật ký cảnh báo ra file đĩa.
   - Kết thúc chương trình sạch sẽ với exit code 0.

---

## 4. LUỒNG THÊM MỚI MỘT CẢM BIẾN

```
User Input (ID, Name, Temp, Warn, Crit)
                 │
                 ▼
     ConsoleUI::handleAddSensor
                 │
  [Bước 1] Kiểm tra ID > 0?
  [Bước 2] Kiểm tra sensorMgr.hasSensor(ID)? (Chặn trùng lặp)
  [Bước 3] Kiểm tra Warn < Crit?
                 │
                 ▼ (Hợp lệ)
  [Bước 4] Khởi tạo Sensor(id, name, temp, warn, crit)
                 │ (Constructor kiểm tra lại invariants)
                 ▼
  [Bước 5] sensorMgr.addSensor(sensor)
                 │
        Khóa std::lock_guard
        m_sensors.push_back(sensor)
        Nhả khóa tự động khi ra khỏi hàm
                 │
                 ▼
       In thông báo thành công
```

---

## 5. LUỒNG LƯU VÀ NẠP DỮ LIỆU AN TOÀN

### Tại sao gọi là "Two-phase Commit" trong File I/O?
Nếu người dùng yêu cầu nạp file nhưng file bị xóa mất, hoặc đường dẫn bị sai:
- **Cách viết ngây thơ:** Gọi `m_sensors.clear()`, sau đó thử mở file. Nếu mở file thất bại $\to$ Toàn bộ dữ liệu trong RAM bị mất vĩnh viễn!
- **Giải pháp của dự án:**
  1. Kiểm tra `std::filesystem::exists(path)`.
  2. Mở file và đọc vào một biến tạm cục bộ: `std::vector<Sensor> loadedSensors`.
  3. Duyệt từng dòng, parse và kiểm tra tính hợp lệ. Dòng lỗi được đếm vào `skippedCount` và bỏ qua.
  4. **Chỉ khi file đã được đọc xong:** Mới gọi `manager.clear()` và đưa các cảm biến đã parse vào `SensorManager`.
  5. Nếu file bị lỗi ở bất kỳ bước nào trước đó: Hàm `return {false, ...}`, dữ liệu hiện tại trong RAM được bảo toàn 100%.

---

## 6. LUỒNG PHÁT SINH VÀ GHI NHẬN CẢNH BÁO

Cảnh báo được kích hoạt khi trạng thái của cảm biến bị thay đổi, bất kể là do **cập nhật thủ công (Menu 3)** hay do **luồng mô phỏng (SimulationEngine)** thực hiện:

```
Nhiệt độ thay đổi (oldTemp -> newTemp)
                 │
                 ▼
   Lấy prevStatus = sensor.getStatus()
   Gán nhiệt độ mới: sensor.setTemperature(newTemp)
   Lấy newStatus = sensor.getStatus()
                 │
  [Kiểm tra] prevStatus != newStatus ?
        ├── KHÔNG ──> Kết thúc (Không tạo cảnh báo rác)
        │
        └── CÓ ─────> Tạo AlertRecord:
                      - sensorId, sensorName
                      - newTemp
                      - prevStatus, newStatus
                      - timestamp (YYYY-MM-DD HH:MM:SS)
                      │
                      ▼
             alertMgr.addAlert(record)
                      │
             Lưu vào memory (m_alerts)
             VÀ ghi append vào data/alerts.txt
```

---

## 7. LUỒNG HOẠT ĐỘNG VÀ DỪNG LUỒNG AN TOÀN

Đây là phần kỹ thuật quan trọng nhất thể hiện kỹ năng Senior trong CV:

### 1. Cơ chế Snapshot Copy (Tránh giữ Lock lâu)
- **Vấn đề:** Nếu luồng worker giữ khóa mutex trong suốt thời gian lặp qua từng cảm biến và tính toán, giao diện console sẽ bị giật lag (freeze) vì không lấy được khóa để hiển thị.
- **Giải pháp:** Luồng worker chỉ gọi `m_sensorManager.getAllSensors()`. Hàm này khóa mutex, sao chép nhanh vector cảm biến ra một bản sao cục bộ (snapshot), rồi **nhả khóa ngay lập tức**. Việc tính toán nhiệt độ ngẫu nhiên diễn ra hoàn toàn ngoài vùng khóa.

### 2. Cơ chế Time Slicing khi Sleep (Tránh đơ khi Stop)
- Giả sử chu kỳ mô phỏng là $1500\text{ ms}$. Nếu ta gọi `std::this_thread::sleep_for(1500ms)`, khi người dùng bấm dừng (Stop), chương trình sẽ phải chờ tới 1.5 giây mới phản hồi.
- **Giải pháp:** Chia $1500\text{ ms}$ thành 30 lát nhỏ, mỗi lát $50\text{ ms}$:
  ```cpp
  int elapsed = 0;
  while (m_running.load() && elapsed < intervalMs) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      elapsed += 50;
  }
  ```
  Nhờ đó, khi `stop()` đặt `m_running = false`, luồng chỉ mất tối đa $50\text{ ms}$ để nhận diện cờ dừng và kết thúc ngay lập tức.

### 3. Vòng đời luồng (RAII)
- `SimulationEngine` sở hữu `std::thread m_workerThread`.
- Trong hàm hủy `~SimulationEngine()`, luôn gọi `stop()` để `join()`. Luồng không bao giờ bị rơi vào trạng thái `detached`, ngăn chặn hoàn toàn lỗi sập chương trình nghiêm trọng `std::terminate()`.

---

## 8. CÁC KỸ THUẬT C++17 QUAN TRỌNG ĐÃ SỬ DỤNG

| Tính năng C++17 | Ứng dụng cụ thể trong dự án | Lợi ích đạt được |
|---|---|---|
| `std::optional<T>` | `SensorManager::getSensorById(id)` | Thể hiện rõ ràng hàm có thể không tìm thấy phần tử mà không cần dùng con trỏ `nullptr` dễ gây lỗi `NullPointerException`. |
| `std::filesystem` | `FileManager`, kiểm tra `exists`, tạo thư mục `create_directories` | Quản lý đường dẫn file độc lập với hệ điều hành (chạy tốt trên cả Windows và Linux). |
| `std::atomic<bool>` | Cờ hiệu `m_running` trong `SimulationEngine` | Thao tác đọc/ghi biến cờ an toàn giữa các luồng mà không cần tốn chi phí khóa mutex nặng nề. |
| `std::lock_guard<std::mutex>` | Bảo vệ `m_sensors` và `m_alerts` | Cơ chế RAII tự động nhả khóa khi kết thúc hàm hoặc khi có ngoại lệ phát sinh, chống hoàn toàn deadlock. |
| `enum class` | `SensorStatus`, `SortOrder` | Enum có phạm vi (scoped enum), kiểu dữ liệu mạnh (strongly typed), không bị xung đột tên toàn cục. |
| `noexcept` | Getters của `Sensor`, kiểm tra ngưỡng tĩnh | Tối ưu hóa hiệu năng biên dịch và báo hiệu cho compiler biết hàm đảm bảo không ném ngoại lệ. |
| `const` correctness | Tất cả các phương thức truy vấn (`const noexcept`) | Đảm bảo tính bất biến của đối tượng khi đọc dữ liệu, giúp code an toàn và dễ bảo trì. |

---

## 9. BỘ CÂU HỎI PHỎNG VẤN THỰC TẾ VÀ CÂU TRẢ LỜI MẪU

### Câu hỏi 1: Tại sao trong `SensorManager::getSensorById` bạn lại trả về `std::optional<Sensor>` thay vì con trỏ `Sensor*` hay tham chiếu `Sensor&`?
> **Trả lời:**  
> "Trong môi trường đa luồng, container `std::vector<Sensor>` có thể được luồng khác thay đổi (thêm phần tử mới gây reallocation hoặc xóa phần tử). Nếu trả về con trỏ `Sensor*` hoặc tham chiếu `Sensor&`, sau khi nhả mutex, vùng nhớ đó có thể đã bị di dời hoặc giải phóng, dẫn đến lỗi **Dangling Pointer** (con trỏ treo) và hành vi bất định (**Undefined Behavior**).  
> Trả về `std::optional<Sensor>` theo dạng giá trị (by value) tạo ra một bản sao snapshot độc lập và an toàn, đồng thời diễn đạt ngữ nghĩa rõ ràng: hàm có thể tìm thấy hoặc không tìm thấy cảm biến mà không cần dùng con trỏ `nullptr`."

### Câu hỏi 2: Bạn xử lý Data Race và Deadlock như thế nào trong `SimulationEngine`?
> **Trả lời:**  
> "Em áp dụng 3 nguyên tắc đồng bộ nghiêm ngặt:  
> 1. Dùng `std::lock_guard<std::mutex>` theo cơ chế RAII để khóa vùng dữ liệu dùng chung (`m_sensors`).  
> 2. **Tuyệt đối không giữ khóa mutex trong khi luồng đang sleep hoặc ghi file I/O**. Luồng worker chỉ khóa mutex trong thời gian micro-giây để lấy bản sao dữ liệu (`getAllSensors()`) rồi nhả khóa ngay lập tức.  
> 3. Dùng cờ hiệu nguyên tử `std::atomic<bool> m_running` để phát tín hiệu dừng giữa các luồng mà không cần dùng mutex riêng cho cờ này."

### Câu hỏi 3: Nếu luồng chính gọi hàm hủy `~SimulationEngine()` trong khi worker thread vẫn đang chạy thì điều gì sẽ xảy ra?
> **Trả lời:**  
> "Nếu một `std::thread` vẫn còn ở trạng thái `joinable()` khi destructor của nó được gọi, chuẩn C++ quy định chương trình sẽ gọi ngay `std::terminate()` và crash ứng dụng.  
> Để khắc phục, em đã triển khai destructor của `SimulationEngine` luôn tự động gọi hàm `stop()`. Hàm `stop()` sẽ đặt cờ `m_running = false`, kiểm tra `if (m_workerThread.joinable())`, và gọi `m_workerThread.join()` để chờ luồng worker kết thúc sạch sẽ rồi mới hủy đối tượng."

### Câu hỏi 4: Dự án của bạn xử lý trường hợp nạp file bị lỗi như thế nào?
> **Trả lời:**  
> "Em triển khai theo mô hình **Two-phase Commit**:  
> Đầu tiên, đọc và parse toàn bộ file vào một vector tạm `std::vector<Sensor> loadedSensors`. Trong quá trình này, các dòng sai cú pháp, thiếu trường hay sai kiểu số đều được phát hiện qua `try/catch` với `std::stod`/`std::stoi` và ghi nhận vào số lượng dòng bỏ qua mà không làm crash app.  
> Chỉ khi toàn bộ file được đọc thành công, em mới xóa dữ liệu cũ trong `SensorManager` và cập nhật dữ liệu mới. Nếu file không tồn tại hoặc lỗi quyền truy cập, hàm báo lỗi trả về `false` và giữ nguyên 100% dữ liệu đang hoạt động trong bộ nhớ."

---

## 10. GỢI Ý CÁCH TỰ GIỚI THIỆU DỰ ÁN ẤN TƯỢNG VỚI NHÀ TUYỂN DỤNG

Bạn có thể dùng đoạn mẫu sau khi được hỏi: *"Hãy giới thiệu một dự án C++ mà bạn tâm đắc nhất"*:

> *"Em xin phép chia sẻ về dự án **Industrial Sensor Monitoring System** mà em xây dựng bằng **C++17** và **CMake**.*  
> *Dự án này mô phỏng một hệ thống giám sát nhiệt độ công nghiệp thời gian thực trong các nhà máy hóa chất và kho lạnh. Điểm nhấn kỹ thuật lớn nhất của dự án nằm ở **kiến trúc đa luồng an toàn (Concurrency Safety)**:*  
> *- Em sử dụng `std::thread` chạy ngầm để mô phỏng biến thiên nhiệt độ và tự động kích hoạt cảnh báo khi vượt ngưỡng `WARNING` hoặc `CRITICAL`.*  
> *- Dữ liệu chia sẻ được đồng bộ bằng `std::mutex` và `std::lock_guard`, kiểm soát dừng bằng `std::atomic<bool>` với kỹ thuật chia lát thời gian (time slicing) để phản hồi dừng tức thì.*  
> *- Toàn bộ hệ thống quản lý danh mục cảm biến, bộ lọc trạng thái, thuật toán tìm kiếm gần đúng, tính toán Dashboard thống kê động, và cơ chế lưu trữ file chịu lỗi (Fault-tolerant I/O).*  
> *Dự án được tích hợp đầy đủ 5 bộ kiểm thử tự động với **CMake CTest**, tuân thủ nguyên lý thiết kế **RAII**, **const correctness** và không để xảy ra memory leak hay data race."*
