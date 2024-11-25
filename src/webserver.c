#include "webserver.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "webserver";

// HTML template for configuration page
static const char* config_html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CryptoDongle Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 0; padding: 20px; }
        .container { max-width: 500px; margin: 0 auto; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; }
        input, select { width: 100%; padding: 8px; margin-bottom: 10px; }
        button { background: #4CAF50; color: white; padding: 10px 15px; border: none; cursor: pointer; }
        button:hover { background: #45a049; }
    </style>
</head>
<body>
    <div class="container">
        <h2>CryptoDongle Configuration</h2>
        <form id="configForm" method="POST" action="/save">
            <div class="form-group">
                <label>WiFi Settings:</label>
                <input type="text" name="ssid" placeholder="WiFi SSID" required>
                <input type="password" name="password" placeholder="WiFi Password">
            </div>
            <div class="form-group">
                <label>Cryptocurrency:</label>
                <select name="crypto">
                    <option value="BTC">Bitcoin (BTC)</option>
                    <option value="ETH">Ethereum (ETH)</option>
                    <option value="SOL">Solana (SOL)</option>
                    <option value="ADA">Cardano (ADA)</option>
                    <option value="XRP">Ripple (XRP)</option>
                    <option value="LTC">Litecoin (LTC)</option>
                    <option value="ETC">Ethereum Classic (ETC)</option>
                    <option value="DOGE">Dogecoin (DOGE)</option>
                    <option value="HNT">Helium (HNT)</option>
                </select>
            </div>
            <div class="form-group">
                <label>Currency:</label>
                <select name="currency">
                    <option value="USD">USD</option>
                    <option value="EUR">EUR</option>
                </select>
            </div>
            <div class="form-group">
                <label>Refresh Interval (seconds):</label>
                <input type="number" name="interval" min="30" max="3600" value="60">
            </div>
            <button type="submit">Save Configuration</button>
        </form>
    </div>
</body>
</html>
)";

// Structure to hold submitted form data
typedef struct {
    char ssid[32];
    char password[64];
    char crypto[10];
    char currency[4];
    int interval;
} form_data_t;

// Parse form data from POST request
static void parse_form_data(char* buf, size_t buf_len, form_data_t* form_data)
{
    char* token;
    char* rest = buf;

    while ((token = strtok_r(rest, "&", &rest))) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = 0;
            char* value = eq + 1;
            
            // URL decode the value
            char decoded_value[64] = {0};
            int decoded_len = 0;
            for (int i = 0; value[i]; i++) {
                if (value[i] == '%' && value[i+1] && value[i+2]) {
                    char hex[3] = {value[i+1], value[i+2], 0};
                    decoded_value[decoded_len++] = strtol(hex, NULL, 16);
                    i += 2;
                } else if (value[i] == '+') {
                    decoded_value[decoded_len++] = ' ';
                } else {
                    decoded_value[decoded_len++] = value[i];
                }
            }
            
            // Store values in form_data structure
            if (strcmp(token, "ssid") == 0) {
                strncpy(form_data->ssid, decoded_value, sizeof(form_data->ssid) - 1);
            } else if (strcmp(token, "password") == 0) {
                strncpy(form_data->password, decoded_value, sizeof(form_data->password) - 1);
            } else if (strcmp(token, "crypto") == 0) {
                strncpy(form_data->crypto, decoded_value, sizeof(form_data->crypto) - 1);
            } else if (strcmp(token, "currency") == 0) {
                strncpy(form_data->currency, decoded_value, sizeof(form_data->currency) - 1);
            } else if (strcmp(token, "interval") == 0) {
                form_data->interval = atoi(decoded_value);
            }
        }
    }
}

// Handle root path request
static esp_err_t root_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, config_html, strlen(config_html));
    return ESP_OK;
}

// Handle form submission
static esp_err_t save_handler(httpd_req_t *req)
{
    char buf[1024];
    int ret, remaining = req->content_len;
    
    if (remaining > sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Content too long");
        return ESP_FAIL;
    }
    
    // Read POST data
    ret = httpd_req_recv(req, buf, remaining);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    // Parse form data
    form_data_t form_data = {0};
    parse_form_data(buf, sizeof(buf), &form_data);
    
    // Save configuration to NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        nvs_set_str(nvs_handle, "wifi_ssid", form_data.ssid);
        nvs_set_str(nvs_handle, "wifi_pass", form_data.password);
        nvs_set_str(nvs_handle, "crypto", form_data.crypto);
        nvs_set_str(nvs_handle, "currency", form_data.currency);
        nvs_set_u32(nvs_handle, "interval", form_data.interval);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
        
        // Send response and restart ESP
        const char* response = "Configuration saved. Device will restart in 5 seconds...";
        httpd_resp_send(req, response, strlen(response));
        
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
        
        return ESP_OK;
    }
    
    // If we get here, something went wrong
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to save configuration");
    return ESP_FAIL;
}

// Start the web server
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    
    // URI handlers
    httpd_uri_t root = {
        .uri       = "/",
        .method    = HTTP_GET,
        .handler   = root_handler,
        .user_ctx  = NULL
    };
    
    httpd_uri_t save = {
        .uri       = "/save",
        .method    = HTTP_POST,
        .handler   = save_handler,
        .user_ctx  = NULL
    };
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &save);
        ESP_LOGI(TAG, "Web server started");
        return server;
    }

    ESP_LOGE(TAG, "Failed to start web server");
    return NULL;
}

// Stop the web server
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}
