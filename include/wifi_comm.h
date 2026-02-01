#ifndef WIFI_COMM_H
#define WIFI_COMM_H

#include <Arduino.h>
#include <WiFi.h>

class wifi_comm {
public:
    enum State {
        IDLE,
        CONNECTING,
        CONNECTED,
        TIMEOUT
    };

    void init(const char* ssid = "COMMMM",
              const char* password = "8765432100",
              uint32_t timeout_ms = 10000,
              uint32_t retry_interval_ms = 5000);

    void update();

    bool isConnected() const;
    State state() const;
    //推播
    bool pushDiscord(const String &msg);

private:
    const char* _ssid = nullptr;
    const char* _password = nullptr;

    uint32_t _timeout_ms = 10000;
    uint32_t _retry_interval_ms = 5000;

    uint32_t _start_ms = 0;
    uint32_t _next_retry_ms = 0;

    State _state = IDLE;
};

#endif
