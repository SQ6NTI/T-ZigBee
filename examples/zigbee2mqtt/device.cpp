
/******************************************************************************/
/***        include files                                                   ***/
/******************************************************************************/

#include "device.h"
#include "app_mqtt.h"
#include "app_db.h"

#include "zbhci.h"
#include "cJSON.h"
#include "esp_log.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

/******************************************************************************/
/***        macro definitions                                               ***/
/******************************************************************************/

/******************************************************************************/
/***        type definitions                                                ***/
/******************************************************************************/

/******************************************************************************/
/***        local function prototypes                                       ***/
/******************************************************************************/

static void subHumidity(uint64_t u64IeeeAddr, cJSON *json);

static void subTemperature(uint64_t u64IeeeAddr, cJSON *json);

static void subPressure(uint64_t u64IeeeAddr, cJSON *json);

static void subOccupancy(uint64_t u64IeeeAddr, cJSON *json);

static void subIlluminanceLux(uint64_t u64IeeeAddr, cJSON *json);

static void handleLilygoLight(const char *topic, const char *data);

static void handleTuyaPlug(const char * topic, const char *data);

static void handleTradfriPlug(const char * topic, const char *data);

static void handleMQTTAttrGet(const char * topic, const char *data);

/******************************************************************************/
/***        exported variables                                              ***/
/******************************************************************************/

/******************************************************************************/
/***        local variables                                                 ***/
/******************************************************************************/

/******************************************************************************/
/***        exported functions                                              ***/
/******************************************************************************/

void wsdcgq11lmAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    cJSON *json_humidity = NULL;
    cJSON *json_temperature = NULL;
    cJSON *json_pressure = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "Xiaomi");
    cJSON_AddStringToObject(
        device,
        "model",
        "Aqara temperature, humidity and pressure sensor (WSDCGQ11LM)"
    );
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddStringToObject(device, "sw_version", "3000-0001");
    cJSON_AddItemToObject(json, "device", device);

    json_humidity = cJSON_Duplicate(json, 1);
    if (json_humidity) {
        subHumidity(u64IeeeAddr, json_humidity);
        cJSON_Delete(json_humidity);
    }
    json_temperature = cJSON_Duplicate(json, 1);
    if (json_humidity) {
        subTemperature(u64IeeeAddr, json_temperature);
        cJSON_Delete(json_temperature);
    }
    json_pressure = cJSON_Duplicate(json, 1);
    if (json_humidity) {
        subPressure(u64IeeeAddr, json_pressure);
        cJSON_Delete(json_pressure);
    }
    ESP_LOGI(
        "Zigbee2MQTT",
        "Successfully interviewed '%#016llx', device has successfully been paired",
        u64IeeeAddr
    );
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void wsdcgq11lmDelete(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/humidity/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/temperature/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/pressure/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");
}


void wsdcgq11lmReport(
    uint64_t u64IeeeAddr,
    int16_t  i16Temperature,
    int16_t  i16Humidity,
    int16_t  i16Pressure
) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);

    printf("wsdcgq11lm_report temp %d\n", i16Temperature);
    printf("wsdcgq11lm_report humid %d\n", i16Humidity);
    printf("wsdcgq11lm_report press %d\n", i16Pressure);

    cJSON_AddNumberToObject(json, "temperature", (double)i16Temperature/100);
    cJSON_AddNumberToObject(json, "humidity",    (double)i16Humidity/100);
    cJSON_AddNumberToObject(json, "pressure",    (double)i16Pressure);
    char *str = cJSON_Print(json);

    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
    cJSON_Delete(json);
}


void rtcgq11lmAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    cJSON *json_illuminance_lux = NULL;
    cJSON *json_occupancy = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "Xiaomi");
    cJSON_AddStringToObject(
        device,
        "model",
        "Aqara human body movement and illuminance sensor (RTCGQ11LM)"
    );
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddStringToObject(device, "sw_version", "3000-0001");
    cJSON_AddItemToObject(json, "device", device);

    json_illuminance_lux = cJSON_Duplicate(json, 1);
    if (json_illuminance_lux) {
        subIlluminanceLux(u64IeeeAddr, json_illuminance_lux);
        cJSON_Delete(json_illuminance_lux);
    }
    json_occupancy = cJSON_Duplicate(json, 1);
    if (json_occupancy) {
        subOccupancy(u64IeeeAddr, json_occupancy);
        cJSON_Delete(json_occupancy);
    }

    ESP_LOGI(
        "Zigbee2MQTT",
        "Successfully interviewed '%#016llx', device has successfully been paired",
        u64IeeeAddr
    );
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void rtcgq11lmDelete(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/illuminance_lux/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic ,"");
    mqtt.publish(topic, "");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/binary_sensor/%s/occupancy/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");
}


