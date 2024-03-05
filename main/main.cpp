extern "C" {
#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <esp_task.h>
#include <driver/gpio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_random.h"
}

#include <cstdint>
#include <string>
#include <memory>
#include <iostream>
#include "../components/httpd/httpd.hpp"
#include "../components/wifi/wificonnection.hpp"
#include "../components/filesystem/ufs.hpp"

static constexpr char TAG[] {"wifi MAIN"};


extern "C" void app_main(void)
{
    //Initialize NVS (Non-volatile storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    gpio_reset_pin((gpio_num_t) 2);
    gpio_set_direction((gpio_num_t)2, GPIO_MODE_OUTPUT);
    gpio_reset_pin((gpio_num_t)4);
    gpio_set_direction((gpio_num_t)4, GPIO_MODE_INPUT);

    auto wifiEvGroup {xEventGroupCreate()};
    if (wifiEvGroup != NULL)
    {
        std::string ssid{"ssid"};
        std::string password{"password"};
        auto wifi {U::WifiConnection(ssid, password, wifiEvGroup)};
        wifi.Connect();

        U::start_rest_server("/www");
        
        while (1)
        {
            vTaskDelay(1);
            gpio_set_level((gpio_num_t)2, gpio_get_level((gpio_num_t)4));
        }
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create event group for WiFi");
    }
}
