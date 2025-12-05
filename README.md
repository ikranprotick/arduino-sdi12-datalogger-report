# Arduino SDI-12 Environmental Data Logger 🌱

This project is an **Arduino-based SDI-12 sensor and data logger** that reads data from a
**BME680 environmental sensor** (temperature, humidity, pressure, gas) and a **BH1750 light
sensor**, logs measurements to an **SD card** with timestamps from an **RTC**, and provides a
**TFT LCD menu + graphing interface** controlled by push buttons.

It also implements a subset of the **SDI-12 protocol**, allowing an external SDI-12 master
to query the device for measurements and identification.

Main sketch file:  
`src/PROJECT_HD_2_MENUCHANGE3.ino`

---

## ✨ Features

- 📡 **SDI-12 protocol support**
  - Address query: `?`
  - Change address: `aA<new>` (e.g. `0A1`)
  - Start measurement: `aM!`
  - Send data: `aD0!`, `aD1!`, `aD2!`
  - Identification: `aI!`
  - Continuous measurement: `aR!`
  - Proper SDI-12 UART config (`1200 baud, 7E1`) and DIRO control pin for TX/RX direction

- 🌍 **Multi-sensor integration**
  - **BME680**:
    - Temperature
    - Pressure
    - Humidity
    - Gas resistance
  - **BH1750**:
    - Light intensity (lux)

- 💾 **Data logging to SD card**
  - Uses **SdFat** with software SPI
  - Logs periodically via **Timer3 interrupt**
  - Adjustable logging interval from the TFT menu
  - Stores environmental data with timestamp (RTC)

- 🕒 **Real-time clock (RTC)**
  - **DS1307** used via RTClib
  - Time is set at upload (`rtc.adjust(DateTime(__DATE__, __TIME__))`)
  - Time is used for data timestamps

- 🖥️ **TFT LCD UI + Graphs**
  - ST7735 TFT display (Adafruit_GFX + Adafruit_ST7735)
  - Multiple UI screens:
    - Idle screen
    - Main menu
    - Sensor selection menu
    - SD card/logging control menu
    - Graph views for:
      - Temperature
      - Pressure
      - Humidity
      - Gas resistance
      - Light (lux)
  - Graphs use simple bar-style/line-style visuals with dynamically updated values

- 🎛️ **Button-based navigation**
  - 4 push buttons for:
    - Up
    - Down
    - Select
    - Back
  - Debouncing implemented using timestamp checks
  - Context-aware behavior based on current menu (main, sensor, SD card, graph)

---

## 🧱 Hardware Overview

You can adapt this to your exact hardware, but the sketch assumes something like:

- **Microcontroller**: Arduino board with multiple serial ports (e.g. Arduino Due / Mega)
- **Sensors**:
  - BME680 (I²C)
  - BH1750 (I²C, address `0x23`)
- **SD Card**:
  - SD card module using **SdFat** over software SPI  
    - `SD_CS_PIN = A3`  
    - Software SPI pins (MISO 12, MOSI 11, SCK 13)
- **RTC**:
  - DS1307 (I²C)
- **Display**:
  - ST7735 TFT
  - Pins:
    - `TFT_CS = 10`
    - `TFT_DC = 9`
    - `TFT_RST = 8`
    - `TFT_MOSI = 11`
    - `TFT_SCLK = 13`
- **Buttons**:
  - `pushButtons[4] = { 5, 4, 3, 2 }`
- **SDI-12 Direction Control Pin**:
  - `DIRO = 7` (HIGH = receive, LOW = transmit)
- **Serial Ports**:
  - `Serial` for debug
  - `Serial1` configured as SDI-12 interface (`1200, SERIAL_7E1`)

> ⚠️ Adjust pins in the code if your wiring is different.

---

## 🔌 SDI-12 Command Handling (High Level)

The function `SDI12_Command(String input)` parses SDI-12 commands received via the
serial debug monitor / SDI-12 line.

Supported patterns (assuming device address `a`):

- `?`  
  → **Address Query** – returns current device address

- `aA<d>`  
  → **Change Address**, where `<d>` is a digit (e.g. `0A1`)  
  Updates `deviceAddress` and returns the new address.

- `aM!`  
  → **Start Measurement**  
  Triggers sensors to take measurements, sets a flag `dataReady = true`.

- `aD0!`, `aD1!`, `aD2!`  
  → **Send Data**  
  Returns measurement strings, e.g.:  
  `a+temperature+pressure+humidity+gas+lux` (split across `D0`, `D1`, `D2` as needed)  
  Once all required data has been sent, flags are reset and `dataReady` is cleared.

- `aI!`  
  → **Send Identification**  
  Responds with identification string like:  
  `a14ENG20009123456789`

- `aR!`  
  → **Continuous Measurement**  
  Attaches a Timer3 interrupt (`Continuous`) to stream data periodically until stopped.

All invalid syntax cases respond with clear error messages, e.g.  
`"ERROR: Invalid Syntax for Start Measurement command"`.

---

## 🖥️ Menu System & Graphs

The code manages UI state using:

- `currentMenu`:
  - `0` – Idle menu
  - `1` – Main menu
  - `2` – Sensor menu
  - `3` – SD card menu
  - `4` – Graph display
- `selection[2]`:
  - `selection[0]` – main menu selection
  - `selection[1]` – sensor selection

Core UI functions include:

- `idle_Menu()` – shows project title and prompt to press PB2 to enter
- `main_Menu()` – options:
  - `1. Sensors`
  - `2. SD Card`
  - `3. Return`
- `sensor_Menu()` – options:
  - `1. Temperature`
  - `2. Pressure`
  - `3. Humidity`
  - `4. Gas Quality`
  - `5. Light Intensity`
- `sdCard_Menu()` – shows SD logging status, interval, and instructions
- `temperature_Graph()`, `pressure_Graph()`, `humidity_Graph()`,
  `gas_Graph()`, `light_Graph()` – draw simple dynamic bar plots with labels

Navigation is handled by:

- `up_scroll()`
- `down_scroll()`
- `select_Button()`
- `back_Button()`

Each function debounces button press and changes `currentMenu` / `selection` accordingly.

---

## 💾 Data Logging

- Uses **Timer3** to periodically call `store_Data()`
- Interval (`sdInterrupt` in seconds) is configurable from the SD card menu
- When logging:
  - File `sensorData.txt` is opened in append mode
  - Sensor data + timestamp is written regularly
- Start/Stop logging from SD menu via PB2


