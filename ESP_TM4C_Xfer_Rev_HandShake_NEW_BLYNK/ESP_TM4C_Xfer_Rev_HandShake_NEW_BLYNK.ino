/**
 * @file ESP_TM4C_Xfer_Rev_HandShake_NEW_BLYNK.ino
 * @author Matthew Yu (matthewjkyu@gmail.com), Mark McDermott, Jonathan Valvano
 * @brief Arduino sketch that runs on the ESP8266 to facilitate TM4C - Blynk operation.
 * @version 0.1
 * @date 2022-02-09
 * @note This program is for the new version of Blynk.
 *
 * @copyright Copyright (c) 2022
 *
 */

/** Defines. */

/* Template ID, device name, and auth token are provided by Blynk. */
#define BLYNK_TEMPLATE_ID "TMPL0hqIUfLL"
#define BLYNK_DEVICE_NAME "EE445L Lab4D"
#define BLYNK_AUTH_TOKEN "UcSNXiCuOccQ3A34T1kEw5YYLcCjxgb-";

/* For serial debug output. */
#define DEBUG1 0

#define GPIO_2 2                // RDY signal to the TM4C
#define RDY 2

//#define GPIO_0 0                // RDY signal from the TM4C
//#define ACK 0

/** Includes. */
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <stdio.h>
#include <string.h>

/** Globals. */
int     Pin_Number  = 255;      // 255 is an invalid pin number
int     Pin_Integer = 0;
float   Pin_Float   = 1.234;
String  Pin_String  = "";       // Empty string


char    ssid[256]   = "Beaubien";
char    pass[256]   = "bullybully";

//char    ssid[256]  = "utexas-iot";
//char    pass[256] =  "4047807372171447";

char    auth[]      = BLYNK_AUTH_TOKEN;
byte    mac[6];                 // MAC address of the 8266.

BlynkTimer timer;         // Start the timer

/** Function Definitions. */

/**
 * @brief SetupWifi receives the SSID, password, and auth code, parses them,
 *        and tries to connect to the WiFi hotspot and Blynk Server.
 */
void SetupWifi(void);

/**
 * @brief SerialInputToBlynkApp receives data from the TM4C and parses it. Data
 *        is then sent to the Blynk server and updated in the app.
 * @note Virtual pins from 50-89 are used to send data back.
 */
void SerialInputToBlynkApp(void);

/**
 * @brief Test8266_IF is used to debug the interface between the TM4C.
 */
void Test8266_IF(void);

/**
 * @brief BLYNK_WRITE is the macro used to send control signals to the TM4C.
 */
#define VirtualPin_Handler(pin_num) \
BLYNK_WRITE(pin_num)                \
{                                   \
  Pin_Number = pin_num;             \
  Pin_String  = param.asStr();      \
  Pin_Integer = param.asInt();      \
  Pin_Float   = param.asDouble();   \
  Serial.print(Pin_Number);         \
  Serial.print(",");                \
  Serial.print(Pin_Integer);        \
  Serial.print(",");                \
  Serial.print(Pin_Float);          \
  Serial.print("\n");               \
}

// ----------------------------------------------------------
// This section calls the Virtual Pin Handler (above) to
// determine which virtual pins on the Blynk App have been
// pressed. That information is then sent to the Mega.
//
// NOTE: Only 16 Virtual Pins are called. Add additional pins
// as needed.

VirtualPin_Handler(V0)
VirtualPin_Handler(V1)
VirtualPin_Handler(V2)
VirtualPin_Handler(V3)
VirtualPin_Handler(V4)
VirtualPin_Handler(V5)
VirtualPin_Handler(V6)
VirtualPin_Handler(V7)
VirtualPin_Handler(V8)
VirtualPin_Handler(V9)
VirtualPin_Handler(V10)
VirtualPin_Handler(V11)
VirtualPin_Handler(V12)
VirtualPin_Handler(V13)
VirtualPin_Handler(V14)
VirtualPin_Handler(V15)

void setup() {
    Serial.begin(9600);         // Set up debug serial port
    Serial.flush();             // Flush the serial port

    pinMode(0, INPUT);          // Set GPIO_0 to an input  - ACK signal from the TM4C
    pinMode(2, OUTPUT);         // Set GPIO_2 to an output - RDY signal to the TM4C

    digitalWrite(RDY, LOW);     // Set the RDY pin LOW

    SetupWifi();               // This routine reads in the new SSID, Password and Authorization code

    timer.setInterval(1000L, SerialInputToBlynkApp);  // This is a scheduler to read serial data
}

void loop() {
    Blynk.run();
    timer.run();
}

