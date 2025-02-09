#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "wifi.h"
#include "esp_mqtt.h"
// #include "lora.h"
#include "cJSON.h"
static const char *TAG = "MQTT_EXAMPLE";

esp_mqtt_client_handle_t client;
esp_mqtt_client_config_t *mqtt_config = NULL;
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRId32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

        msg_id = esp_mqtt_client_subscribe(client, "/flespi/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/flespi/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/flespi/qos0", NULL, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        if (strcmp(event->topic, "/flespi/qos1") == 0)
        {
            uint8_t buf[256];
            int send_len = sprintf((char *)buf, event->data);
            // printf("%s \n", buf);
            // send_lora_packet(buf, send_len);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
static void mqtt_app_start(void)
{
    mqtt_config = (esp_mqtt_client_config_t *)malloc(sizeof(esp_mqtt_client_config_t));
    memset(mqtt_config, 0x00, sizeof(esp_mqtt_client_config_t));

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = URI,
        .credentials.client_id = ClientId,
        .credentials.username = Username,
        .credentials.authentication.password = Password};

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void mqtt_start(void)
{
    mqtt_app_start();
}

void mqtt_stop(void)
{
    esp_err_t err = esp_mqtt_client_disconnect(client);
    if (err == ESP_OK)
    {
        printf("Disconnected mqtt\n");
    }
    else
    {
        printf("Failed to disconnect: %s\n", esp_err_to_name(err));
    }
    esp_mqtt_client_stop(client);
}
void Publisher_Task(char *data, int lenght)
{
    if (client == NULL)
    {
        mqtt_app_start();
    }
    esp_mqtt_client_publish(client, "/flespi/qos0", data, 0, 0, 0);
}
esp_mqtt_client_config_t *mqtt_app_get_config(void)
{
    if (mqtt_config == NULL)
    {
        mqtt_config = (esp_mqtt_client_config_t *)malloc(sizeof(esp_mqtt_client_config_t));
    }
    return mqtt_config;
}