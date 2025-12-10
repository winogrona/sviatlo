#include <ble.hpp>
#include <nvm.hpp>
#include <strip.hpp>
#include <settings.hpp>

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <vector>
#include <string>

namespace ble {
    using std::vector;
    using std::string;

    class DisconnectCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer *pServer) {
            printf("[ble] device connected\n");
        }

        void onDisconnect(BLEServer *pServer) {
            printf("[ble] Device disconnected\n");
            printf("[ble] Starting to advertise again\n");

            pServer->startAdvertising();
        }
    };

    class ColorCallbacks : public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *characteristic) {
            std::string value = characteristic->getValue();
            printf("[ble] received a new color string: \"%s\"\n", value.c_str());

            vector<color::Color> colors;
            size_t pos = 0;
            while (pos < value.size()) {
                size_t sep = value.find(';', pos);
                if (sep == string::npos) break;
                string token = value.substr(pos, sep - pos);
                if (!token.empty()) {
                    if (token.size() == 6) {
                        try {
                            uint8_t r = static_cast<uint8_t>(std::stoi(token.substr(0,2), nullptr, 16));
                            uint8_t g = static_cast<uint8_t>(std::stoi(token.substr(2,2), nullptr, 16));
                            uint8_t b = static_cast<uint8_t>(std::stoi(token.substr(4,2), nullptr, 16));
                            colors.emplace_back(r, g, b);
                        } catch (const std::invalid_argument& e) {
                            printf("[ble] failed to parse color token (invalid argument): '%s', error: %s\n", token.c_str(), e.what());
                        } catch (const std::out_of_range& e) {
                            printf("[ble] failed to parse color token (out of range): '%s', error: %s\n", token.c_str(), e.what());
                        }
                    } else {
                        printf("[ble] invalid token (expected 6 hex chars): %s'\n", token.c_str());
                        printf("[ble] discarding the message");
                        return;
                    }
                }
                pos = sep + 1;
            }

            nvm::save_last_strip_state(colors);
            strip::leds = colors;
            strip::show();
        }
    };

    /// @brief Sets up the strip; Requires nvm
    void setup(vector<color::Color> strip_state) {
        BLEDevice::init(settings::BLE_DEVICE_NAME); // BLE device name
        BLEServer *server = BLEDevice::createServer();
        server->setCallbacks(new DisconnectCallbacks());

        BLEService *service = server->createService(SERVICE_UUID);

        BLECharacteristic *colorCharacteristic = service->createCharacteristic(
            COLOR_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR
        );

        BLECharacteristic *lengthCharacteristic = service->createCharacteristic(
            LENGTH_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ
        );

        colorCharacteristic->setCallbacks(new ColorCallbacks());

        string color_string = "";

        for (auto color : strip_state) {
            color_string.append(color.to_hex() + ";");
        }

        colorCharacteristic->setValue(color_string);
        lengthCharacteristic->setValue((string) std::to_string(strip::len));

        service->start();

        BLEAdvertising *pAdvertising = server->getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(true);
        pAdvertising->start();
    }
}