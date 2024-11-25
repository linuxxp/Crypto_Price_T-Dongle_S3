#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi_manager.h"
#include "webserver.h"
#include "crypto_price.h"

static const char *TAG = "main";
static bool ap_mode = false;

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi
    init_wifi();

    // Try to connect to configured WiFi
    if (!connect_to_wifi()) {
        // If failed, try to connect to open network
        if (!scan_and_connect_open_network()) {
            // If no open network, create AP
            create_ap();
            httpd_handle_t server = start_webserver();
            ap_mode = true;
            ESP_LOGI(TAG, "Started in AP mode");
        }
    }

    // Initialize crypto price module
    init_crypto_price();

    // If connected to WiFi, start main application loop
    if (!ap_mode) {
        ESP_LOGI(TAG, "Started in Station mode");
        crypto_price_t price_data;
        
        while (1) {
            if (fetch_crypto_price(&price_data)) {
                update_display(&price_data);
            } else {
                ESP_LOGE(TAG, "Failed to fetch crypto price");
            }

            // Get refresh interval from NVS
            nvs_handle_t nvs_handle;
            uint32_t refresh_interval = 60; // default 60 seconds
            if (nvs_open("storage", NVS_READONLY, &nvs_handle) == ESP_OK) {
                nvs_get_u32(nvs_handle, "interval", &refresh_interval);
                nvs_close(nvs_handle);
            }

            vTaskDelay(pdMS_TO_TICKS(refresh_interval * 1000));
        }
    }
}