void rtcgq11lmReport(
    uint64_t u64IeeeAddr,
    int8_t   u8Occupancy,
    uint16_t u16Illuminance
) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);

    cJSON_AddBoolToObject(json, "occupancy", u8Occupancy);
    cJSON_AddNumberToObject(json, "illuminance", u16Illuminance);
    char *str = cJSON_Print(json);

    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
    cJSON_Delete(json);
}


void lilygoLightAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };
    char topic_head[128] = { 0 };
    char subTopics[128] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    char *str = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic_head, sizeof(topic_head) - 1,
        "homeassistant/light/0x%016llx",
        u64IeeeAddr
    );
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);\
    snprintf(subTopics, sizeof(subTopics) - 1, "%s/set", topic_head);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "LILYGO");
    cJSON_AddStringToObject(device, "model", "LILYGO ordinary light");
    cJSON_AddStringToObject(device, "name", "LILYGO.Light");
    cJSON_AddStringToObject(device, "sw_version", "0.1.0");
    cJSON_AddItemToObject(json, "device", device);
    cJSON_AddStringToObject(json, "~", topic_head);
    cJSON_AddStringToObject(json, "name", "LILYGO.Light");
    cJSON_AddStringToObject(json, "cmd_t", "~/set");
    cJSON_AddStringToObject(json, "stat_t", "~/state");
    cJSON_AddStringToObject(json, "schema", "json");
    cJSON_AddStringToObject(json, "unique_id", ieeeaddr_str);

    str = cJSON_Print(json);
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
    mqtt.subscribe(subTopics, handleLilygoLight);
    ESP_LOGI(
        "Zigbee2MQTT",
        "Successfully interviewed '%#016llx', device has successfully been paired",
        u64IeeeAddr
    );
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void lilygoLightDelete(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/light/%s/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");
}


void lilygoLightReport(uint64_t u64IeeeAddr, uint8_t u8OnOff) {
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    if (!json) return ;

    // printf("lilygoLightReport %d\n", u8OnOff);

    snprintf(topic, sizeof(topic) - 1, "homeassistant/light/0x%016llx/state", u64IeeeAddr);
    if (u8OnOff == 0x01) {
        cJSON_AddStringToObject(json, "state", "ON");
    } else {
        cJSON_AddStringToObject(json, "state", "OFF");
    }

    // cJSON_AddStringToObject(json, "state", u8OnOff ? "ON": "OFF");
    char *str = cJSON_Print(json);
    // printf("topic: %s, data: %s\n", topic, str);
    // appMQTTPublish(topic, str);
    mqtt.publish(topic ,str);
    cJSON_Delete(json);
}


void espressifLightAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };
    char topic_head[128] = { 0 };
    char subTopics[128] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    char *str = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic_head, sizeof(topic_head) - 1,
        "homeassistant/light/0x%016llx",
        u64IeeeAddr
    );
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);\
    snprintf(subTopics, sizeof(subTopics) - 1, "%s/set", topic_head);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "Espressif");
    cJSON_AddStringToObject(device, "model", "Espressif ordinary light");
    cJSON_AddStringToObject(device, "name", "Espressif.Light");
    cJSON_AddStringToObject(device, "sw_version", "0.1.0");
    cJSON_AddItemToObject(json, "device", device);
    cJSON_AddStringToObject(json, "~", topic_head);
    cJSON_AddStringToObject(json, "name", "Espressif.Light");
    cJSON_AddStringToObject(json, "cmd_t", "~/set");
    cJSON_AddStringToObject(json, "stat_t", "~/state");
    cJSON_AddStringToObject(json, "schema", "json");
    cJSON_AddStringToObject(json, "unique_id", ieeeaddr_str);

    str = cJSON_Print(json);
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
    mqtt.subscribe(subTopics, handleLilygoLight);
    ESP_LOGI(
        "Zigbee2MQTT",
        "Successfully interviewed '%#016llx', device has successfully been paired",
        u64IeeeAddr
    );
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}


