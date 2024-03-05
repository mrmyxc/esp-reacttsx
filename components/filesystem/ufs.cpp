extern "C"
{
#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_log.h"
}

#include <string>

namespace U
{
    static constexpr char UFSTAG[] {"UFS"};

    /**
     * @brief Initialise SPIFFS
     *
     * @param mountPoint
     * @returns esp_err_t
     */
    esp_err_t filesystem_init(const std::string_view mountPoint)
    {

        esp_vfs_spiffs_conf_t conf;
        conf.base_path = mountPoint.data();
        conf.partition_label = NULL;
        conf.max_files = 5;
        conf.format_if_mount_failed = false;

        esp_err_t ret = esp_vfs_spiffs_register(&conf);

        if (ret != ESP_OK)
        {
            if (ret == ESP_FAIL)
            {
                ESP_LOGE(UFSTAG, "Failed to mount or format filesystem");
            }
            else if (ret == ESP_ERR_NOT_FOUND)
            {
                ESP_LOGE(UFSTAG, "Failed to find SPIFFS partition");
            }
            else
            {
                ESP_LOGE(UFSTAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            }
            return ESP_FAIL;
        }

        size_t total = 0, used = 0;
        ret = esp_spiffs_info(NULL, &total, &used);
        if (ret != ESP_OK)
        {
            ESP_LOGE(UFSTAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        }
        else
        {
            ESP_LOGI(UFSTAG, "Partition size: total: %d, used: %d", total, used);
        }
        return ESP_OK;
    }
}
