#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <map>


bool DEBUG = true;
uint64_t duration = 3000;
std::map<int, String> tagsList;



//an array of commands that is given to control the module
const uint8_t RFID_cmdnub[45][40] =
{
  {0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E,},       //0. Hardware version 
  {0xBB, 0x00, 0x03, 0x00, 0x01, 0x01, 0x05, 0x7E,},       //1. Software version 
  {0xBB, 0x00, 0x03, 0x00, 0x01, 0x02, 0x06, 0x7E,},       //2. manufacturers  
  {0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E,},             //3. Single polling instruction 
  {0xBB, 0x00, 0x27, 0x00, 0x03, 0x22, 0x00, 0x0A, 0x56, 0x7E,}, //4. Multiple polling instructions 
  {0xBB, 0x00, 0x28, 0x00, 0x00, 0x28, 0x7E,},             //5. Stop multiple polling instructions 
  { 0xBB, 0x00, 0x0C, 0x00, 0x13, 0x09, 0x00, 0x00, 0x00, 0x20,
    0x60, 0x00, 0x30, 0x08, 0x33, 0xB2, 0xDD, 0xD9, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0xBC, 0x7E,
  },                        //6. Set the SELECT parameter instruction --> target == 000 == all tags, action == 010 == all tags regardless state, membank == 01 == EPC
  {0xBB, 0x00, 0x0B, 0x00, 0x00, 0x0B, 0x7E,},              //7. Get the SELECT parameter 
  {0xBB, 0x00, 0x12, 0x00, 0x01, 0x01, 0x14, 0x7E,},        //8. Set the SELECT mode 
  { 0xBB, 0x00, 0x39, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00, 0x03, 0x00, 0x03, 0x4A, 0x7E,
  },                      //9. Read label data storage area 
  { 0xBB, 0x00, 0x49, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x02, 0x00, 0x06, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xE7, 0x7E,
  },   //10. Write the label data store 
  { 0xBB, 0x00, 0x82, 0x00, 0x07, 0x00, 0x00, 0xFF,
    0xFF, 0x02, 0x00, 0x80, 0x09, 0x7E,
  },                       //11. Lock the LOCK label data store 
  { 0xBB, 0x00, 0x65, 0x00, 0x04, 0x00, 0x00, 0xFF, 0xFF, 0x67,
    0x7E,
  },                                                //12. Inactivate the kill tag 
  {0xBB, 0x00, 0x11, 0x00, 0x02, 0x00, 0xC0, 0xD3, 0x7E,}, //13. Set communication baud rate 
  {0xBB, 0x00, 0x0D, 0x00, 0x00, 0x0D, 0x7E,},            //14. Get parameters related to the Query command 
  {0xBB, 0x00, 0x0E, 0x00, 0x02, 0x10, 0x20, 0x40, 0x7E,}, //15. Set the Query parameter 
  {0xBB, 0x00, 0x07, 0x00, 0x01, 0x03, 0x0B, 0x7E,},      //16. Set up work area 
  {0xBB, 0x00, 0x08, 0x00, 0x00, 0x08, 0x7E,},            //17. Acquire work locations 
  {0xBB, 0x00, 0xAB, 0x00, 0x01, 0x05, 0xB1, 0x7E,},      //18. Set up working channel 
  {0xBB, 0x00, 0xAA, 0x00, 0x00, 0xAA, 0x7E,},            //19. Get the working channel 
  {0xBB, 0x00, 0xAD, 0x00, 0x01, 0xFF, 0xAD, 0x7E,},      //20. Set to automatic frequency hopping mode
  { 0xBB, 0x00, 0xA9, 0x00, 0x06, 0x05, 0x01, 0x02,
    0x03, 0x04, 0x05, 0xC3, 0x7E,
  },                             //21. Insert the working channel 
  {0xBB, 0x00, 0xB7, 0x00, 0x00, 0xB7, 0x7E,},            //22. Acquire transmitting power 
  {0xBB, 0x00, 0xB6, 0x00, 0x02, 0x0B, 0xB8, 0x7B, 0x7E,}, //23. Set the transmitting power 
  {0xBB, 0x00, 0xB0, 0x00, 0x01, 0xFF, 0xB0, 0x7E,},      //24. Set up transmitting continuous carrier 
  {0xBB, 0x00, 0xF1, 0x00, 0x00, 0xF1, 0x7E,},            //25. Gets the receiving demodulator parameters 
  {0xBB, 0x00, 0xF0, 0x00, 0x04, 0x03, 0x06, 0x01, 0xB0, 0xAE, 0x7E,}, //26. Set the receiving demodulator parameters 
  {0xBB, 0x00, 0xF2, 0x00, 0x00, 0xF2, 0x7E,},            //27. Test the RF input block signal 
  {0xBB, 0x00, 0xF3, 0x00, 0x00, 0xF3, 0x7E,},            //28. Test the RSSI signal at the RF input 
  {0x00},
  {0xBB, 0x00, 0x17, 0x00, 0x00, 0x17, 0x7E,},            //30. Module hibernation 
  {0xBB, 0x00, 0x1D, 0x00, 0x01, 0x02, 0x20, 0x7E,},      //31. Idle hibernation time of module
  {0xBB, 0x00, 0x04, 0x00, 0x03, 0x01, 0x01, 0x03, 0x0C, 0x7E,}, //32. The IDLE mode 
  {0xBB, 0x00, 0xE1, 0x00, 0x05, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xE4, 0x7E,}, //33.NXP G2X label supports ReadProtect/Reset ReadProtect command 
  {0xBB, 0x00, 0xE3, 0x00, 0x05, 0x00, 0x00, 0xFF, 0xFF, 0x01, 0xE7, 0x7E,}, //34. The NXP G2X label supports the CHANGE EAS directive 
  {0xBB, 0x00, 0xE4, 0x00, 0x00, 0xE4, 0x7E,},            //35. The NXP G2X tag supports the EAS_ALARM directive 
  {0xBB, 0x00, 0xE0, 0x00, 0x06, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xE4, 0x7E,}, //36.NXP G2X label 16bits config-word 
  { 0xBB, 0x00, 0xE5, 0x00, 0x08, 0x00, 0x00, 0xFF,
    0xFF, 0x01, 0x01, 0x40, 0x00, 0x2D, 0x7E,
  },                   //37.Impinj Monza 4 Qt tags support Qt instructions 
  { 0xBB, 0x00, 0xD3, 0x00, 0x0B, 0x00, 0x00, 0xFF,
    0xFF, 0x01, 0x03, 0x00, 0x00, 0x01, 0x07, 0x00, 0xE8, 0x7E,
  },   //38.The BlockPermalock directive permanently locks blocks of a user's Block 
  { 0xBB, 0x00, 0x0C, 0x00, 0x13, 0x0B, 0x00, 0x00, 0x00, 0x30,
    0x60, 0x00, 0x30, 0x08, 0x33, 0xB2, 0xDD, 0xD9, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0xCE, 0x7E,
  },                        //39. Set the SELECT parameter instruction --> target == 000 == all tags, action == 010 == all tags regardless state, membank == 11 == TID
};

