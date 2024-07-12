#ifndef _ESP_MQTT_H_
#define _ESP_MQTT_H_

#include "mqtt_client.h"

#define URI "mqtt://mqtt.flespi.io"
#define ClientId "ESP32"
#define Username "ah2cbuQ57EsGfyGVtW7939tlbBprFq17Gwdbie3jcRiG57SHrZduYBRGhotdrrcL"
#define Password ""
#define MAX_URI_LENGTH 256
#define MAX_CLIENT_ID_LENGTH 128
#define MAX_USERNAME_LENGTH 128
#define MAX_PASSWORD_LENGTH 128
void mqtt_start(void);
void mqtt_stop(void);
void Publisher_Task(char *data, int lenght);

#endif