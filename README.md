# 🌦️ Mini Weather Station with DHT11, LED, and Raspberry Pi 5

This is a mini weather station project that uses a **DHT11 sensor** for temperature and humidity readings, an **LED** for visual feedback, and a **Raspberry Pi 5**. The system logs data to a CSV file, publishes readings via **MQTT**, and includes a **Python GUI** to visualize the collected data.

---

## 🚀 Features

- 🧠 **Reads temperature and humidity** via DHT11
- 💡 **Blinks LED** with each reading
- 💾 **Logs data** in CSV format
- 📡 **Publishes MQTT messages** with sensor readings
- 🖥️ **Python GUI dashboard** using `ttkbootstrap` and `tkinter`
- 🌍 **Displays location** via IP geolocation

---

## 🧰 Hardware & Tools

- Raspberry Pi 5 (running 64-bit Linux)
- DHT11 sensor (connected via IIO interface)
- LED (connected to GPIO line 20 on `/dev/gpiochip4`)
- MQTT broker (e.g., Mosquitto running on `localhost:1883`)

---

## 🛠️ Build and Run (C++)

### 🔧 Cross Compilation

This project uses a cross-compiler for `aarch64`:

```bash
aarch64-linux-gnu-g++ main.cpp -o dht11_rpi \
  -I ./gpiod_cross/include \
  ./gpiod_cross/lib/libgpiod.a \
  -static
```
## 📁 Input Paths
Temperature: /sys/bus/iio/devices/iio:device0/in_temp_input

Humidity: /sys/bus/iio/devices/iio:device0/in_humidityrelative_input

Output file: dht_data.csv

## ▶️ Run on Raspberry Pi
Transfer the compiled binary to the Raspberry Pi 5 and run:

```bash

./dht11_rpi
```
This will:

Read sensor data every minute

Log to dht_data.csv

Publish to mqtt://localhost:1883 on topic temp/humidity

Blink LED on GPIO 20

## 🖼️ GUI Monitor (Python)
📦 Requirements
Install the following Python packages:

```bash

pip install ttkbootstrap requests
```

## ▶️ Run the Dashboard

```bash

python3 gui_app.py
```
Where gui_app.py is your Python script (shown above).

The app displays:

Timestamped sensor values

Auto-detected city and country via IP

Live updates every minute (aligned to 0 seconds)

## 🧪 Example MQTT Payload
```json
{
  "timestamp": "2025-07-22 12:00:00",
  "temperature": 26.4,
  "humidity": 45.7
}
```
## 🗂️ Project Structure
```bash
project/
│
├── main.cpp            # C++ code for reading sensors and publishing via MQTT
├── gui_app.py          # Python GUI for displaying logged data
├── dht_data.csv        # Generated CSV file with timestamped readings
├── gpiod_cross/        # Cross-compiled libgpiod headers and library
└── README.md
```

## 🎥 Project Demo

Watch the demo video here:  
👉 [Mini Weather Station Project Video](https://drive.google.com/file/d/17E39RMn4XkAehJsM1z2kUhPalUeIcsUE/view?usp=drive_link)


## 📝 Notes

MQTT is used for future scalability (remote monitoring).

Make sure your Raspberry Pi has internet access for IP geolocation.

CSV format allows easy export and plotting for long-term analysis.

You Can use Mobile Application to access to data.

## 📜 License

MIT License

