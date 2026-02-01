#include "wifi_comm.h"

void wifi_comm::init(const char* ssid,
                     const char* password,
                     uint32_t timeout_ms,
                     uint32_t retry_interval_ms)
{
    _ssid = ssid;
    _password = password;
    _timeout_ms = timeout_ms;
    _retry_interval_ms = retry_interval_ms;

    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    _start_ms = millis();
    _state = CONNECTING;

    Serial.print("[WiFi] Connecting to ");
    Serial.println(_ssid);
}

void wifi_comm::update()
{
    // 已連線：監控是否掉線（非阻塞）
    if (_state == CONNECTED) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WiFi] Disconnected");
            _state = IDLE;
            _next_retry_ms = millis() + _retry_interval_ms;
        }
        return;
    }

    // 連線中：檢查成功 / 超時（非阻塞）
    if (_state == CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            _state = CONNECTED;
            Serial.print("[WiFi] Connected, IP: ");
            Serial.println(WiFi.localIP());
            return;
        }

        if (millis() - _start_ms >= _timeout_ms) {
            _state = TIMEOUT;
            _next_retry_ms = millis() + _retry_interval_ms;
            Serial.println("[WiFi] Connect timeout");
            return;
        }

        return; // 繼續等待，不阻塞
    }

    // IDLE / TIMEOUT：到時間就重試（非阻塞）
    if ((_state == IDLE || _state == TIMEOUT) && _ssid && _password) {
        if (millis() >= _next_retry_ms) {
            Serial.println("[WiFi] Retry connecting...");
            WiFi.disconnect(true);
            WiFi.begin(_ssid, _password);
            _start_ms = millis();
            _state = CONNECTING;
        }
    }
}

bool wifi_comm::isConnected() const
{
    return (_state == CONNECTED) && (WiFi.status() == WL_CONNECTED);
}

wifi_comm::State wifi_comm::state() const
{
    return _state;
}
