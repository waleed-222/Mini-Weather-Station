#include <iostream>
#include <mqtt/async_client.h>
#include <gpiod.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <ctime>
#include <sstream>


#define GPIO_CHIP "/dev/gpiochip4" 
#define GPIO_LINE 20

#define TEMP_PATH "/sys/bus/iio/devices/iio:device0/in_temp_input"
#define HUM_PATH  "/sys/bus/iio/devices/iio:device0/in_humidityrelative_input"
#define CSV_FILE  "dht_data.csv"

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("mqtt_client");
const std::string TOPIC = "temp/humidity";
const int QOS = 1;

float read_sensor_value(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Failed to open " << path << std::endl;
        return -1;
    }

    float value;
    file >> value;
    return value;
}

void blink_led(gpiod_line* line) {
    gpiod_line_set_value(line, 1);
    usleep(200000); // 200 ms
    gpiod_line_set_value(line, 0);
}

class ClientCallback : public virtual mqtt::callback {
public:
    void connection_lost(const std::string &cause) override {
        std::cout << "Connection lost: " << cause << std::endl;
    }

    void delivery_complete(mqtt::delivery_token_ptr token) override {}
    void message_arrived(mqtt::const_message_ptr msg) override {}
};

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    ClientCallback callback;
    client.set_callback(callback);

    mqtt::connect_options conn_opts;
    conn_opts.set_keep_alive_interval(20);
    conn_opts.set_clean_session(true);

    try {
        client.connect(conn_opts)->wait();
    } catch (const mqtt::exception& exc) {
        std::cerr << "MQTT connection error: " << exc.what() << std::endl;
        return 1;
    }

    // Initialize GPIO
    gpiod_chip *chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Error: Unable to open GPIO chip\n";
        return 1;
    }

    gpiod_line *line = gpiod_chip_get_line(chip, GPIO_LINE);
    if (!line || gpiod_line_request_output(line, "mqtt_blink", 0) < 0) {
        std::cerr << "Error: Unable to configure GPIO line\n";
        gpiod_chip_close(chip);
        return 1;
    }

    // Create CSV header if needed
    std::ofstream csv_file(CSV_FILE, std::ios::app);
    if (csv_file.tellp() == 0) {
        csv_file << "Timestamp,Temperature(C),Humidity(%)\n";
    }
    csv_file.close();

    while (true) {
        // Wait until seconds == 0
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_c);
        int seconds = now_tm->tm_sec;

        int sleep_duration = 60 - seconds;
        std::this_thread::sleep_for(std::chrono::seconds(sleep_duration));

        // Get current timestamp after sleeping
        now = std::chrono::system_clock::now();
        now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* timestamp_tm = std::localtime(&now_c);

        // Read sensor values
        float temp_raw = read_sensor_value(TEMP_PATH);
        float hum_raw = read_sensor_value(HUM_PATH);

        float temp_c = temp_raw / 1000.0f;
        float hum = hum_raw / 1000.0f;

        // Format timestamp
        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timestamp_tm);

        // Log to CSV
        std::ofstream csv(CSV_FILE, std::ios::app);
        csv << time_str << "," << temp_c << "," << hum << "\n";
        csv.close();

        // Publish to MQTT
        std::ostringstream payload;
        payload << "{ \"timestamp\": \"" << time_str << "\", \"temperature\": " << temp_c << ", \"humidity\": " << hum << " }";

        try {
            client.publish(TOPIC, payload.str(), QOS, false)->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "MQTT publish error: " << exc.what() << std::endl;
        }

        // Blink LED
        blink_led(line);

        std::cout << "Logged & Published: " << payload.str() << std::endl;
    }

    // Never reached
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    client.disconnect()->wait();

    return 0;
}
