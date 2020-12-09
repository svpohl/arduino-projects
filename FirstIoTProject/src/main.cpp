#include <time.h>
#include <ESP8266WiFi.h>
#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <iothubtransportmqtt.h>
#include <AzureIoTUtility.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

// Times before 2010 (1970 + 40 years) are invalid
#define MIN_EPOCH 40 * 365 * 24 * 3600

#define IOT_CONFIG_WIFI_SSID "wlan.team-pohl.rocks"
#define IOT_CONFIG_WIFI_PASSWORD "Y3M9GKiDx9RLTnx3kqN5"
#define IOT_CONFIG_CONNECTION_STRING "HostName=arduiHub.azure-devices.net;DeviceId=lolinD1pro;SharedAccessKey=57zT6nftMC5K1t/DPm/1TkWRFJYklC/u2gHTvcQ7qeo="

char msg[100];

IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle = nullptr;

Adafruit_BMP085 bmp;

void initWifi()
{
    // Attempt to connect to Wifi network:
    Serial.print("\r\n\r\nAttempting to connect to SSID: ");
    Serial.println(IOT_CONFIG_WIFI_SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(IOT_CONFIG_WIFI_SSID, IOT_CONFIG_WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\r\nConnected to wifi");
}

static void initTime()
{
    time_t epochTime;

    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime < MIN_EPOCH)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.print("Fetched NTP epoch time is: ");
            Serial.println(epochTime);
            break;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    initWifi();
    initTime();

    // Init IoT client
    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(IOT_CONFIG_CONNECTION_STRING, MQTT_Protocol);
    if (iotHubClientHandle == NULL)
    {
        Serial.println("Failed on IoTHubClient_LL_Create");
    }

    if (!bmp.begin())
    {
        Serial.println("Could not find a valid BMP085 sensor, check wiring!");
        while (1)
        {
        }
    }
}

void loop()
{
    sprintf(msg, "{ 'pressure':%.2u,'temperature':%.2f, altitude:%.2f }", bmp.readPressure(), bmp.readTemperature(), bmp.readAltitude());
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)msg, strlen(msg));
    if (messageHandle == NULL)
    {
        Serial.println("unable to create a new IoTHubMessage");
    }
    else
    {
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, nullptr, nullptr) != IOTHUB_CLIENT_OK)
        {
            Serial.println("failed to hand over the message to IoTHubClient");
        }
        else
        {
            Serial.println("IoTHubClient accepted the message for delivery");
        }
        IoTHubMessage_Destroy(messageHandle);
    }

    IOTHUB_CLIENT_STATUS status;
    while ((IoTHubClient_LL_GetSendStatus(iotHubClientHandle, &status) == IOTHUB_CLIENT_OK) && (status == IOTHUB_CLIENT_SEND_STATUS_BUSY))
    {
        IoTHubClient_LL_DoWork(iotHubClientHandle);
        ThreadAPI_Sleep(100);
    }
    Serial.println(msg);
    Serial.println("wait...");
    delay(5000);
}