uint16_t calculateCRC(const std::string& hexCode) {
    uint16_t crc = 0xFFFF; // Initial value

    for (char hexChar : hexCode) {
        crc ^= static_cast<uint16_t>(hexChar) << 8; // XOR with high byte
        for (int i = 0; i < 8; ++i) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x8005) : (crc << 1); // Shift and XOR
        }
    }

    return crc;
}


//sends the command to the module
void Sendcommand(uint8_t com_nub)
{
  uint8_t b = 0;
  while (RFID_cmdnub[com_nub][b] != 0x7E)
  {
    Serial2.write(RFID_cmdnub[com_nub][b]);
    Serial.print(" 0x");
    Serial.print(RFID_cmdnub[com_nub][b], HEX);  
    b++;
  }
  Serial2.write(0x7E);
  Serial2.write("\n\r");
  Serial.println();
}

uint8_t DATA_I[16384];

//where the output is stored
void Readcallback()
{
  uint8_t DATA_I_NUB = 0;
  while(Serial2.available() == 0);
  while (Serial2.available())
  {
    delay(2);
    DATA_I[DATA_I_NUB] = Serial2.read();
      if(DATA_I[DATA_I_NUB] < 16) {
        Serial.print(" 0x0");
      } else {
        Serial.print(" 0x");
      }
      Serial.print(DATA_I[DATA_I_NUB], HEX);
    DATA_I_NUB++;
  }
  Serial.println();
}


bool checkIfTagExists(String EPCCode){
  for(int i = 0; i < tagsList.size(); i++){
    if(tagsList[i].equals(EPCCode)){
      return false;
    }
  }
  return true;
}

uint8_t calculateChecksum(uint8_t* command, uint8_t commandLength) {
  uint8_t checksum = 0;
  for (uint8_t i = 1; i < commandLength-1; i++) {
    checksum += command[i];
  }
  checksum = checksum % 256;
  return checksum;
}

