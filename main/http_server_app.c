#include "http_server_app.h"
#include <stdint.h>
#include "esp_mac.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "wifi.h"
static const char *TAG = "HTTP_SERVER";

static httpd_handle_t http_server_handle = NULL;

// static httpd_req_t *REG;

static int g_wifi_connect_status = NONE;

extern const uint8_t jquery_3_3_1_min_js_start[] asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[] asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");
extern const uint8_t app_css_start[] asm("_binary_app_css_start");
extern const uint8_t app_css_end[] asm("_binary_app_css_end");
extern const uint8_t app_js_start[] asm("_binary_app_js_start");
extern const uint8_t app_js_end[] asm("_binary_app_js_end");
/**
 * Set JSON callback
 */
static http_get_callback_t http_get_json_callback = NULL;

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Jquery requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

    return ESP_OK;
}
/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "index.html requested");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

    return ESP_OK;
}
/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.css requested");

    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

    return ESP_OK;
}

/**
 * wifiConnect.json handler is invoked after the connect button is pressed
 * and handles receiving the SSID and password entered by the user
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "/wifiConnect.json requested");

    size_t len_ssid = 0, len_pass = 0;
    char *ssid_str = NULL, *pass_str = NULL;

    // Get SSID header
    len_ssid = httpd_req_get_hdr_value_len(req, "my-connect-ssid") + 1;
    if (len_ssid > 1)
    {
        ssid_str = malloc(len_ssid);
        if (httpd_req_get_hdr_value_str(req, "my-connect-ssid", ssid_str, len_ssid) == ESP_OK)
        {
            ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-ssid: %s", ssid_str);
        }
    }

    // Get Password header
    len_pass = httpd_req_get_hdr_value_len(req, "my-connect-pwd") + 1;
    if (len_pass > 1)
    {
        pass_str = malloc(len_pass);
        if (httpd_req_get_hdr_value_str(req, "my-connect-pwd", pass_str, len_pass) == ESP_OK)
        {
            ESP_LOGI(TAG, "http_server_wifi_connect_json_handler: Found header => my-connect-pwd: %s", pass_str);
        }
    }

    // Update the Wifi networks configuration and let the wifi application know
    wifi_config_t *wifi_config = wifi_app_get_wifi_config();
    memset(wifi_config, 0x00, sizeof(wifi_config_t));
    memcpy(wifi_config->sta.ssid, ssid_str, len_ssid);
    memcpy(wifi_config->sta.password, pass_str, len_pass);
    wifi_app_send_message(WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER);

    free(ssid_str);
    free(pass_str);

    return ESP_OK;
}
/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "app.js requested");

    httpd_resp_set_type(req, "application/javascript");
    httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

    return ESP_OK;
}

/**
 * wifiConnectStatus handler updates the connection status for the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "/wifiConnectStatus requested");

    char statusJSON[100];

    sprintf(statusJSON, "{\"wifi_connect_status\":%d}", g_wifi_connect_status);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, statusJSON, strlen(statusJSON));

    return ESP_OK;
}

/**
 * wifiDisconnect.json handler responds by sending a message to the Wifi application to disconnect.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_wifi_disconnect_json_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "wifiDisconect.json requested");

    wifi_app_send_message(WIFI_APP_MSG_USER_REQUESTED_STA_DISCONNECT);

    return ESP_OK;
}
//  esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
// {
//     if (strcmp("/", req->uri) == 0) {
//         httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
//         /* Return ESP_OK to keep underlying socket open */
//         return ESP_OK;
//     } else if (strcmp("/echo", req->uri) == 0) {
//         httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/echo URI is not available");
//         /* Return ESP_FAIL to close underlying socket */
//         return ESP_FAIL;
//     }
//     /* For any other URI send 404 and close socket */
//     httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
//     return ESP_FAIL;
// }

static httpd_handle_t http_sever_configure(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.core_id = 0;

    // Adjust the default priority to 1 less than the wifi application task
    config.task_priority = 4;

    // Bump up the stack size (default is 4096)
    config.stack_size = 8192;

    // Increase uri handlers
    config.max_uri_handlers = 20;

    // Increase the timeout limits
    config.recv_wait_timeout = 10;
    config.send_wait_timeout = 10;

    ESP_LOGI(TAG,
             "http_server_configure: Starting server on port: '%d' with task priority: '%d'",
             config.server_port,
             config.task_priority);
    // Start the httpd http_sever_handle

    if (httpd_start(&http_server_handle, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");

        // register jquery.js handler
        httpd_uri_t jquery_js = {
            .uri = "/jquery-3.3.1.min.js",
            .method = HTTP_GET,
            .handler = http_server_jquery_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &jquery_js);

        // register index.html handler
        httpd_uri_t index_html = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = http_server_index_html_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &index_html);
        // register app.css handler
        httpd_uri_t app_css = {
            .uri = "/app.css",
            .method = HTTP_GET,
            .handler = http_server_app_css_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &app_css);

        // register wifiConnect.json handler
        httpd_uri_t wifi_connect_json = {
            .uri = "/wifiConnect.json",
            .method = HTTP_POST,
            .handler = http_server_wifi_connect_json_handler,
            .user_ctx = NULL};

        // register wifiConnectStatus.json handler
        httpd_uri_t wifi_connect_status_json = {
            .uri = "/wifiConnectStatus",
            .method = HTTP_POST,
            .handler = http_server_wifi_connect_status_json_handler,
            .user_ctx = NULL};

        // register wifiDisconnect.json handler
        httpd_uri_t wifi_disconnect_json = {
            .uri = "/wifiDisconnect.json",
            .method = HTTP_DELETE,
            .handler = http_server_wifi_disconnect_json_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(http_server_handle, &wifi_disconnect_json);
        httpd_register_uri_handler(http_server_handle, &wifi_connect_json);
        return http_server_handle;
    }
    else
    {
        ESP_LOGI(TAG, "Error starting server!");
    }
    return NULL;
}
void start_webserver(void)
{
    if (http_server_handle == NULL)
    {
        http_server_handle = http_sever_configure();
    }
}
void stop_webserver(void)
{
    // Stop the httpd server
    if (http_server_handle)
    {
        httpd_stop(http_server_handle);
        ESP_LOGI(TAG, "HTTP SERVER STOP");
        http_server_handle = NULL;
    }
}
void http_set_json_callback(void *cb)
{
    http_get_json_callback = cb;
}
