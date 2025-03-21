         /////////////////////////////////////////////  
        //    WhatsApp Halloween-Themed RFID       //
       //      Talking Doorbell w/ RGB Eyes       //
      //        -------------------------        //
     //           (Arduino Nano 33 IoT)         //           
    //             by Kutluhan Aktar           // 
   //                                         //
  /////////////////////////////////////////////

// Startle your guests who do not have an entrance permit (RFID tag) and get notified w/ WhatsApp messages without checking the door :)
//
// For more information:
// https://www.theamplituhedron.com/projects/WhatsApp-Halloween-Themed-RFID-Talking-Doorbell-with-RGB-Eyes/
//
// Connections
// Arduino Nano 33 IoT:           
//                            MFRC522
// D9  ----------------------- RST
// D10 ----------------------- SDA
// D11 ----------------------- MOSI
// D12 ----------------------- MISO
// D13 ----------------------- SCK
//                            DFPlayer Mini
//     ----------------------- VCC (5V External)
// TX  ----------------------- RX
// RX  ----------------------- TX
// GND ----------------------- GND
//                            RGB_Eye_1
// D3  ----------------------- R
// D5  ----------------------- G
// D6  ----------------------- B
//                            RGB_Eye_2
// A2  ----------------------- R
// A3  ----------------------- G
// A5  ----------------------- B
//                            Doorbell (Button)
// D2  ----------------------- S