void SendSetPowerCommand(uint16_t powerLevel) {
  // Set Transmit Power command (Command Index: 23)

  // Split power level into two bytes
  uint8_t powerLevelHighByte = powerLevel >> 8;
  uint8_t powerLevelLowByte = powerLevel & 0xFF;

  // Calculate payload length
  uint8_t payloadLength = 2;

  // Assemble the command array
  uint8_t setPowerCommand[11] = {
    0xBB,       // Header
    0x00,       // Type
    0xB6,       // Command number
    0x00,       // Payload length (high byte)
    payloadLength, // Payload length (low byte)
    powerLevelHighByte, // Power level (high byte)
    powerLevelLowByte,  // Power level (low byte)
    0x00,       // Placeholder for checksum      
  };

  // Calculate checksum  
  setPowerCommand[7] = calculateChecksum(setPowerCommand, 9);

  // Send the command to the module
  for (uint8_t b = 0; b < 8; b++) {
    Serial2.write(setPowerCommand[b]);
    Serial.print(" 0x");
    Serial.print(setPowerCommand[b], HEX);
  }
  
  Serial2.write(0x7E);
  Serial2.write("\n\r");
  Serial.println();
}


/* VERSION 1
void ReadcallbackCounting(){ 
  String EPCCode = "";
  uint8_t DATA_I_NUB = 0;
  while(Serial2.available() == 0);
  while (Serial2.available())
  9
  {
    delay(2);

    //console printing
    DATA_I[DATA_I_NUB] = Serial2.read();
    if(DATA_I[DATA_I_NUB] < 16) {
      Serial.print(" 0x0");
    } else {
      Serial.print(" 0x");
    }
    Serial.print(DATA_I[DATA_I_NUB], HEX);

   
    if(DATA_I_NUB % 24 >= 8 && DATA_I_NUB % 24 <= 19){
      if(DATA_I[DATA_I_NUB] < 16) {
        EPCCode = EPCCode + " 0" + String(DATA_I[DATA_I_NUB], HEX);
      } else {
        EPCCode = EPCCode + " "+ String(DATA_I[DATA_I_NUB], HEX);
      }
    }
    if(DATA_I_NUB % 24 == 0 && DATA_I_NUB != 0){
      if(checkIfTagExists(EPCCode)){
        tagsList[tagsList.size()] = EPCCode;
      }
      EPCCode = "";
    }
    DATA_I_NUB++;
  }
  Serial.println();
}*/

/* VERSION 2 */
void ReadcallbackCounting(){ 
  String EPCCode = "";
  uint16_t DATA_I_NUB = 0;
  //emptyning DATA_I
  for(int i = 0; i < 16384; i++){
    DATA_I[i] = 0;
  }
  while(Serial2.available() == 0);
  while (Serial2.available())
  {
    //console printing
    DATA_I[DATA_I_NUB] = Serial2.read();
    /*
    if(DATA_I[DATA_I_NUB] < 16) {
      Serial.print(" 0x0");
    } else {
      Serial.print(" 0x");
    }
    Serial.print(String(DATA_I[DATA_I_NUB], HEX));
    */
    DATA_I_NUB++;
  }
  Serial.println();

  DATA_I_NUB = 0;
  while( DATA_I_NUB < 16384)
  {
    if(String(DATA_I[DATA_I_NUB], HEX).equals("bb")){
      if(String(DATA_I[DATA_I_NUB+1], HEX).equals("2")){
        if (String(DATA_I[DATA_I_NUB+2], HEX).equals("22")){
          for(int i = 8; i < 20; i++){
            if(DATA_I[DATA_I_NUB+i] < 16) {
              EPCCode = EPCCode + " 0" + String(DATA_I[DATA_I_NUB+i], HEX);
            } else {
              EPCCode = EPCCode + " "+ String(DATA_I[DATA_I_NUB+i], HEX);
            }
          }
          int found = EPCCode.indexOf("7e");
          if(found == -1){
            if(checkIfTagExists(EPCCode) && String(DATA_I[DATA_I_NUB+23], HEX).equals("7e")){
              tagsList[tagsList.size()] = EPCCode;
            }
          }
          EPCCode = "";
        }
      }
    }
    DATA_I_NUB++;
  }
  
  
}


/* VERSION 3
void ReadcallbackCounting(){ 
  String EPCCode = "";
  uint16_t DATA_I_NUB = 0;
  while(Serial2.available() == 0);
  while (Serial2.available())
  {
    delay(2);

    //console printing
    DATA_I[DATA_I_NUB] = Serial2.read();
    if(DATA_I[DATA_I_NUB] < 16) {
      Serial.print(" 0x0");
    } else {
      Serial.print(" 0x");
    }
    Serial.print(String(DATA_I[DATA_I_NUB], HEX));
    DATA_I_NUB++;
  }
  Serial.println();

  DATA_I_NUB = 0;
  while( DATA_I_NUB < 1024)
  {
    if(String(DATA_I[DATA_I_NUB], HEX).equals("bb")){
      if(String(DATA_I[DATA_I_NUB+1], HEX).equals("1")){
        if (String(DATA_I[DATA_I_NUB+2], HEX).equals("39")){
          for(int i = 8; i < 25; i++){
            if(DATA_I[DATA_I_NUB+i] < 16) {
              EPCCode = EPCCode + " 0" + String(DATA_I[DATA_I_NUB+i], HEX);
            } else {
              EPCCode = EPCCode + " "+ String(DATA_I[DATA_I_NUB+i], HEX);
            }
          }
          int found = EPCCode.indexOf("7e");
          if(found == -1){
            if(checkIfTagExists(EPCCode) && String(DATA_I[DATA_I_NUB+27], HEX).equals("7e")){
              tagsList[tagsList.size()] = EPCCode;
            }
          }
          EPCCode = "";
        }
      }
    }
    DATA_I_NUB++;
  }
}
*/

