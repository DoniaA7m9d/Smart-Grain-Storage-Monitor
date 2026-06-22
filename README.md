# 🌾 Smart Grain Storage Monitor

<img src="architecture.png" alt="System Architecture Diagram" width="100%">

### An ESP32-based multi-sensor I2C IoT system that monitors grain storage environmental conditions using SHT31 sensors, provides real-time Telegram alerts for mold risk, and logs data to Google Sheets for time-series analysis.

---

## 🛑 The Problem

Strategic grain stocks like wheat and corn are prone to rapid spoilage and mold, leading to massive economic losses. The main causes are:
1. **High ambient humidity** inside the storage facility beyond safe limits.
2. **Sudden temperature changes** that trigger mold and insect growth.

## 💡 The Solution

This project provides a complete IoT monitoring system using simple AI logic and ultra-low power management:

### Key Features:
*   **📡 Multi-Sensor Network:** Uses 2-3 **SHT31** I2C sensors for high-accuracy ±2% RH readings across the entire storage. Better precision and faster response than DHT sensors.
*   **🚨 Real-time Alerts:** The system doesn't wait for periodic reports. If humidity exceeds the danger threshold (e.g., 70%) at any time, it sends an urgent Telegram alert for immediate action.
*   **📊 Data Logging for Analysis:** All readings for each sensor and the average are automatically sent to **Google Sheets**. Clean time-series data ready for **Data Science** and predictive analysis.
*   **🔋 Ultra-Low Power Management:** The system enters deep sleep and disconnects between readings, allowing a power bank to run the device for months without maintenance.

---

## 🏗️ System Architecture

The design relies on a smooth I2C data flow from physical sensors to the cloud. You can see the diagram below:

<img src="architecture.png" alt="System Flow Architecture" width="800">

*(Diagram shows: SHT31 Sensors via I2C -> ESP32 -> Deep Sleep <-> Risk Check -> Wi-Fi Connection -> Send to Telegram and Google Sheets)*

---

## 🛠️ Hardware Requirements

*   **ESP32 Development Board** (e.g., DOIT ESP32 DEVKIT V1)
*   **2-3x SHT31 Humidity & Temperature Sensors** (I2C interface, address 0x44 or 0x45)
*   **Large Power Bank** (10,000mAh+ recommended)
*   For full Deep Sleep: Connect a wire between **GPIO 16 (D0)** and **RST** on the ESP32.
*   4.7kΩ pull-up resistors if your SHT31 module doesn't have them onboard.
*   Jumper wires and a grain storage for testing! 🌾

---

## 💻 Installation & Setup

### 1. Arduino IDE Setup for ESP32:
*   Install ESP32 board definitions in the IDE.
*   Install the following libraries from Library Manager:
    *   `Adafruit SHT31 Library` - for SHT31 I2C communication
    *   `Adafruit BusIO` - dependency for SHT31 library
    *   `UniversalTelegramBot`
    *   `ArduinoJson`
*   Open the attached `grain_monitor.ino` file.

### 2. Cloud Services Setup:

#### A. Telegram Bot:
1.  Create a new bot using **BotFather** to get the `BOT_TOKEN`.
2.  Get your `CHAT_ID` or the storage group's `CHAT_ID`.
3.  Add these credentials to the code.

#### B. Google Sheets Data Logging:
1.  Create a new Google Sheet.
2.  Open **Apps Script** and paste the code from `google_script.js`.
3.  Deploy the code as a **Web App** with access set to "Anyone".
4.  Get the `Deployment ID` and add it to the Arduino code.

---

## 📊 Usage & Output Examples

### Example Telegram Alert Message:
When a risk is detected, a Markdown-formatted message is sent:

> **🚨 URGENT RISK ALERT - Grain Storage** 🚨
>
> ⚠️ **Sensor 2: Critical Humidity 78%**
>
> 📊 **Overall Average:**
> 🔹 Storage Humidity: 72.4%
> 🔹 Storage Temperature: 31.0°C
>
> 🛑 **Recommendation:** Please activate ventilation fans immediately to prevent crop spoilage.

### Example Google Sheets Log:
Data is logged automatically in this format, ready to download as CSV and analyze with Python:

| Timestamp | Temp 1 | Hum 1 | Temp 2 | Hum 2 | Temp 3 | Hum 3 | Avg Temp | Avg Humidity | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| 2026-06-05 08:00:00 | 29.5 | 65.2 | 29.8 | **78.1** | 30.1 | 62.0 | 29.8 | 68.4 | Emergency |
| 2026-06-05 14:00:00 | 28.1 | 60.1 | 28.5 | 62.3 | 28.8 | 61.5 | 28.5 | 61.3 | OK |

---

## 🤝 Contributing

The project is open for contributions! Ideas to improve analysis accuracy or integrate new sensors like DS18B20 for grain core temperature are very welcome. Fork the repo and start developing.