void espressifLightDelete(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/light/%s/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");
}


void espressifLightReport(uint64_t u64IeeeAddr, uint8_t u8OnOff) {
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    if (!json) return ;

    // printf("lilygoLightReport %d\n", u8OnOff);

    snprintf(topic, sizeof(topic) - 1, "homeassistant/light/0x%016llx/state", u64IeeeAddr);
    if (u8OnOff == 0x01) {
        cJSON_AddStringToObject(json, "state", "ON");
    } else {
        cJSON_AddStringToObject(json, "state", "OFF");
    }

    // cJSON_AddStringToObject(json, "state", u8OnOff ? "ON": "OFF");
    char *str = cJSON_Print(json);
    // printf("topic: %s, data: %s\n", topic, str);
    // appMQTTPublish(topic, str);
    mqtt.publish(topic ,str);
    cJSON_Delete(json);
}

void lilygoSensorAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    cJSON *json_humidity = NULL;
    cJSON *json_temperature = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "LILYGO");
    cJSON_AddStringToObject(device, "model", "LILYGO temp/humi sensor");
    cJSON_AddStringToObject(device, "name", "LILYGO.Sensor");
    cJSON_AddStringToObject(device, "sw_version", "0.1.0");
    cJSON_AddItemToObject(json, "device", device);
    cJSON_AddStringToObject(json, "name", "LILYGO.Sensor");
    cJSON_AddStringToObject(json, "schema", "json");
    cJSON_AddStringToObject(json, "uniq_id", ieeeaddr_str);

    json_humidity = cJSON_Duplicate(json, 1);
    if (json_humidity)
    {
        subHumidity(u64IeeeAddr, json_humidity);
        cJSON_Delete(json_humidity);
    }
    json_temperature = cJSON_Duplicate(json, 1);
    if (json_temperature)
    {
        subTemperature(u64IeeeAddr, json_temperature);
        cJSON_Delete(json_temperature);
    }
    ESP_LOGI(
        "Zigbee2MQTT",
        "Successfully interviewed '%#016llx', device has successfully been paired",
        u64IeeeAddr
    );
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}

void lilygoSensorDelete(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/humidity/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
    mqtt.publish(topic, "");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/temperature/config",
        ieeeaddr_str
    );
    // appMQTTPublish(topic, "");
     mqtt.publish(topic, "");
}


void lilygoSensorReport(
    uint64_t u64IeeeAddr,
    int16_t  i16Temperature,
    int16_t  i16Humidity
) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);

    cJSON_AddNumberToObject(json, "temperature", (double)i16Temperature/100);
    cJSON_AddNumberToObject(json, "humidity",    (double)i16Humidity/100);
    char *str = cJSON_Print(json);

    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
    cJSON_Delete(json);
}

void tuyaPlugAdd(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };
    char topic_head[128] = { 0 };
    char subTopics[128] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    char *str = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic_head, sizeof(topic_head) - 1, "homeassistant/switch/0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);
    snprintf(subTopics, sizeof(subTopics) - 1, "%s/set", topic_head);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "TuYa");
    cJSON_AddStringToObject(device, "model", "Smart plug (with power monitoring) (TS011F_plug_1)");
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddItemToObject(json, "device", device);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);
    cJSON_AddStringToObject(json, "name", ieeeaddr_str);
    cJSON_AddStringToObject(json, "payload_off", "OFF");
    cJSON_AddStringToObject(json, "payload_on", "ON");
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_switch_zigbee2mqtt", ieeeaddr_str);
    cJSON_AddStringToObject(json, "unique_id", topic);
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.state }}");

    str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);
    mqtt.publish(topic, str);
    mqtt.subscribe(subTopics, handleTuyaPlug);
    ESP_LOGI("Zigbee2MQTT", "Successfully interviewed '%#016llx', device has successfully been paired", u64IeeeAddr);
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}

