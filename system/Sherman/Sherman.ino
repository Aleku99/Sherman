#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define PUMP_PIN 13;

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
uint8_t message[]={0,0,0,0,0,0,0};
uint8_t message_length = 0;
uint8_t mode, watering_time,watering_interval;

void read_message()
{
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if (success) 
  {
    Serial.println("Found an NFC device!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    uint8_t  payload;
    for (uint8_t i=0; i < uidLength; i++) 
    {
      Serial.print(" 0x");Serial.print(uid[i], HEX);       
      //Serial.print(" ");Serial.print(uid[i], HEX);       
      //hex_value += (String)uid[i];
    }
    payload = uid[0]+uid[1];  
    //Serial.println(", value="+hex_value);
    if(payload == 0xF )  //first 2 bytes from received tag should be 0x06 0x09; this is the value set by out app in the tag 
    {
      Serial.print("Received tag from irrigation system app.");
      for (uint8_t i=0; i < uidLength; i++) 
      {
        message[i] = uid[i];      
      }
      message_length = uidLength;
    }
    else
    {
      Serial.print("Received tag from unknown device"); 
    }
    Serial.println("");
    
    // Wait 1 second before continuing
    delay(1000);
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Waiting for a device...");
  }
}
void process_message()
{
  
}
void change_config(uint8_t mode, uint16_t watering_interval, uint8_t watering_time)
{
  
}

void setup(void) 
{  
  Serial.begin(115200);
  Serial.println("Hello!");
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) 
  {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  // configure board to read RFID tags
  nfc.SAMConfig();
  Serial.println("Waiting for an ISO14443A card");
}

void loop(void)
{
   uint8_t message[]={0,0,0,0,0,0,0};
   uint8_t message_length = 0;
   read_message();
   if(message_length != 0)
   {
      //start doing things
      process_message(); 
      change_config(mode,watering_interval_watering_time);
   }
   else
   {
      //do nothing
   }  
}
