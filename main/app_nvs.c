#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "app_nvs.h"
#include "wifi.h"
#include "esp_mqtt.h"

// Tag for logging to the monitor
static const char TAG[] = "nvs";

// NVS name space used for station mode credentials
const char app_nvs_sta_creds_namespace[] = "stacreds";

// NVS name space used for MQTT congfig
const char app_nvs_mqtt_cfg_namespace[] = "mqttcfg";

esp_err_t app_nvs_save_sta_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI(TAG, "app_nvs_save_sta_creds: Saving station mode credentials to flash");

    wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();

    if (wifi_sta_config)
    {
        esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_sta_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Set SSID
        esp_err = nvs_set_blob(handle, "ssid", wifi_sta_config->sta.ssid, MAX_SSID_LENGTH);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_sta_creds: Error (%s) setting SSID to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Set Password
        esp_err = nvs_set_blob(handle, "password", wifi_sta_config->sta.password, MAX_PASSWORD_LENGTH);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_sta_creds: Error (%s) setting Password to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Commit credentials to NVS
        esp_err = nvs_commit(handle);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_sta_creds: Error (%s) comitting credentials to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }
        nvs_close(handle);
        ESP_LOGI(TAG, "app_nvs_save_sta_creds: wrote wifi_sta_config: Station SSID: %s Password: %s", wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
    }

    printf("app_nvs_save_sta_creds: returned ESP_OK\n");
    return ESP_OK;
}

bool app_nvs_load_sta_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "app_nvs_load_sta_creds: Loading Wifi credentials from flash");

    if (nvs_open(app_nvs_sta_creds_namespace, NVS_READONLY, &handle) == ESP_OK)
    {
        wifi_config_t *wifi_sta_config = wifi_app_get_wifi_config();

        memset(wifi_sta_config, 0x00, sizeof(wifi_config_t));

        // Allocate buffer
        size_t wifi_config_size = sizeof(wifi_config_t);
        uint8_t *wifi_config_buff = (uint8_t *)malloc(sizeof(uint8_t) * wifi_config_size);
        memset(wifi_config_buff, 0x00, sizeof(wifi_config_size));

        // Load SSID
        wifi_config_size = sizeof(wifi_sta_config->sta.ssid);
        esp_err = nvs_get_blob(handle, "ssid", wifi_config_buff, &wifi_config_size);
        if (esp_err != ESP_OK)
        {
            free(wifi_config_buff);
            printf("app_nvs_load_sta_creds: (%s) no station SSID found in NVS\n", esp_err_to_name(esp_err));
            return false;
        }
        memcpy(wifi_sta_config->sta.ssid, wifi_config_buff, wifi_config_size);

        // Load Password
        wifi_config_size = sizeof(wifi_sta_config->sta.password);
        esp_err = nvs_get_blob(handle, "password", wifi_config_buff, &wifi_config_size);
        if (esp_err != ESP_OK)
        {
            free(wifi_config_buff);
            printf("app_nvs_load_sta_creds: (%s) retrieving password!\n", esp_err_to_name(esp_err));
            return false;
        }
        memcpy(wifi_sta_config->sta.password, wifi_config_buff, wifi_config_size);

        free(wifi_config_buff);
        nvs_close(handle);

        printf("app_nvs_load_sta_creds: SSID: %s Password: %s\n", wifi_sta_config->sta.ssid, wifi_sta_config->sta.password);
        return wifi_sta_config->sta.ssid[0] != '\0';
    }
    else
    {
        return false;
    }
}

esp_err_t app_nvs_clear_sta_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI(TAG, "app_nvs_clear_sta_creds: Clearing Wifi station mode credentials from flash");

    esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_sta_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Erase credentials
    esp_err = nvs_erase_all(handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_sta_creds: Error (%s) erasing station mode credentials!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Commit clearing credentials from NVS
    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_sta_creds: Error (%s) NVS commit!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    nvs_close(handle);

    printf("app_nvs_clear_sta_creds: returned ESP_OK\n");
    return ESP_OK;
}