/**
 * @brief SetupWifi receives the SSID, password, and auth code, parses them,
 *        and tries to connect to the WiFi hotspot and Blynk Server.
 */
void SetupWifi(void) {
    char serBuf[1024];
    static int bufpos = 0;

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(3000);

    digitalWrite(RDY, HIGH);    // Set RDY to TM4C
    delay(100);                 // Wait for system to stabilize

    // Wait for TM4C to start sending data.
    while (Serial.available() == 0) { delay(0); }

    // Capture data from the TM4C in bytes. Ignores newlines.
    while (Serial.available() > 0) {
        char inchar = Serial.read();
        if (inchar != '\n') {
            serBuf[bufpos] = inchar;
            ++bufpos;
            delay(10);
        }
    }

    // Copy over authentication token, ssid, password.
    // Note: the string is terminated with a dangling comma.
    if (bufpos  > 0) {
        strcpy(auth, strtok(serBuf, ","));
        strcpy(ssid, strtok(NULL, ","));
        strcpy(pass, strtok(NULL, ","));
    }

    Serial.flush();

    #ifdef DEBUG
    Serial.println();
    Serial.print("SSID: ");
    Serial.print(ssid);
    Serial.println();
    Serial.print("Password: ");
    Serial.print(pass);
    Serial.println();
    Serial.print("Auth: ");
    Serial.print(auth);
    Serial.println();
    Serial.flush();
    #endif

    Blynk.begin(auth, ssid, pass);

    #ifdef DEBUG
    Serial.println();
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[5],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.print(mac[0],HEX);
    Serial.println();
    Serial.println("Connected to Blynk server.");
    Serial.println("Leaving SetupWifi.");
    Serial.flush();
    #endif

    delay(50);                            // Wait before negating RDY signal
    digitalWrite(RDY, LOW);               // Negate RDY to TM4C
}

/**
 * @brief SerialInputToBlynkApp receives data from the TM4C and parses it. Data
 *        is then sent to the Blynk server and updated in the app.
 * @note Virtual pins from 50-89 are used to send data back.
 */
void SerialInputToBlynkApp(void) {
    while (Serial.available() > 0) {
        // look for the next valid integer in the incoming serial stream:
        int pinNumber  = Serial.parseInt();
        int pinInteger = Serial.parseInt();
        float pinFloat = Serial.parseFloat();

        // Look for the newline as the EOL indication
        if (Serial.read() == '\n') {
            #ifdef DEBUG3
            Serial.print("V");
            Serial.print(pinNumber);
            Serial.print(",");
            Serial.print(pinInteger);
            Serial.print(",");
            Serial.print(pinFloat);
            Serial.print("\n");
            # endif

            // This section of code determines if integer or floating point
            // values are returned to the Blynk app. Virtual pins between
            // 50 and 69 return floating point. Virtual pins between 70 and
            // 89 return integers
            if ((pinNumber > 49) && (pinNumber < 70)) {
                Blynk.virtualWrite(pinNumber, pinFloat);
            } else if ((pinNumber > 69) && (pinNumber < 100)) {
                Blynk.virtualWrite(pinNumber, pinInteger);
            } else {
                Blynk.virtualWrite(pinNumber, "Help");
            }
        }
    }
}

/**
 * @brief Test8266_IF is used to debug the interface between the TM4C.
 */
void Test8266_IF(void) {
    char serBuf[1024];
    static int bufpos = 0;      // starts the buffer back at the first position in the incoming serial.read

    digitalWrite(RDY, HIGH);    // Set RDY to TM4C
    delay(100);                 // Wait for system to stabilize

    // Wait for TM4C to start sending data.
    while (Serial.available() == 0) { delay(0); }

    // Capture data from the TM4C in bytes. Ignores newlines.
    while (Serial.available() > 0) {
        char inchar = Serial.read();
        if (inchar != '\n') {
            serBuf[bufpos] = inchar;
            ++bufpos;
            delay(10);
        }
    }

    // Copy over authentication token, ssid, password.
    if (bufpos  > 0) {
        strcpy(auth, strtok(serBuf, ","));
        strcpy(ssid, strtok(NULL, ","));
        strcpy(pass, strtok(NULL, ","));
    }

    Serial.print("SSID: ");
    Serial.print(ssid);
    Serial.println();
    Serial.print("Password: ");
    Serial.print(pass);
    Serial.println();
    Serial.print("Auth: ");
    Serial.print(auth);
    Serial.println();
    Serial.println("Leaving 8266 Test Routine");
    Serial.flush();

    digitalWrite(RDY, LOW);            // Negate RDY to TM4C
}
