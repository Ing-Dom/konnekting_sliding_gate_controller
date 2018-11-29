#include <KonnektingDevice.h>
#include "config.h"
#include <Bounce2.h>


// include device related configuration code, created by "KONNEKTING CodeGenerator"
#include "kdevice_sgc.h"


Bounce debouncer_in_closed = Bounce();
Bounce debouncer_in_opened = Bounce(); 
Bounce debouncer_in_barrier = Bounce(); 


#define T1_CYCLETIME 25
#define T2_CYCLETIME 50
#define T3_CYCLETIME 100
#define T4_CYCLETIME 500
#define T5_CYCLETIME 1000
unsigned long T1_last_run = 0;
unsigned long T2_last_run = 0;
unsigned long T3_last_run = 0;
unsigned long T4_last_run = 0;
unsigned long T5_last_run = 0;


int out_openclose_cnt = 0;
bool in_closed_lastvalues[8];

enum gate_state 
{ 
   unknown,
   opened,
   opening,
   closed,
   closing,
   intermediate_will_open,
   intermediate_will_close
} main_gate_state;




// Callback function to handle com objects updates
void knxEvents(byte index)
{
  switch (index)
  {
        case COMOBJ_cmd_close:
            if (Knx.read(COMOBJ_cmd_close))
            {
                Debug.println(F("Received 1 on cmd_close"));

                out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
                
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

  debouncer_in_closed.attach(IN_CLOSED);
  debouncer_in_closed.interval(100); // interval in ms

  debouncer_in_opened.attach(IN_OPENED);
  debouncer_in_opened.interval(100); // interval in ms

  debouncer_in_barrier.attach(IN_BARRIER);
  debouncer_in_barrier.interval(100); // interval in ms
}



void loop()
{
    Knx.task();
    debouncer_in_closed.update();
    debouncer_in_opened.update();
    debouncer_in_barrier.update();
    unsigned long currentTime = millis();
    
    // only do measurements and other sketch related stuff if not in programming mode
    if (Konnekting.isReadyForApplication())
    {
      if(currentTime > mytime + 25)
      {
        int in_current_raw = analogRead(IN_CURRENT);
      
        mytime = currentTime;


       
        // Handling of Inputs
        in_closed = debouncer_in_closed.read();




         // Handling of Outputs

        if(out_openclose_cnt > 0)
        {
          digitalWrite(OUT_OPENCLOSE, HIGH);
          out_openclose_cnt--;
        }
        else
        {
          digitalWrite(OUT_OPENCLOSE, LOW);
        }
      }
    }
      
      if(currentTime > mytime2 + 5000)
      {
        
        mytime2 = currentTime;
      }
    }
}

void callT()
{
  unsigned long currentMillis = millis();

  if(calculateElapsedMillis(T1_last_run, currentMillis) >= T1_CYCLETIME)
  {
    T1();
    //ToDo: Diag runtime of slice
  }
  else if(calculateElapsedMillis(T2_last_run, currentMillis) >= T2_CYCLETIME)
  {
    T2();
  }
}

void T1()
{
  
}
void T2()
{
  
}
void T3()
{
  
}
void T4()
{
  
}
void T5()
{
  
}

unsigned long calculateElapsedMillis(unsigned long lastrunMillis, unsigned long currentMillis)
{
  if(currentMillis > lastrunMillis)
    return currentMillis-lastrunMillis;
  else // this means, there was an overflow
    return (0xFFFFFFFF-lastrunMillis)+currentMillis;
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

