
extern "C"
{
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
}

#include <string>
#include <iostream>
#include <cstdint>
#include <wificonnection.hpp>
#include <memory>

namespace U
{
    enum class WifiConnectionEvent
    {
        Disconnected,
        Connected
    };

    bool WifiConnection::isWifiDriverInitialised {false};
    const char WifiConnection::TAG[] {"WiFi"};

    WifiConnection::WifiConnection(const std::string& ssid, 
                        const std::string& pass, EventGroupHandle_t evGroup) 
    {
        if (!isWifiDriverInitialised)
        {
            wifiEventGroup = evGroup;

            ESP_ERROR_CHECK(esp_netif_init());
            ESP_ERROR_CHECK(esp_event_loop_create_default());
            esp_netif_create_default_wifi_sta();
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));

            ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            EventHandler,
                                                            (void *) this,
                                                            &idEvent));
            ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            EventHandler,
                                                            (void *) this,
                                                            &ipEvent));

            wifiConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            wifiConfig.sta.sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK;
            // wifiConfig.sta.sae_h2e_identifier is not required
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

            reconnectTimer = xTimerCreate("WiFiTimer", reconnectInterval, pdFALSE, (void *) this, TimerCallback);
            assert(reconnectTimer != NULL);

            ESP_ERROR_CHECK(esp_wifi_start());

            isWifiDriverInitialised = true;
        }
    }

    WifiConnectionState WifiConnection::IsConnected()
    {
        return isWifiConnected;
    }

    int WifiConnection::Connect()
    {
        isWifiConnected = WifiConnectionState::Disconnected;

        esp_wifi_connect();

        EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
            (int) WifiConnectionEvent::Connected | 
            (int) WifiConnectionEvent::Disconnected,
            pdFALSE,
            pdFALSE,
            timeToConnect);

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
        if (bits & (int) WifiConnectionEvent::Connected) 
        {
            ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                    wifiConfig.sta.ssid, wifiConfig.sta.password);
            isWifiConnected = WifiConnectionState::Connected;
            return 0;
        } 
        else
        {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                    wifiConfig.sta.ssid, wifiConfig.sta.password);
            return -1;
        } 
        
        return -1;
    }

    int WifiConnection::Disconnect()
    {
        isWifiConnected = WifiConnectionState::Disconnected;
        return esp_wifi_disconnect();
    }

    int WifiConnection::SetConfiguration(const std::string& ssid, 
                                const std::string& password)
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        std::copy(std::begin(ssid), std::end(ssid), std::begin(wifiConfig.sta.ssid));
        std::copy(std::begin(password), std::end(password), std::begin(wifiConfig.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifiConfig) );

        return 0;
    }

    WifiConnection::~WifiConnection()
    {
        ESP_ERROR_CHECK(esp_wifi_stop());
        vEventGroupDelete(wifiEventGroup);
        ESP_ERROR_CHECK(esp_wifi_deinit());
        ESP_ERROR_CHECK(esp_event_loop_delete_default());
        ESP_ERROR_CHECK(esp_netif_deinit());
    }

    void WifiConnection::TimerCallback(TimerHandle_t timerHandle)
    {
        WifiConnection * conn = (WifiConnection *) (pvTimerGetTimerID(timerHandle));
        conn->Connect();
    }

    void WifiConnection::EventHandler(void* arg, esp_event_base_t eventBase,
                                int32_t eventID, void* eventData)
    {
        static size_t connectionRetries{0};

        WifiConnection * conn = (WifiConnection *) arg;

        if ((eventBase == WIFI_EVENT) && (eventID == WIFI_EVENT_STA_DISCONNECTED))
        {
            // xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
            xEventGroupClearBits(conn->wifiEventGroup, 
                        (int) WifiConnectionEvent::Connected);
            
            ESP_LOGI(TAG,"connect to the AP fail");

            // Retry 5 times then wait 30 seconds to retry again
            if (connectionRetries < 2) 
            {
                ESP_LOGI(TAG, "retry to connect to the AP");
                esp_wifi_connect();
                connectionRetries++;
            }
            else
            {
                if (xTimerIsTimerActive(conn->reconnectTimer))
                {
                    
                    std::cout << "Timer is already running \n";
                    // Timer is already running
                    return;
                }

                ESP_LOGI(TAG, "Waiting to retry");
                connectionRetries = 0;
                std::cout << "interval is " << reconnectInterval << "\n";
                xTimerStart(conn->reconnectTimer, reconnectInterval);
            }
        } 
        else if (eventBase == IP_EVENT && eventID == IP_EVENT_STA_GOT_IP) 
        {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) eventData;
            ESP_LOGI(TAG, "got ip:`" IPSTR, IP2STR(&event->ip_info.ip));
            connectionRetries = 0;

            xEventGroupSetBits(conn->wifiEventGroup, 
                        (int) WifiConnectionEvent::Connected);
        }   
    }
}
