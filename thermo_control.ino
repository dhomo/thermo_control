#include <LLAPSerial.h>
#include <EEPROM.h>
//#include "EEPROMAnything.h"

#define DEVICEID "TC"	// this is the LLAP device ID

#define RLYPIN 2     // what I/O the DHT-22 data pin is connected to

boolean heating = false;

//struct config_t
//{
//  long alarm;
//  int mode;
//} 
//configuration;

byte T1Set = 240; // поделить на 10 . осторожно!!!! макс температура 25,5 С
byte T1Hyst = 10; // поделить на 10


void setup() {
  //  EEPROM_readAnything(0, configuration);

  Serial.begin(115200);
  pinMode(8,OUTPUT);	      
  // Radio enable pin  
  digitalWrite(8,HIGH);
  pinMode(4,OUTPUT);		// Radio sleep pin
  digitalWrite(4,LOW);	// ensure the radio is not sleeping
  delay(1000);				// allow the radio to startup

  LLAP.init(DEVICEID);
  LLAP.sendMessage(F("STARTED"));
}

void processMessage(){
  if (LLAP.bMsgReceived == false) return; // если нет новых сообщений то валим


  while (LLAP.sMessage.endsWith("-"))
    LLAP.sMessage.remove(LLAP.sMessage.length()-1);


  if (LLAP.sMessage.startsWith("T1SET"))
  {

    float newT1Set = LLAP.sMessage.substring(5).toFloat();

    if ((newT1Set !=0) and (newT1Set > 10)) // нехрен делать уставку температуры меньше 10С     
      T1Set = newT1Set*10;

    LLAP.sendIntWithDP("T1SET", T1Set, 1);   // just echo the message back
  }

  else if (LLAP.sMessage.startsWith("T1HYST"))
  {
    float newT1Hyst = LLAP.sMessage.substring(6).toFloat();
    if (newT1Hyst >=0)      
      T1Hyst = newT1Hyst*10;

    LLAP.sendIntWithDP("T1HYST", T1Hyst, 1);   // just echo the message back
  }

  else if (LLAP.sMessage.startsWith("GIVEMESET")) // запрос всех настроек
  {
    LLAP.sendIntWithDP("T1SET", T1Set, 1);   
    LLAP.sendIntWithDP("T1HYST", T1Hyst, 1);    
    LLAP.bMsgReceived = false;	// if we do not clear the message flag then message processing will be blocked
    return;
  }

  LLAP.bMsgReceived = false;	// сообщение как смогли обработали, готовы к приему следующего
}

void listener(){
  if (LLAP.bNotMyMsgReceived == false) return; // если нет новых сообщений то валим

  while (LLAP.sNotMyMessage.endsWith("-"))
    LLAP.sMessage.remove(LLAP.sMessage.length()-1);


  if (LLAP.sNotMyMessage.startsWith("T1TMPA")) // пришли данные по температуре T1TMPA23.74
  {
    LLAP.sendMessage("HEATing");
    float newT1 = LLAP.sNotMyMessage.substring(6).toFloat();
    if (newT1 <= (T1Set - T1Hyst)/10.0)      
      LLAP.sendMessage("HEATON T1");
    if (newT1 >= T1Set/10.0)      
      LLAP.sendMessage("HEATOFFT1");        
  }


  LLAP.bNotMyMsgReceived = false;// сообщение как смогли обработали, готовы к приему следующего
}

void loop() {
  processMessage();
  listener();
  //  delay(1000);
  //  LLAP.sendIntWithDP("T1SET", T1Set, 1);   
  //   LLAP.sendIntWithDP("T1HYST", T1Hyst, 1); 

  //  static unsigned long lastTime = millis();
  //  if (millis() - lastTime >= 30000)
  //  {
  //    //    lastTime = millis();
  //    //    int h = dht.readHumidity() * 10;
  //    //    int t = dht.readTemperature() * 10;
  //    //    // check if returns are valid, if they are NaN (not a number) then something went wrong!
  //    //    if (isnan(t) || isnan(h)) {
  //    //      LLAP.sendMessage(F("ERROR"));
  //    //    } 
  //    //    else {
  //    //      LLAP.sendIntWithDP("HUM",h,1);
  //    //      //delay(100);
  //    //      LLAP.sendIntWithDP("TMP",t,1);
  //    //    }
  //  }
}