/*

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/

#include <SPI.h>      // RC522 Module uses SPI protocol
#include <WiFiNINA.h>
#include <MFRC522.h>  // Library for Mifare RC522 Devices
#include "DFRobotDFPlayerMini.h"

char ssid[] = "[_SSID_]";        // your network SSID (name)
char pass[] = "[_PASSWORD_]";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Enter the IPAddress of your Raspberry Pi.
IPAddress server(192, 168, 1, 20);

// Define the pathway of the application in the Raspberry Pi.
String application = "/WhatsApp_Talking_Doorbell/";

// Initialize the Ethernet client library
WiFiClient client;

// Create DFPlayer Mini object.
DFRobotDFPlayerMini myDFPlayer;

// Create the MFRC522 instance.
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Define the MFRC522 module key input.
MFRC522::MIFARE_Key key;

// Define the data holders:
String lastRead; // recently read UID
int red, green, blue;
int bell = 0; // ring

// After executing the register_new_UIDs() function, paste the registered UID list here and define the total guest number:
char *invited_guests[] = {"9B B6 3C 75", "56 1B 0D F8"};
#define guest_number 2

// Define RGB pins.
#define redPin_1 3
#define greenPin_1 5
#define bluePin_1 6
#define redPin_2 A2
#define greenPin_2 A3
#define bluePin_2 A5

// Define the doorbell pin.
#define doorbell 2

void setup() {
  Serial.begin(9600); 
  
  pinMode(redPin_1, OUTPUT);
  pinMode(greenPin_1, OUTPUT);
  pinMode(bluePin_1, OUTPUT);
  pinMode(redPin_2, OUTPUT);
  pinMode(greenPin_2, OUTPUT);
  pinMode(bluePin_2, OUTPUT);
  pinMode(doorbell, INPUT);
  
  // Initiate DFPlayer Mini on the second serial port on RX0 and TX1 pins.
  Serial1.begin(9600);
  while(!myDFPlayer.begin(Serial1)){ Serial.println("Not Connected!"); }
  Serial.println("DFPlayer Connected!!!");
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).
  //myDFPlayer.volumeUp(); //Volume Up
  //myDFPlayer.volumeDown(); //Volume Down
  // Set EQ
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  // Set the SD Card as default source.
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  // Initialize MFRC522 Hardware
  SPI.begin();          
  mfrc522.PCD_Init();
  Serial.println("\n----------------------------------\nApproximate a New Card or Key Tag : \n");

  // Check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) { Serial.println("Communication with WiFi module failed!"); while (true); }
  // Attempt to connect to the WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // Wait 10 seconds for connection:
    delay(10000);
  }

  // Verify connection on the serial monitor.
  Serial.println("Connected to wifi");

  // Adjust the default color of RGB eyes.
  adjustColor('f', 255, 0, 255);
  adjustColor('s', 255, 0, 255);
  
}

void loop(){  
  // Startle and inform guests when they press the button - doorbell.
  if(digitalRead(doorbell) == HIGH){
    bell++;
    if(bell == 1){
      myDFPlayer.play(1);
      adjustColor('f', 255, 255, 0);
      adjustColor('s', 255, 255, 0);
      WhatsApp_Message("There%20is%20someone%20at%20the%20door!%20Waiting%20for%20action...%0a%0aStatus%20=>%20New%20Guest");
    }else if(bell == 2){
      myDFPlayer.play(2);
      adjustColor('f', 255, 0, 0);
      adjustColor('s', 255, 0, 0);
      WhatsApp_Message("The%20guest%20has%20not%20been%20apprised%20of%20the%20Halloween-themed%20RFID%20entrance%20system%20:)%0a%0aStatus%20=>%20Second%20Attempt");
    }else{
      bell = 0;
    }
    delay(2000);
  }

  // Uncomment to get and register new UIDs for comparison.
  //register_new_UIDs();

  read_UID();
}

int read_UID() {
  // Detect the new card or tag UID. 
  if ( ! mfrc522.PICC_IsNewCardPresent()) { 
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }

  // Display the new UID.
  Serial.print("\n----------------------------------\nNew Card or Key Tag UID : ");
  for (int i = 0; i < mfrc522.uid.size; i++) {
    lastRead += mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ";
    lastRead += String(mfrc522.uid.uidByte[i], HEX);
  }
  lastRead.trim();
  lastRead.toUpperCase();
  Serial.print(lastRead);
  Serial.print("\n----------------------------------\n");

  // Detect whether the recently read UID (lastRead) is in the registered UIDs in the invited_guests string array to activate the DFPlayer with the related voice file.
  volatile boolean UID_Found = false;
  for(int i=0;i<guest_number;i++){
    if(lastRead == invited_guests[i]){
      UID_Found = true;
    }
  }
  if(UID_Found == true){
    myDFPlayer.play(4);
    adjustColor('f', 0, 255, 0);
    adjustColor('s', 0, 255, 0);
    WhatsApp_Message("One%20of%20the%20guests%20relinquished%20an%20entrance%20pass%20-%20registered%20UID%20-%20has%20been%20arrived%20:)%0a%0aStatus%20=>%20Accurate");   
  }else if(UID_Found == false){
    myDFPlayer.play(3);
    adjustColor('f', 0, 255, 255);
    adjustColor('s', 0, 255, 255);
    WhatsApp_Message("Some%20uninvited%20guest%20is%20trying%20to%20breach%20the%20entrance%20system!%0a%0aStatus%20=>%20Trespassing");    
  }

  // Reset the doorbell:
  bell = 0;

  lastRead = "";
  mfrc522.PICC_HaltA();
  return 1;
}

int register_new_UIDs() {
  // Detect the new card or tag UID. 
  if ( ! mfrc522.PICC_IsNewCardPresent()) { 
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }

  // Get the recently read UID.
  for (int i = 0; i < mfrc522.uid.size; i++) {  //
    lastRead += mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ";
    lastRead += String(mfrc522.uid.uidByte[i], HEX);
  }
  lastRead.trim();
  lastRead.toUpperCase();
  // To save the new UIDs to the invited_guests string array, copy the UID list after executing this function.
  Serial.print("\"" + lastRead + "\", ");
  lastRead = "";
  mfrc522.PICC_HaltA();
  return 1;
}

void WhatsApp_Message(String body){
  // Define the required settings by Twilio - Account SID, Auth Token, FROM phone number (incoming), and TO phone number (outgoing).
  String sid = "[_SID_]";
  String a_token = "[_AUTH_TOKEN_]";
  String FROM = "14155238886";
  String TO = "[_REGISTERED_PHONE_NUMBER_]";
  
  // Connect to the web application named WhatsApp Talking Doorbell on the Raspberry Pi.
  if (client.connect(server, 80)) {
    Serial.println("Connected to the server!"); // if you get a connection, report back via serial:
    // Make an HTTP request:
    client.println("GET " + application + "?sid=" + sid + "&a_token=" + a_token + "&body=" + body + "&from=" + FROM + "&to=" + TO + " HTTP/1.1");
    client.println("Host: 192.168.1.20");
    client.println("Connection: close");
    client.println();
  }else{
    Serial.println("Server Connection Failed!");
  }
  delay(2000); // Wait 2 seconds after connection...  
}

void adjustColor(char eye, int r, int g, int b){
  switch (eye){
    case 'f':
      analogWrite(redPin_1, r);
      analogWrite(greenPin_1, g);
      analogWrite(bluePin_1, b);
    break;
    case 's':
      analogWrite(redPin_2, r);
      analogWrite(greenPin_2, g);
      analogWrite(bluePin_2, b);
    break;
  }
}