void tuyaPlugDelete(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/switch/%s/config", ieeeaddr_str);
    mqtt.publish(topic ,"");
}

void tuyaPlugReport(uint64_t u64IeeeAddr, uint8_t u8OnOff)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    if (!json) return ;

    printf("tuyaPlugReport %d\n", u8OnOff);

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    if (u8OnOff == 0x01)
    {
        cJSON_AddStringToObject(json, "state", "ON");
    }
    else
    {
        cJSON_AddStringToObject(json, "state", "OFF");
    }

    // cJSON_AddStringToObject(json, "state", u8OnOff ? "ON": "OFF");
    char *str = cJSON_Print(json);
    // printf("topic: %s, data: %s\n", topic, str);
    mqtt.publish(topic ,str);
    cJSON_Delete(json);
}

void tradfriPlugAdd(uint64_t u64IeeeAddr) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };
    char topic_head[128] = { 0 };
    char subTopics[128] = { 0 };
    cJSON *json = cJSON_CreateObject();
    cJSON *device = cJSON_CreateObject();
    cJSON *identifiers = cJSON_CreateArray();
    char *str = NULL;

    if (!json) return ;
    if (!device) goto OUT;
    if (!identifiers) goto OUT1;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic_head, sizeof(topic_head) - 1, "homeassistant/switch/0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);
    snprintf(subTopics, sizeof(subTopics) - 1, "%s/set", topic_head);

    cJSON_AddItemToArray(identifiers, cJSON_CreateString(ieeeaddr_str));
    cJSON_AddItemToObject(device, "identifiers", identifiers);
    cJSON_AddStringToObject(device, "manufacturer", "IKEA");
    cJSON_AddStringToObject(device, "model", "TRADFRI control outlet");
    cJSON_AddStringToObject(device, "name", ieeeaddr_str);
    cJSON_AddItemToObject(json, "device", device);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);
    cJSON_AddStringToObject(json, "name", ieeeaddr_str);
    cJSON_AddStringToObject(json, "payload_off", "OFF");
    cJSON_AddStringToObject(json, "payload_on", "ON");
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_switch_zigbee2mqtt", ieeeaddr_str);
    cJSON_AddStringToObject(json, "unique_id", topic);
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.state }}");

    str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s/config", topic_head);
    mqtt.publish(topic, str);
    mqtt.subscribe(subTopics, handleTradfriPlug);
    ESP_LOGI("Zigbee2MQTT", "Successfully interviewed '%#016llx', device has successfully been paired", u64IeeeAddr);
OUT:
    cJSON_Delete(json);
    return;
OUT1:
    cJSON_Delete(json);
    cJSON_Delete(device);
}

void tradfriPlugDelete(uint64_t u64IeeeAddr)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "homeassistant/switch/%s/config", ieeeaddr_str);
    mqtt.publish(topic ,"");
}

void tradfriPlugReport(uint64_t u64IeeeAddr, uint8_t u8OnOff)
{
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    cJSON *json = cJSON_CreateObject();
    if (!json) return ;

    printf("tradfriPlugReport %d\n", u8OnOff);

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    if (u8OnOff == 0x01)
    {
        cJSON_AddStringToObject(json, "state", "ON");
    }
    else
    {
        cJSON_AddStringToObject(json, "state", "OFF");
    }

    // cJSON_AddStringToObject(json, "state", u8OnOff ? "ON": "OFF");
    char *str = cJSON_Print(json);
    // printf("topic: %s, data: %s\n", topic, str);
    mqtt.publish(topic ,str);
    cJSON_Delete(json);
}

void subMQTTAttrGet()
{
    char topic[128] = { 0 };

    printf("Subscribing to lilygo mqtt topics\n");

    snprintf(topic, sizeof(topic) - 1, "lilygo/readattr", topic);
    mqtt.subscribe(topic, handleMQTTAttrGet);
    //snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/0xa4c138c2ec163bd8/set");
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/0xa4c1387ca2489e61/set");
    mqtt.subscribe(topic, handleTuyaPlug);
}

/******************************************************************************/
/***        local functions                                                 ***/
/******************************************************************************/

