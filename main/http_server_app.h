#ifndef _HTTP_SERVER_APP_H
#define _HTTP_SERVER_APP_H
#include <stdint.h>

typedef void (*http_get_callback_t)(void);
typedef enum http_server_wifi_connect_status
{
    NONE = 0,
    HTTP_WIFI_STATUS_CONNECTING,
    HTTP_WIFI_STATUS_CONNECT_FAILED,
    HTTP_WIFI_STATUS_CONNECT_SUCCESS,
    HTTP_WIFI_STATUS_DISCONNECTED,
} http_server_wifi_connect_status_e;

void start_webserver(void);
void stop_webserver(void);
void http_set_json_callback(void *cb);
// void json_response(char *data, int len);
#endif