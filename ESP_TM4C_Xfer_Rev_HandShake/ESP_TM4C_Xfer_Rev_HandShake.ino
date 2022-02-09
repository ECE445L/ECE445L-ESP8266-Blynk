// **************************************************************************
//
//      THIS CODE RUNS ON THE ESP8266 NOT ON THE TM4C!!
//
// **************************************************************************
//
//      This revision has the hardware handshake signals to interface 
//      to the Mega and TM4C123
//
// **************************************************************************

// Comment out the next line to disable verbose messages
// which will mess up the interface to the TM4C. Use only
// for debug on the 8266 programmer

//#define BLYNK_PRINT Serial

// ----------------------------------------------------------------------------
// DEFINEs section

#define DEBUG1 0                // First level of Debug
#define DEBUG2 0                // Second level of Debug
#define DEBUG3 0                // Third level of Debug
//#undef DEBUG1
//#undef DEBUG2
#undef DEBUG3

#define GPIO_2 2                // RDY signal to the TM4C
#define RDY 2

//#define GPIO_0 0                // RDY signal from the TM4C
//#define ACK 0

// ----------------------------------------------------------------------------
// INCLUDEs section

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <stdio.h>
#include <string.h>

// ----------------------------------------------------------------------------
// Variable assignments

int     Pin_Number  = 255;      // 255 is an invalid pin number
int     Pin_Integer = 0;
float   Pin_Float   = 1.234;
String  Pin_String  = "";       // Empty string

//char  ssid[256]  = "Baldy";


//char    ssid[256]  = "Beaubien";
//char    pass[256]  = "bullybully";

char    ssid[256]  = "utexas-iot";
char    pass[256] =  "4047807372171447";

//char    auth[256]  = "3267e5ebd9a14724862a96767a5f73b1";
char    auth[256]  = "47a158de021146b68294d1fbb664c5d1";


char    http[1024] = "blynk-cloud.com";
char    port[16]   = "8442";

//byte arduino_mac[] = { 0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
byte mac[6];                     // the MAC address of your Wifi shield
// ----------------------------------------------------------------------------
// CODE section starts here

BlynkTimer timer;         // Start the timer

// ----------------------------------------------------------------------------
// This routine is called once a second to determine if the WiFi needs to be
// disconnected
/*
void Disconnect_WiFi(void) {

  if (digitalRead(GPIO_0) == 0) { 
      WiFi.disconnect(); 
      #ifdef DEBUG1
      Serial.print("Disconnecting from WiFi");
      Serial.println();
      #endif
  }
}
*/



// ----------------------------------------------------------------------------
// This routine sets up Wifi. First step is receive the SSID, Password and
// Authorization code using CSV format. Second step is to parse it and try
// to connect to the WiFi hotspot. Once the connection is made we then try to
// connect to Blynk server.
//

void Setup_Wifi(void) {
  
  char ser_buf[1024];
  static int bufpos = 0;              // starts the buffer back at the first position in the incoming serial.read

// Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();                  // Disconnect the Wifi before re-enabling it

  delay(3000);                        // Wait for system to stabilize
  
  digitalWrite(RDY, HIGH);            // Set RDY to TM4C

  //Serial.print("########");           // Indication to the Mega that Serial setup is ready
  //Serial.println();                   // Mega waits for \n to continue


  //while ((digitalRead(ACK) != 1) or (Serial.available() == 0)) {
  while ((Serial.available() == 0)) {  // Wait for TM4C to start sending data
    }
  
  while (Serial.available() > 0)   {

      char inchar = Serial.read();  // assigns one byte (as serial.read()'s only input one byte at a time

      if (inchar != '\n') {         // if the incoming character is not a newline then process byte
        ser_buf[bufpos] = inchar;   // the buffer position in the array get assigned to the current read
        bufpos++;                   // once that has happend the buffer advances, doing this over and over again until the end of package marker is read.
        delay(10);
      }
  } 

  // ----------------------------------------------------------------------------
  // This debug block double checks that the # of received 
  // characters matches the transmitted number
     
  #ifdef DEBUG3
  Serial.print("bufpos: ");
  Serial.println(bufpos);
  #endif

  // ----------------------------------------------------------------------------
  // Rip the 3 fields out of the CSV data. The sequence of data from the TM4C is:
  // authorization_code, ssid, password, 
  // Note: the string is terminate with a dangling comma
  //
  if (bufpos  > 0) {
    strcpy(auth, strtok(ser_buf, ","));
    strcpy(ssid, strtok(NULL, ","));  
    strcpy(pass, strtok(NULL, ","));
  }

  #ifdef DEBUG2
/*
  Serial.print("Size of ssid: ");
  Serial.println(strlen(ssid));

  Serial.print("Size of pass: ");
  Serial.println(strlen(pass));

  Serial.print("Size of auth: ");
  Serial.println(strlen(auth));
*/
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

  //Serial.print("Web page: ");
  //Serial.print(http);
  //Serial.println();
  #endif   
  
  Serial.flush();                   // Flush the serial queue
  // ----------------------------------------------------------------------------
  // Wait for the WiFi to connect. There is no timeout mechanism
  // so the timeout is up the Mega. The Mega would count the number
  // of '.' to determine when to reset the WiFi module
  //
  // Blynk.begin suppport 3 ways of specifying the website to connect to
  //      Blynk.begin(auth, ssid, pass);
  //      Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //      Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);
  
  WiFi.begin(ssid, pass);  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
 
  Serial.flush();                   // Flush the serial queue

  #ifdef DEBUG2
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  Serial.println("Waiting for connection the Blynk server");  
  Serial.println("");
  
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
  Serial.println(mac[0],HEX);
  
  #endif
  
  Serial.flush();                     // Flush the serial queue
  
  // ----------------------------------------------------------------------------------
  // This section connects to the Blynk server and sets up the connection to the phone app
  //
  //Blynk.config(auth);                 // In place of Blynk.begin(auth, ssid, pass);
  
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  
  Blynk.connect(33);                  // Timeout set to 10 seconds
  while (Blynk.connect() == false)    // Wait until connected or timeout occurs
  {   //delay(10); 
      Serial.print(".");
  }

  /*
  while ((digitalRead(0) != 0) or (Serial.available() == 0))  {       // Wait for ACK from TM4C before exiting setup
  }
*/
  
  #ifdef DEBUG1

  Serial.println("Connected to Blynk server");
  Serial.println("Leaving Setup_Wifi");
  #endif
  
  Serial.flush();                       // Flush the serial queue
  
  delay(50);                            // Wait before negating RDY signal
  digitalWrite(RDY, LOW);               // Negate RDY to TM4C
  
}  // End of Setup_Wifi(void)