esp_err_t app_nvs_save_mqtt_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI("MQTT_APP", "app_nvs_save_mqtt_creds: Saving MQTT credentials to flash");

    esp_mqtt_client_config_t *mqtt_cfg = mqtt_app_get_mqtt_config();

    if (mqtt_cfg)
    {
        esp_err = nvs_open(app_nvs_mqtt_cfg_namespace, NVS_READWRITE, &handle);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Set URI
        esp_err = nvs_set_str(handle, "uri", mqtt_cfg->broker.address.uri);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) setting URI to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Set Client ID
        esp_err = nvs_set_str(handle, "client_id", mqtt_cfg->credentials.client_id);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) setting Client ID to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }

        // Set Username
        esp_err = nvs_set_str(handle, "username", mqtt_cfg->credentials.username);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) setting Username to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }
        // Set Password
        esp_err = nvs_set_str(handle, "password", mqtt_cfg->credentials.authentication.password);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) setting Password to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }
        // Commit credentials to NVS
        esp_err = nvs_commit(handle);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_save_mqtt_creds: Error (%s) committing credentials to NVS!\n", esp_err_to_name(esp_err));
            return esp_err;
        }
        nvs_close(handle);
        ESP_LOGI("MQTT_APP", "app_nvs_save_mqtt_creds: wrote mqtt_cfg: URI: %s Client ID: %s Username: %s Password: %s", mqtt_cfg->broker.address.uri, mqtt_cfg->credentials.client_id, mqtt_cfg->credentials.username, mqtt_cfg->credentials.authentication.password);
    }

    return ESP_OK;
}
bool app_nvs_load_mqtt_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI("MQTT_APP", "app_nvs_load_mqtt_creds: Loading MQTT credentials from flash");

    if (nvs_open(app_nvs_mqtt_cfg_namespace, NVS_READONLY, &handle) == ESP_OK)
    {
        esp_mqtt_client_config_t *mqtt_cfg = mqtt_app_get_mqtt_config();

        // Initialize memory
        memset(mqtt_cfg, 0x00, sizeof(esp_mqtt_client_config_t));

        // Load URI
        size_t uri_size = MAX_URI_LENGTH;
        char uri[MAX_URI_LENGTH];
        esp_err = nvs_get_str(handle, "uri", uri, &uri_size);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_load_mqtt_creds: (%s) no MQTT URI found in NVS\n", esp_err_to_name(esp_err));
            return false;
        }
        mqtt_cfg->broker.address.uri = strdup(uri);

        // Load Client ID
        size_t client_id_size = MAX_CLIENT_ID_LENGTH;
        char client_id[MAX_CLIENT_ID_LENGTH];
        esp_err = nvs_get_str(handle, "client_id", client_id, &client_id_size);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_load_mqtt_creds: (%s) no MQTT Client ID found in NVS\n", esp_err_to_name(esp_err));
            return false;
        }
        mqtt_cfg->credentials.client_id = strdup(client_id);

        // Load Username
        size_t username_size = MAX_USERNAME_LENGTH;
        char username[MAX_USERNAME_LENGTH];
        esp_err = nvs_get_str(handle, "username", username, &username_size);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_load_mqtt_creds: (%s) no MQTT Username found in NVS\n", esp_err_to_name(esp_err));
            return false;
        }
        mqtt_cfg->credentials.username = strdup(username);

        // Load Password
        size_t password_size = MAX_PASSWORD_LENGTH;
        char password[MAX_PASSWORD_LENGTH];
        esp_err = nvs_get_str(handle, "password", password, &password_size);
        if (esp_err != ESP_OK)
        {
            printf("app_nvs_load_mqtt_creds: (%s) no MQTT Password found in NVS\n", esp_err_to_name(esp_err));
            return false;
        }
        mqtt_cfg->credentials.authentication.password = strdup(password);

        nvs_close(handle);

        printf("app_nvs_load_mqtt_creds: URI: %s Client ID: %s Username: %s Password: %s\n",
               mqtt_cfg->broker.address.uri,
               mqtt_cfg->credentials.client_id,
               mqtt_cfg->credentials.username,
               mqtt_cfg->credentials.authentication.password);

        return true;
    }
    else
    {
        return false;
    }
}

esp_err_t app_nvs_clear_mqtt_creds(void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI("MQTT_APP", "app_nvs_clear_mqtt_creds: Clearing MQTT credentials from flash");

    esp_err = nvs_open(app_nvs_mqtt_cfg_namespace, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_mqtt_creds: Error (%s) opening NVS handle!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Erase all credentials
    esp_err = nvs_erase_all(handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_mqtt_creds: Error (%s) erasing MQTT credentials!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Commit clearing credentials from NVS
    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_clear_mqtt_creds: Error (%s) committing NVS erase!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    nvs_close(handle);

    printf("app_nvs_clear_mqtt_creds: returned ESP_OK\n");
    return ESP_OK;
}