/* code */
void setup() {
  bool DEBUG = true;
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("begin of UHF-Reader");
  Serial2.begin(115200, SERIAL_8N1, 19,21);
  
  //set working area to Europe(depends on the region)
  Serial.println("Set up work area:");
  Sendcommand(16);
  Serial.println("Receiving: ");
  Readcallback();

  //setting the power to 26dBm
  Serial.println("Set Transmit Power to 26dBm");
  Sendcommand(23);
  Serial.println("Receiving:");
  Readcallback();

  //set working channel
  Serial.println("Set up working channel:");
  Sendcommand(18);
  Serial.println("Receiving:");
  Readcallback();

  //set automatic frequency hopping mode
  Serial.println("Set to automatic frequency hopping mode:");
  Sendcommand(20);
  Serial.println("Receiving:");
  Readcallback();
}

/*
void loop() {

  //send set select parameter instruction
  Serial.println("Set the SELECT parameter instruction:");
  Sendcommand(6);
  Serial.println("Receiving:");
  Readcallback();

  //send set select mode
  Serial.println("Set the SELECT mode:");
  Sendcommand(8);
  Serial.println("Receiving:");
  Readcallback();

  //write the label data store
  Serial.println("Write the label data store:");
  Sendcommand(10);
  Serial.println("Receiving:");
  Readcallback();
  
  
  Serial.println(tagsList.size());
  for (uint8_t i = 0; i < tagsList.size(); i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(tagsList[i]);
  }
  tagsList.clear();
  delay(5000);
}
*/


void loop() {

  //send start multiple polling command
  Serial.println("Start Multiple polling:");
  Sendcommand(4);

  //wait for duration
  delay(duration);

  //send stop multiple polling command
  Serial.println("Stop Multiple Polling");
  Sendcommand(5);

  //wait for 1 second
  delay(1000);

  //read the tags
  Serial.println("Recieving:");
  ReadcallbackCounting();
 
  //Serial printout of the tags
  Serial.println(tagsList.size());
  for (uint8_t i = 0; i < tagsList.size(); i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(tagsList[i]);
  }

  //wait for 1.5 seconds
  delay(1500);

  //clear the tagslist
  tagsList.clear();
}


//main driver
/*
void loop() {
  
  for (size_t i = 0; i < 10; i++) {
    Serial.println("Single polling:");
    Sendcommand(3);
    Serial.println("Recieving:");
    Readcallback();
    Serial.println("Requesting data:");
    Sendcommand(9);
    Serial.println("Recieving data:");
    Readcallback();
    delay(3000);
    Serial.println();
  }
   Serial.println("End of programm");
  delay(5000);
}
*/

//Code for changing the epc code of an RFID tag
/*
void loop() {

  Serial.println("Single polling: ");
  Sendcommand(3);
  Serial.println("Receiving: ");
  ReadcallbackCounting();

  //set the select parameter
  Serial.println("Set the SELECT parameter instruction:");
  Sendcommand(6);
  Serial.println("Receiving:");
  Readcallback();

  //set the select mode
  Serial.println("Set the SELECT mode:");
  Sendcommand(8);
  Serial.println("Receiving:");
  Readcallback();

  //write the label data store
  Serial.println("Write the label data store:");
  Sendcommand(10);
  Serial.println("Receiving:");
  Readcallback();


  



  
  delay(10000);

  

  uint64_t startTime = millis();

  
  Serial.println("Start Multiple polling:");
  Sendcommand(4);

  
  while(millis() - startTime < duration)
  {
    
  }

  Serial.println("Stop Multiple Polling");
  Sendcommand(5);
 

  Serial.println("Recieving:");
  ReadcallbackCounting();
 
  Serial.println("GOED BEZIG");
  Serial.println(tagsList.size());
  for (size_t i = 0; i < tagsList.size(); i++) {
    Serial.print(i);
    Serial.print(": ");
    Serial.println(tagsList[i]);
  }
  
}
*/