#pragma once
#ifdef __cpluscplus
extern "C" {
#endif
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#ifdef __cpluscplus
}
#endif

#include <string>

namespace U
{
    enum class WifiConnectionState
    {
        Disconnected,
        Connected
    };

    class WifiConnection
    {
    private:
        static constexpr TickType_t timeToConnect {pdMS_TO_TICKS(3000)};
        static constexpr TickType_t reconnectInterval {pdMS_TO_TICKS(15000)};
        static bool isWifiDriverInitialised;
        static const char TAG[];
        EventGroupHandle_t wifiEventGroup;
        wifi_config_t wifiConfig;
        WifiConnectionState isWifiConnected;
        esp_event_handler_instance_t idEvent;
        esp_event_handler_instance_t ipEvent;
        TimerHandle_t reconnectTimer;
        static void TimerCallback(TimerHandle_t timerHandle);
        static void EventHandler(void* arg, esp_event_base_t eventBase,
                                int32_t eventID, void* eventData);

    public:

        WifiConnection() = delete;

        WifiConnection(const std::string& ssid, 
                        const std::string& pass, 
                        EventGroupHandle_t wifiEventGroup);

        int Connect();

        int Disconnect();

        int SetConfiguration(const std::string& ssid, 
                            const std::string& password);

        WifiConnectionState IsConnected();
        
        ~WifiConnection();
    };
}