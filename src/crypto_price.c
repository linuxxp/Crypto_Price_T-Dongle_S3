#include "crypto_price.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "SSD1306.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "crypto_price";
static char selected_crypto[10] = "BTC";
static char selected_currency[4] = "USD";

void init_crypto_price(void)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        size_t required_size;
        if (nvs_get_str(nvs_handle, "crypto", NULL, &required_size) == ESP_OK) {
            nvs_get_str(nvs_handle, "crypto", selected_crypto, &required_size);
        }
        if (nvs_get_str(nvs_handle, "currency", NULL, &required_size) == ESP_OK) {
            nvs_get_str(nvs_handle, "currency", selected_currency, &required_size);
        }
        nvs_close(nvs_handle);
    }

    // Initialize display
    ssd1306_init();
}

bool fetch_crypto_price(crypto_price_t* price_data)
{
    char url[256];
    snprintf(url, sizeof(url), 
             "https://api.coingecko.com/api/v3/simple/price?ids=%s&vs_currencies=%s&include_24h_change=true",
             selected_crypto, selected_currency);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    bool success = false;

    if (err == ESP_OK) {
        char *response_buffer = malloc(1024);
        int response_len = esp_http_client_read(client, response_buffer, 1024);
        if (response_len > 0) {
            response_buffer[response_len] = 0;
            
            cJSON *root = cJSON_Parse(response_buffer);
            if (root) {
                cJSON *crypto = cJSON_GetObjectItem(root, selected_crypto);
                if (crypto) {
                    cJSON *price = cJSON_GetObjectItem(crypto, selected_currency);
                    cJSON *change = cJSON_GetObjectItem(crypto, "24h_change");
                    
                    if (price && change) {
                        strncpy(price_data->symbol, selected_crypto, sizeof(price_data->symbol) - 1);
                        price_data->price = price->valuedouble;
                        price_data->change_24h = change->valuedouble;
                        success = true;
                    }
                }
                cJSON_Delete(root);
            }
        }
        free(response_buffer);
    }
    
    esp_http_client_cleanup(client);
    return success;
}

void update_display(const crypto_price_t* price_data)
{
    ssd1306_clear_screen();
    
    // Display crypto symbol and price
    char price_str[32];
    snprintf(price_str, sizeof(price_str), "%s: %.2f %s", 
             price_data->symbol, price_data->price, selected_currency);
    
    ssd1306_set_text_size(1);
    ssd1306_set_cursor(0, 0);
    ssd1306_write_string(price_str);
    
    // Display 24h change
    char change_str[32];
    snprintf(change_str, sizeof(change_str), "24h: %.2f%%", price_data->change_24h);
    
    ssd1306_set_cursor(0, 16);
    ssd1306_write_string(change_str);
    
    ssd1306_refresh();
}
