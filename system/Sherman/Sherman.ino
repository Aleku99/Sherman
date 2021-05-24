#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define PUMP_PIN 8
#define BUTTON_PIN 3
#define ONE_SECOND 1000

enum mode_type {Timer, Sensor};

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
uint8_t  watering_time = 2; //in seconds
mode_type mode = Timer;
uint32_t current_time = 0;  //in seconds;
uint32_t watering_interval = 1; //in minutes
uint32_t intervals_array[10] = {1, 5, 15, 30, 60, 120, 240, 480, 960, 1400}; //minutes 
uint32_t watertime_array[10] = {1, 2, 3, 5, 8, 10, 15, 30}; //seconds
uint16_t humidity_level; 
NfcAdapter nfc2 = NfcAdapter(pn532i2c); 
NdefMessage msg; 
NdefRecord rcd;
byte payload_array[100];
int payload_length = 0;
volatile int interrupt_called; //used like a flag to be able to create new config


/************TEST***************/
void test_config()
{
  if(current_time == (60 * ONE_SECOND * watering_interval)) Serial.println("Water interval configuration works.");
  else Serial.println("Unexpected call of water function. (Interval length was not reached)");
}
/*******************************/

void read_message()
{ 
  
  if (nfc2.tagPresent()) 
  {
      NfcTag tag = nfc2.read();
      tag.print();
      msg = tag.getNdefMessage();
      rcd = msg.getRecord(0);
      rcd.getPayload(payload_array);
      
      if(payload_array[3]=='6' && payload_array[4]=='9')  //first 2 bytes from NDEF should be 6 and 9
      {
        payload_length = rcd.getPayloadLength();
        Serial.println("Received tag from irrigation system app");
        process_message();
      }
      else if(payload_array[3]=='7' && payload_array[4]=='0')
      {
        payload_length = rcd.getPayloadLength();
        Serial.println("Received tag from irrigation system app");
        process_message();
      }
      else
      {
        Serial.println("Received tag from unknown device"); 
      }
      
      interrupt_called = 0;
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
  /*
   Message structure:
   payload_array[3] & payload_array[4]: used for recognising app and mode 
   payload_array[5] : used for watering_interval
   payload_array[6]: used for watering time
   */
  uint32_t temp_watering_interval, temp_watering_time, temp_mode;

  temp_watering_interval =  intervals_array[payload_array[5] - 48]; 
  temp_watering_time = watertime_array[payload_array[6] - 48]; 
  temp_mode = (payload_array[3]=='7'); 
  change_config(temp_mode, temp_watering_interval, temp_watering_time);
}

void change_config(uint8_t i_mode, uint32_t i_watering_interval, uint8_t i_watering_time)
{
  mode = i_mode;
  watering_interval = i_watering_interval;
  watering_time = i_watering_time;
  current_time = 0;
  Serial.print("watering_interval = ");
  Serial.println(watering_interval);
  Serial.print("watering_time = ");
  Serial.println(watering_time);
  Serial.print("mode = ");
  Serial.println(mode);
}

void crazy_loop()
{
  Serial.println(current_time);
  if(mode == Timer)
  {
    if(current_time  >= (watering_interval * 60 * ONE_SECOND))
    {
      water();
      current_time = ONE_SECOND + watering_time * ONE_SECOND;  
    }
    else
    {
      current_time = current_time + ONE_SECOND;;  
    }
  }
  else if(mode == Sensor)
  {
    if(humidity_level < 400)
    {
      water();  
    }
  }
  delay(ONE_SECOND);
}
void water()
{
  test_config();
  digitalWrite(PUMP_PIN, LOW);
  delay(watering_time * ONE_SECOND);
  digitalWrite(PUMP_PIN,HIGH);
  delay(ONE_SECOND);
  interrupt_called = 0;
}
void ISR_button()
{
  interrupt_called = 1;
}
void setup(void) 
{  
  Serial.begin(115200);

  Serial.println("Initialising");
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  Serial.println(versiondata);
  //commented to test sleep mode 
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

  //configure interrupt pin
   pinMode(BUTTON_PIN, INPUT_PULLUP);
   attachInterrupt(digitalPinToInterrupt(BUTTON_PIN),ISR_button,FALLING);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, HIGH);
  interrupt_called = 0;
}

void loop(void)
{
  if(interrupt_called == 1)
  {
     read_message();
  }
  else
  {
    crazy_loop(); 
  } 
  
}
