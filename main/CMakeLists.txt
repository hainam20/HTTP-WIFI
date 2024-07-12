idf_component_register(SRCS "app_nvs.c" "http_server_app.c" "wifi.c" "main.c" "esp_mqtt.c"
                    INCLUDE_DIRS "."
                    EMBED_FILES webpages/app.css webpages/index.html webpages/app.js webpages/jquery-3.3.1.min.js)
