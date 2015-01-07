#include "LLAPSerial.h"
#include <EEPROM.h>
//#include "EEPROMAnything.h"
#include <avr/wdt.h>

#define DEVICEID "HC"   // this is the LLAP device ID

#define RLYPIN 10
#define TESTPIN 13

boolean heating = false;

#define T1SETADR 0
byte T1Set = 220; // поделить на 10 . осторожно!!!! макс температура 25,5 С

#define T1HYSTADR 1
byte T1Hyst = 10; // поделить на 10




void setup()
{
    wdt_disable(); // бесполезная строка до которой не доходит выполнение при bootloop
    wdt_enable (WDTO_8S);

    // read a byte from the current address of the EEPROM
    T1Set = EEPROM.read(T1SETADR);
    T1Hyst = EEPROM.read(T1HYSTADR);

    Serial.begin(115200);
    pinMode(RLYPIN, OUTPUT);  // relay pin
    pinMode(TESTPIN, OUTPUT);  // relay pin

    pinMode(8, OUTPUT);  // Radio enable pin
    digitalWrite(8, HIGH);

    pinMode(4, OUTPUT);       // Radio sleep pin
    digitalWrite(4, LOW); // ensure the radio is not sleeping

    delay(1000);              // allow the radio to startup

    LLAP.init(DEVICEID);
    LLAP.sendMessage(F("STARTED"));
}

void procPersonalMes()
{
    if (LLAP.bMsgReceived == false) return; // если нет новых сообщений то валим

    // убираем лишние дефисы с конца
    while (LLAP.sMessage.endsWith("-"))
        LLAP.sMessage.remove(LLAP.sMessage.length() - 1);


    if (LLAP.sMessage.startsWith("T1SET")) {
        float newT1Set = LLAP.sMessage.substring(5).toFloat();
        if ((newT1Set > 10) and (newT1Set < 25.5)) { // нехрен делать уставку температуры меньше 10С
            T1Set = newT1Set * 10;
            EEPROM.write(T1SETADR, T1Set);
        }
        LLAP.sendIntWithDP(F("T1SET"), T1Set, 1);   // just echo the message back

    } else if (LLAP.sMessage.startsWith("T1HYST")) {
        float newT1Hyst = LLAP.sMessage.substring(6).toFloat();
        if (newT1Hyst >= 0) {
            T1Hyst = newT1Hyst * 10;
            EEPROM.write(T1HYSTADR, T1Hyst);
        }
        LLAP.sendIntWithDP(F("T1HYST"), T1Hyst, 1);   // just echo the message back

    } else if (LLAP.sMessage.startsWith("HELLO")) {
        digitalWrite(TESTPIN, HIGH);
        LLAP.sendMessage(F("HELLO"));

    } else if (LLAP.sMessage.startsWith("GIVEMESET")) { // запрос всех настроек
        LLAP.sendIntWithDP(F("T1SET"), T1Set, 1);
        LLAP.sendIntWithDP(F("T1HYST"), T1Hyst, 1);
    
    } else 
        LLAP.sendMessage(F("ERROR"));

    LLAP.bMsgReceived = false;    // сообщение как смогли обработали, готовы к приему следующего
}

void procBroadcast()
{
    if (LLAP.bNotMyMsgReceived == false) return; // если нет новых чужих сообщений то валим

    while (LLAP.sNotMyMessage.endsWith("-"))
        LLAP.sNotMyMessage.remove(LLAP.sNotMyMessage.length() - 1);

    if (LLAP.sNotMyMessage.startsWith("T1TMPA")) { // пришли данные по температуре T1TMPA23.74
        //LLAP.sendMessage("HEATing");
        float newT1 = LLAP.sNotMyMessage.substring(6).toFloat();
        if ((newT1 <= (T1Set - T1Hyst) / 10.0) and !heating ) {
            heating = true;
            digitalWrite(RLYPIN, HIGH);
            LLAP.sendMessage(F("HEATON T1"));
        } else if ((newT1 >= T1Set / 10.0) and heating ) {
            heating = false;
            digitalWrite(RLYPIN, LOW);
            LLAP.sendMessage(F("HEATOFFT1"));
        }
    }
    LLAP.bNotMyMsgReceived = false;// сообщение как смогли обработали, готовы к приему следующего
}

void loop()
{
    procPersonalMes();
    procBroadcast();

    //  digitalWrite(TESTPIN, HIGH);
    //  delay(1000);
    //  digitalWrite(TESTPIN, LOW);
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
    wdt_reset();
}



















