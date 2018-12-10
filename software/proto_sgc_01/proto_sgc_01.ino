#include <KonnektingDevice.h>
#include "config.h"
#include <Bounce2.h>


// include device related configuration code, created by "KONNEKTING CodeGenerator"
#include "kdevice_sgc.h"

// Stuff for CycleControl

#define T1_CYCLETIME 25
#define T2_CYCLETIME 50
#define T3_CYCLETIME 100
#define T4_CYCLETIME 500
#define T5_CYCLETIME 10000
unsigned long T1_last_run = 0;
unsigned long T2_last_run = 0;
unsigned long T3_last_run = 0;
unsigned long T4_last_run = 0;
unsigned long T5_last_run = 0;
unsigned int T1_duration = 0;
unsigned int T2_duration = 0;
unsigned int T3_duration = 0;
unsigned int T4_duration = 0;
unsigned int T5_duration = 0;
unsigned int T1_overtime = 0;
unsigned int T2_overtime = 0;
unsigned int T3_overtime = 0;
unsigned int T4_overtime = 0;
unsigned int T5_overtime = 0;

// End Stuff for CycleControl


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

Bounce debouncer_in_closed = Bounce();
Bounce debouncer_in_opened = Bounce(); 
Bounce debouncer_in_barrier = Bounce();

bool in_closed = false;
bool in_opened = false;
bool in_barrier = false;
unsigned int out_openclose_cnt = 0;
unsigned int out_openpart_cnt = 0;
unsigned int in_current_smoothed = 0;



// Debug stuff
bool enable_drivecurrent_sendknx = false;




// Callback function to handle com objects updates
void knxEvents(byte index)
{
  switch (index)
  {
        case COMOBJ_cmd_close:
            if (Knx.read(COMOBJ_cmd_close))
            {
                Debug.println(F("Received 1 on cmd_close"));
                if(in_opened)
                {
                  Debug.println(F("cmd_close => openclose"));
                  out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
                }
            }
       break;
       case COMOBJ_cmd_open:
            if (Knx.read(COMOBJ_cmd_open))
            {
                Debug.println(F("Received 1 on cmd_open"));
                if(in_closed)
                {
                  Debug.println(F("cmd_open => openclose"));
                  out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
                }
            }
       break;
        case COMOBJ_cmd_stop:
            if (Knx.read(COMOBJ_cmd_stop))
            {
                Debug.println(F("Received 1 on COMOBJ_cmd_stop"));

                out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
                
            }
        break;
       case COMOBJ_cmd_partly_open:
            if (Knx.read(COMOBJ_cmd_partly_open))
            {
                Debug.println(F("Received 1 on COMOBJ_cmd_partly_open"));
                enable_drivecurrent_sendknx = true;
            }
            else
            {
                Debug.println(F("Received 0 on COMOBJ_cmd_partly_open"));
                enable_drivecurrent_sendknx = false;
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
      callT();
    }
}

void callT()
{
  unsigned long currentMillis = millis();
  //Debug.println(F("elapsedT1: %u %u %u %u %u"), calculateElapsedMillis(T1_last_run, currentMillis), T1_last_run, currentMillis );
  if(calculateElapsedMillis(T1_last_run, currentMillis) >= T1_CYCLETIME)
  {
    T1_overtime = calculateElapsedMillis(T1_last_run, currentMillis) - T1_CYCLETIME;
    T1_last_run = currentMillis;
    T1();
    T1_duration = calculateElapsedMillis(currentMillis, millis());
  }
  else if(calculateElapsedMillis(T2_last_run, currentMillis) >= T2_CYCLETIME)
  {
    T2_overtime = calculateElapsedMillis(T2_last_run, currentMillis) - T2_CYCLETIME;
    T2_last_run = currentMillis;
    T2();
    T2_duration = calculateElapsedMillis(currentMillis, millis());
  }
  else if(calculateElapsedMillis(T3_last_run, currentMillis) >= T3_CYCLETIME)
  {
    T3_overtime = calculateElapsedMillis(T3_last_run, currentMillis) - T3_CYCLETIME;
    T3_last_run = currentMillis;
    T3();
    T3_duration = calculateElapsedMillis(currentMillis, millis());
  }
  else if(calculateElapsedMillis(T4_last_run, currentMillis) >= T4_CYCLETIME)
  {
    T4_overtime = calculateElapsedMillis(T4_last_run, currentMillis) - T4_CYCLETIME;
    T4_last_run = currentMillis;
    T4();
    T4_duration = calculateElapsedMillis(currentMillis, millis());
  }
  else if(calculateElapsedMillis(T5_last_run, currentMillis) >= T5_CYCLETIME)
  {
    T5_overtime = calculateElapsedMillis(T5_last_run, currentMillis) - T5_CYCLETIME;
    T5_last_run = currentMillis;
    T5();
    T5_duration = calculateElapsedMillis(currentMillis, millis());
  }
}

void T1()
{
  int in_current_raw = analogRead(IN_CURRENT);
  
  
  // Handling of Inputs
  
  if(in_closed != debouncer_in_closed.read())
  {
    in_closed = debouncer_in_closed.read();
    Knx.write(COMOBJ_stat_closed, in_closed);
    Debug.println(F("inclosed changed: %d"), in_closed );
  }

  if(in_opened != debouncer_in_opened.read())
  {
    in_opened = debouncer_in_opened.read();
    Knx.write(COMOBJ_stat_opened, in_opened);
    Debug.println(F("inopened changed: %d"), in_opened );
  }

  if(in_barrier != debouncer_in_barrier.read())
  {
    in_barrier = debouncer_in_barrier.read();
    Knx.write(COMOBJ_stat_barrier, in_barrier);
    Debug.println(F("inbarrier changed: %d"), in_barrier );
  }

  
  
  
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

  if(out_openpart_cnt > 0)
  {
    digitalWrite(OUT_OPENPART, HIGH);
    out_openpart_cnt--;
  }
  else
  {
    digitalWrite(OUT_OPENPART, LOW);
  }
}

void T2()
{
  
}

void T3()
{
  in_current_smoothed = 0.2 * analogRead(IN_CURRENT) + 0.8 * in_current_smoothed;
  if(enable_drivecurrent_sendknx)
  {
    Knx.write(COMOBJ_debug, in_current_smoothed);
  }
}

void T4()
{
  
}

void T5()
{
  Knx.write(COMOBJ_debug, 0);
  //Debug.println(F("in_current_smoothed: %d"), in_current_smoothed );
}

unsigned long calculateElapsedMillis(unsigned long lastrunMillis, unsigned long currentMillis)
{
  if(currentMillis >= lastrunMillis)
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