// --------------------------------------------------------
// This is the MACRO used to send control signals to the TM4C
// 

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


// ----------------------------------------------------------------------------
// This routine receives data from the Mega and parses it.
// The parsed data is then sent to the Blynk Server which 
// sends it to the Blynk App. 
// NOTE: Virtual pins from 50-89 are used to send data back
// to the Blynk app

void SerialInput_to_BlynkApp(void) { 
  while (Serial.available() > 0) {

    // look for the next valid integer in the incoming serial stream:

    int Pin_Number  = Serial.parseInt();
    int Pin_Integer = Serial.parseInt();
    float Pin_Float = Serial.parseFloat();
 
    // Look for the newline as the EOL indication
    if (Serial.read() == '\n') {

      #ifdef DEBUG3

      Serial.print("V");        
      Serial.print(Pin_Number);
      Serial.print(",");
      Serial.print(Pin_Integer);
      Serial.print(",");
      Serial.print(Pin_Float);
      Serial.print("\n");
      # endif

      // This section of code determines if integer or floating point
      // values are returned to the Blynk app. Virtual pins between 
      // 50 and 69 return floating point. Virtual pins between 70 and 
      // 89 return integers
      
      
      if((Pin_Number >49) && (Pin_Number < 70)) {
        Blynk.virtualWrite(Pin_Number, Pin_Float );
      }
      else if((Pin_Number >69) && (Pin_Number < 100)) {
        Blynk.virtualWrite(Pin_Number, Pin_Integer );
      }
      else {
        Blynk.virtualWrite(Pin_Number, "Help");
      }
    }
  }
}


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// 
// THIS IS THE MAIN SETUP CODE. IT IS ONLY RUN ONCE
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void setup() {


Serial.begin(9600);               // Set up debug serial port
Serial.flush();                   // Flush the serial port

pinMode(0, INPUT);               // Set GPIO_0 to an input  - ACK signal from the TM4C
pinMode(2, OUTPUT);                // Set GPIO_2 to an output - RDY signal to the TM4C

digitalWrite(RDY, LOW);           // Set the RDY pin LOW

Setup_Wifi();                     // This routine reads in the new SSID, Password and Authorization code
  
timer.setInterval(1000L, SerialInput_to_BlynkApp);      // This is a scheduler to read serial data
  
}  // End of setup()


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// 
// THIS IS THE MAIN LOOP. IT RUNS CONTINOUSLY
//
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void loop()
{


  Blynk.run();
  timer.run();
  
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
  
}   // End of loop()






// ----------------------------------------------------------------------------
// This routine is used to debug the interface between the TM4c

void Test8266_IF(void) {

  char ser_buf[1024];
  static int bufpos = 0;              // starts the buffer back at the first position in the incoming serial.read

  
  
  digitalWrite(RDY, HIGH);            // Set RDY to TM4C
  delay(100);                          // Wait for system to stabilize
    
  while (Serial.available() == 0)
    {      delay(0);         // Wait for TM4C to start sending data
    }

  while (Serial.available() > 0)   {
    char inchar = Serial.read();      // assigns one byte (as serial.read()'s only input one byte at a time

    if (inchar != '\n') {             // if the incoming character is not a newline then process byte
      ser_buf[bufpos] = inchar;       // the buffer position in the array get assigned to the current read
      bufpos++;                       // once that has happend the buffer advances, doing this over and over again until the end of package marker is read.
      delay(10);
      }
  } 

  if (bufpos  > 0) {
    strcpy(auth, strtok(ser_buf, ","));
    strcpy(ssid, strtok(NULL, ","));  
    strcpy(pass, strtok(NULL, ","));
  }
/*
  Serial.print("Size of ssid: ");
  Serial.println(strlen(ssid));

  Serial.print("Size of pass: ");
  Serial.println(strlen(pass));

  Serial.print("Size of auth: ");
  Serial.println(strlen(auth));
*/
  Serial.print("SSID: ");
  Serial.print(ssid);
  Serial.println();

  Serial.print("Password: ");
  Serial.print(pass);
  Serial.println();

  Serial.print("Auth: ");
  Serial.print(auth);
  Serial.println();

  Serial.print("Web page: ");
  Serial.print(http);
  Serial.println();
    
  //while ((digitalRead(ACK) != 0))  
  //{       // Wait for ACK from TM4C before exiting setup    }
  
  Serial.println("Leaving 8266 Setup_Wifi Routine");
  
  Serial.flush();

  digitalWrite(RDY, LOW);            // Negate RDY to TM4C
  
}  // END of Test8266

