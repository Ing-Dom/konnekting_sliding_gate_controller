#include <KonnektingDevice.h>
#include "config.h"


// include device related configuration code, created by "KONNEKTING CodeGenerator"
#include "kdevice_sgc.h"



unsigned long mytime = 0;
unsigned long mytime2 = 0;




// Callback function to handle com objects updates
void knxEvents(byte index)
{
  switch (index)
  {
        case COMOBJ_cmd_close:
            if (Knx.read(COMOBJ_cmd_close))
            {
                Debug.println(F("Received 1 on cmd_close"));

                //ToDo: check if 1
                //If yes, set OUT_OPENCLOSE to 1 for 500ms
            }
            break;

        default:
            break;
    }
}

void setup()
{

    // debug related stuff
#ifdef KDEBUG

    // Start debug serial with 9600 bauds
    DEBUGSERIAL.begin(9600);

#ifdef __AVR_ATmega32U4__
    // wait for serial port to connect. Needed for Leonardo/Micro/ProMicro only
    while (!DEBUGSERIAL)
#endif

    // make debug serial port known to debug class
    // Means: KONNEKTING will sue the same serial port for console debugging
    Debug.setPrintStream(&DEBUGSERIAL);
#endif

    setup_gpio();

    // Initialize KNX enabled Arduino Board
    Konnekting.init(KNX_SERIAL, PROG_BUTTON_PIN, PROG_LED_PIN, MANUFACTURER_ID, DEVICE_ID, REVISION);
    if (!Konnekting.isFactorySetting())
    {
        ;
    }
    Debug.println(F("Setup is ready. go to loop..."));
}


void setup_gpio()
{
  digitalWrite(OUT_OPENCLOSE, LOW);
  digitalWrite(OUT_OPENPART, LOW);
  
  pinMode(OUT_OPENCLOSE, OUTPUT);
  pinMode(OUT_OPENPART, OUTPUT);

  pinMode(IN_CLOSED, INPUT);
  pinMode(IN_OPENED, INPUT);
  pinMode(IN_BARRIER, INPUT);
  pinMode(IN_CURRENT, INPUT);
}

void reboot()
{
    Knx.end();

#ifdef ESP8266 
    DEBUG_PRINTLN(F("ESP8266 restart"));
    ESP.restart();
#elif __SAMD21G18A__
    // do reset of arduino zero
    setupWDT(0); // minimum period
    while (1) {
    }
#elif __AVR_ATmega328P__
    // to overcome WDT infinite reboot-loop issue
    // see: https://github.com/arduino/Arduino/issues/4492
    DEBUG_PRINTLN(F("software reset NOW"));
    delay(500);
    asm volatile ("  jmp 0");
#else     
    DEBUG_PRINTLN(F("WDT reset NOW"));
    wdt_enable(WDTO_500MS);
    while (1) {
    }
#endif    

}

void loop()
{
    Knx.task();
    unsigned long currentTime = millis();
    
    // only do measurements and other sketch related stuff if not in programming mode
    if (Konnekting.isReadyForApplication())
    {
      if(currentTime > mytime + 25)
      {
        int in_current_raw = analogRead(IN_CURRENT);
      
        mytime = currentTime;
      }
      
      if(currentTime > mytime2 + 5000)
      {
        
        mytime2 = currentTime;
      }
    }
}