static void subHumidity(uint64_t u64IeeeAddr, cJSON *json) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "humidity");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "humidity");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "humidity");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "\%");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.humidity }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/humidity/config",
        ieeeaddr_str
    );

    // push message
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
}


static void subTemperature(uint64_t u64IeeeAddr, cJSON *json) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "temperature");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "temperature");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "temperature");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "°C");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.temperature }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/temperature/config",
        ieeeaddr_str
    );

    // push message
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
}


static void subPressure(uint64_t u64IeeeAddr, cJSON *json) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "pressure");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "hPa");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.pressure }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/sensor/%s/pressure/config", 
        ieeeaddr_str
    );

    // push message
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
}


static void subOccupancy(uint64_t u64IeeeAddr, cJSON *json) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "motion");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s %s", ieeeaddr_str, "occupancy");
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddBoolToObject(json, "payload_off", false);
    cJSON_AddBoolToObject(json, "payload_on", true);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "pressure");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "hPa");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.occupancy }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1,
        "homeassistant/binary_sensor/%s/pressure/config", 
        ieeeaddr_str
    );

    // push message
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
}


static void subIlluminanceLux(uint64_t u64IeeeAddr, cJSON *json) {
    char ieeeaddr_str[20] = { 0 };
    char topic[128] = { 0 };

    if (!json) return ;

    snprintf(ieeeaddr_str, sizeof(ieeeaddr_str) - 1, "0x%016llx", u64IeeeAddr);

    cJSON_AddStringToObject(json, "device_class", "illuminance");
    cJSON_AddBoolToObject(json, "enabled_by_default", 1);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "json_attributes_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s illuminance lux", ieeeaddr_str);
    cJSON_AddStringToObject(json, "name", topic);

    cJSON_AddStringToObject(json, "state_class", "measurement");

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "zigbee2mqtt/%s", ieeeaddr_str);
    cJSON_AddStringToObject(json, "state_topic", topic);

    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, "%s_%s_zigbee2mqtt", ieeeaddr_str, "illuminance_lux");
    cJSON_AddStringToObject(json, "unique_id", topic);

    cJSON_AddStringToObject(json, "unit_of_measurement", "lx");
    cJSON_AddStringToObject(json, "value_template", "{{ value_json.illuminance }}");

    char *str = cJSON_Print(json);
    memset(topic, 0, sizeof(topic));
    snprintf(topic, sizeof(topic) - 1, 
        "homeassistant/sensor/%s/illuminance_lux/config",
        ieeeaddr_str
    );

    // push message
    // appMQTTPublish(topic, str);
    mqtt.publish(topic, str);
}


static void handleLilygoLight(const char * topic, const char *data) {
    if (!topic || !data) return ;
    uint64_t u64IeeeAddr = 0;
    ts_DstAddr  sDstAddr;

    sscanf(topic, "homeassistant/light/0x%016llx/set", &u64IeeeAddr);
    printf("u64IeeeAddr: 0x%016llx\n", u64IeeeAddr);

    DeviceNode *device = findDeviceByIeeeaddr(u64IeeeAddr);
    if (!device) {
        printf("on device\n");
        return;
    }
    sDstAddr.u16DstAddr = device->u16NwkAddr;

    cJSON *json = cJSON_Parse(data);
    if (!json) {
        printf("json error\n");
        return ;
    }
    cJSON *state = cJSON_GetObjectItem(json, "state");
    if (!state) {
        cJSON_Delete(json);
        printf("json error\n");
        return ;
    }
    if (!memcmp(state->valuestring, "ON", strlen("ON"))) {
        zbhci_ZclOnoffOn(0x02, sDstAddr, 1, 1);
    } else {
        zbhci_ZclOnoffOff(0x02, sDstAddr, 1, 1);
    }

    // zbhci_ZclAttrWrite(0x02, sDstAddr, 1, 1, 0, 0x0006, 1, &sAttrList);
    cJSON_Delete(json);
    delay(100);
}

