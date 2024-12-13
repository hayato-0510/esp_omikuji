#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

constexpr uint8_t BTN_PIN = 4;
unsigned long buttonPressStart = 0; // ボタンが押され始めた時間
bool dataSent = false;              // データ送信が完了したかどうか

// マスターのMACアドレス（送信先）
uint8_t masterAddress[] = {0xA0, 0xDD, 0x6C, 0x69, 0xC2, 0xE4};

static const char *client_ssid = "FairyGuide_Connect";
static const char *client_password = "password";

// ESP-NOW用のメッセージ構造体
typedef struct struct_message {
    char message[250];
} struct_message;

struct_message dataToSend;

void WiFi_setup();
void esp_now_setup();
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status);

void setup() {
    pinMode(BTN_PIN, INPUT);
    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("Hold the button for 3 seconds to send 'OMIKUJI'..."));

    WiFi_setup();
    esp_now_setup();
}

void loop() {
    // ボタンが押されているかのチェック
    if (digitalRead(BTN_PIN) == HIGH) {
        if (buttonPressStart == 0) {
            buttonPressStart = millis(); // 押された時間を記録
        } else if (millis() - buttonPressStart >= 3000) {
            // 3秒以上押されていた場合
            if (!dataSent) {
                snprintf(dataToSend.message, sizeof(dataToSend.message), "OMIKUJI");
                esp_err_t result = esp_now_send(masterAddress, (uint8_t *)&dataToSend, sizeof(dataToSend));
                if (result == ESP_OK) {
                    Serial.println("Data sent: OMIKUJI");
                    dataSent = true; // 一度だけ送信する
                } else {
                    Serial.println("Data send failed");
                }
            }
        }
    } else {
        // ボタンが離された場合
        buttonPressStart = 0; // リセット
        dataSent = false;     // 次回送信を可能にする
    }
}

void WiFi_setup()
{
  WiFi.mode(WIFI_STA); // Wi-FiをStationモードに設定
  WiFi.begin(client_ssid, client_password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connected Wifi");
  }

  Serial.println("MACアドレス: " + WiFi.macAddress());
}

void esp_now_setup() {
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOWの初期化に失敗しました");
        return;
    }
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, masterAddress, 6);
    peerInfo.channel = WiFi.channel();
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("マスターの登録に失敗しました");
    return;
  }
    esp_now_add_peer(&peerInfo);
    esp_now_register_send_cb(onSent);
}

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("ESP-NOW送信ステータス: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "成功" : "失敗");
}
