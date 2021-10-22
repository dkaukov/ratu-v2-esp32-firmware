#pragma once

#define DEVICE_ID "atu-01"

#define IP_CONFIGURATION_TYPE IP_CONFIGURATION_TYPE_STATIC
#define IP_CONFIGURATION_ADDRESS 192, 168, 1, 184
#define IP_CONFIGURATION_MASK 255, 255, 255, 0
#define IP_CONFIGURATION_GW 192, 168, 1, 1

#define SYSLOG_SERVER "elk.sphere.home"
#define SYSLOG_PORT 5140
#define SYSLOG_DEVICE_HOSTNAME ""
#define SYSLOG_APP_NAME "ratu"

#define NTP_SERVER "router.sphere.home"

#define MQTT_HOST_NAME "mqtt.sphere.home"
#define MQTT_CONFIG_TOPIC_ROOT "ratu-v2/configurations"
#define MQTT_DEVICE_TOPIC_ROOT "ratu-v2/devices"
#define MQTT_KEEP_ALIVE_SEC 5