static void handleTuyaPlug(const char * topic, const char *data)
{
    printf("handleTuyaPlug: received mqtt command\n");

    if (!topic || !data) return ;
    uint64_t u64IeeeAddr = 0;
    ts_DstAddr  sDstAddr;

    sscanf(topic, "zigbee2mqtt/0x%016llx/set", &u64IeeeAddr);
    printf("u64IeeeAddr: 0x%016llx\n", u64IeeeAddr);

    DeviceNode *device = findDeviceByIeeeaddr(u64IeeeAddr);
    if (!device)
    {
        printf("on device\n");
    }
    sDstAddr.u16DstAddr = device->u16NwkAddr;

    cJSON *json = cJSON_Parse(data);
    if (!json)
    {
        printf("json error\n");
        return ;
    }
    cJSON *state = cJSON_GetObjectItem(json, "state");
    if (!state)
    {
        cJSON_Delete(json);
        printf("json error\n");
        return ;
    }
    if (!memcmp(state->valuestring, "ON", strlen("ON")))
    {
        zbhci_ZclOnoffOn(0x02, sDstAddr, 1, 1);
    }
    else
    {
        zbhci_ZclOnoffOff(0x02, sDstAddr, 1, 1);
    }

    // zbhci_ZclAttrWrite(0x02, sDstAddr, 1, 1, 0, 0x0006, 1, &sAttrList);
    cJSON_Delete(json);
    delay(100);
}

static void handleTradfriPlug(const char * topic, const char *data)
{
    printf("handleTradfriPlug: received mqtt command\n");

    if (!topic || !data) return ;
    uint64_t u64IeeeAddr = 0;
    ts_DstAddr  sDstAddr;

    sscanf(topic, "zigbee2mqtt/0x%016llx/set", &u64IeeeAddr);
    printf("u64IeeeAddr: 0x%016llx\n", u64IeeeAddr);

    DeviceNode *device = findDeviceByIeeeaddr(u64IeeeAddr);
    if (!device)
    {
        printf("on device\n");
    }
    sDstAddr.u16DstAddr = device->u16NwkAddr;

    cJSON *json = cJSON_Parse(data);
    if (!json)
    {
        printf("json error\n");
        return ;
    }
    cJSON *state = cJSON_GetObjectItem(json, "state");
    if (!state)
    {
        cJSON_Delete(json);
        printf("json error\n");
        return ;
    }
    if (!memcmp(state->valuestring, "ON", strlen("ON")))
    {
        zbhci_ZclOnoffOn(0x02, sDstAddr, 1, 1);
    }
    else
    {
        zbhci_ZclOnoffOff(0x02, sDstAddr, 1, 1);
    }

    // zbhci_ZclAttrWrite(0x02, sDstAddr, 1, 1, 0, 0x0006, 1, &sAttrList);
    cJSON_Delete(json);
    delay(100);
}

static void handleMQTTAttrGet(const char * topic, const char *data) {
    printf("Received attr_get mqtt command\n");
    if (!topic || !data) return ;
    //uint64_t u64IeeeAddr = 0xa4c138c2ec163bd8;
    uint64_t u64IeeeAddr = 0xa4c1387ca2489e61;

    ts_DstAddr  sDstAddr;
    uint16_t uAttrList[1];

    DeviceNode *device = findDeviceByIeeeaddr(u64IeeeAddr);
    if (!device)
    {
        printf("on device\n");
    }
    sDstAddr.u16DstAddr = device->u16NwkAddr;

    cJSON *json = cJSON_Parse(data);
    if (!json)
    {
        printf("json error\n");
        return ;
    }
    cJSON *cluster = cJSON_GetObjectItem(json, "cluster");
    if (!cluster)
    {
        cJSON_Delete(json);
        printf("json error\n");
        return ;
    }
    cJSON *attr = cJSON_GetObjectItem(json, "attr");
    if (!attr)
    {
        cJSON_Delete(json);
        printf("json error\n");
        return ;
    }
    uAttrList[0] = attr->valueint;
    zbhci_ZclAttrRead(0x2, sDstAddr, 1, 1, 0, cluster->valueint, 1, uAttrList);
    //zbhci_ZclReadReportCfg(0x02, sDstAddr, 1, 1, 0, cluster->valueint, 1, uAttrList);
}

/******************************************************************************/
/***        END OF FILE                                                     ***/
/******************************************************************************/
