#include <KonnektingDevice.h>
#include "config.h"
#include <Bounce2.h>
#include <EEPROM.h>


// include device related configuration code, created by "KONNEKTING CodeGenerator"
#include "kdevice_sgc.h"

// Stuff for CycleControl

#define T1_CYCLETIME 25
#define T2_CYCLETIME 4
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


#define EEPROM_ADR_ZCV_VALID 0
#define EEPROM_ADR_ZCV_LB 1
#define EEPROM_ADR_ZCV_HB 2

#define EEPROM_ADR_ZCV_VALID_MAGIC 158


int param_drivecurrent_zero = 0;
int param_drivecurrent_jit = 0;
int param_drivecurrent_num = 0;
bool param_drivecurrent_autozero = false;
unsigned short param_send_status_cyclic = 0;


Bounce debouncer_in_closed = Bounce();
Bounce debouncer_in_opened = Bounce(); 
Bounce debouncer_in_barrier = Bounce();

bool in_closed = false;
bool in_opened = false;
bool in_barrier = false;
unsigned int out_openclose_cnt = 0;
unsigned int out_openpart_cnt = 0;

int in_current_smoothed = 0;
int in_currents[50];
unsigned short in_currents_pointer = 0;
int in_current_smoothed_int = 0;
int in_current_raw_old3 = 0;
int in_current_raw_old2 = 0;
int in_current_raw_old1 = 0;

unsigned short not_move_count = 0;
int zero_current_values[30];
unsigned short zero_current_values_pointer = 0;
int zero_current_value = 0;



unsigned short opening_cnt = 0;
unsigned short closing_cnt = 0;
unsigned short stopped_cnt = 0;
bool moving_opening = false;
bool moving_closing = false;




// Debug stuff
bool test1 = false;
bool test2 = false;
bool test3 = false;





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
       case COMOBJ_cmd_leave_open:
            if (Knx.read(COMOBJ_cmd_leave_open))
            {
                Debug.println(F("Received 1 on COMOBJ_cmd_leave_open"));
            }
       break;
       case COMOBJ_cmd_partly_open:
            if (Knx.read(COMOBJ_cmd_partly_open))
            {
                Debug.println(F("Received 1 on COMOBJ_cmd_partly_open"));
                out_openpart_cnt = 20; // 20x 25ms = 500ms set OUT_OPENPART to 1 for 500ms
            }
       break;
       case COMOBJ_cmd_stop:
            if (Knx.read(COMOBJ_cmd_stop))
            {
                Debug.println(F("Received 1 on COMOBJ_cmd_stop"));

                if(!in_closed && !in_opened && !moving_opening && !moving_closing) // ToDo
                {
                   out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
                }
            }
        break;
        case COMOBJ_cmd_position:
            Debug.println(F("Received %u on COMOBJ_cmd_position"),Knx.read(COMOBJ_cmd_position));
        break;

        case COMOBJ_test1:
            if (Knx.read(COMOBJ_test1))
            {
                Debug.println(F("Received 1 on COMOBJ_test1"));
                test1 = true;
            }
            else
            {
              Debug.println(F("Received 0 on COMOBJ_test1"));
              test1 = false;
            }
       break;

       case COMOBJ_test2:
            if (Knx.read(COMOBJ_test2))
            {
                Debug.println(F("Received 1 on COMOBJ_test2"));
                test2 = true;
            }
            else
            {
              Debug.println(F("Received 0 on COMOBJ_test2"));
              test2 = false;
            }
       break;

        case COMOBJ_test3:
            if (Knx.read(COMOBJ_test3))
            {
                Debug.println(F("Received 1 on COMOBJ_test3"));
                out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENPART to 1 for 500ms
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
		param_send_status_cyclic = (int)(Konnekting.getUINT16Param(PARAM_send_status_cyclic));
        param_drivecurrent_zero = (unsigned short)(Konnekting.getUINT8Param(PARAM_drivecurrent_zero));
        param_drivecurrent_jit = (int)(Konnekting.getUINT16Param(PARAM_drivecurrent_jit));
        param_drivecurrent_num = (int)(Konnekting.getUINT16Param(PARAM_drivecurrent_num));
		param_drivecurrent_autozero = Konnekting.getUINT8Param(PARAM_drivecurrent_autozero);
		
		if(param_drivecurrent_autozero)
		{
			int zero_current_value_from_eeprom = GetZeroCurrentValueFromEEPROM();
			if(zero_current_value_from_eeprom >= 0)
			{
				zero_current_value = zero_current_value_from_eeprom;
			}
			else
			{
				zero_current_value = param_drivecurrent_zero;
			}
		}
		else
		{
			zero_current_value = param_drivecurrent_zero;
		}
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

void T1() // 25ms
{
  int in_current_raw = analogRead(IN_CURRENT);
  
  
  // Handling of Inputs
  
  // open and close are Low-active
  
  if(in_closed != !(debouncer_in_closed.read()))
  {
    in_closed = !(debouncer_in_closed.read());
    Knx.write(COMOBJ_stat_closed, in_closed);
    Debug.println(F("inclosed changed: %d"), in_closed );
  }

  if(in_opened != !(debouncer_in_opened.read()))
  {
    in_opened = !(debouncer_in_opened.read());
    Knx.write(COMOBJ_stat_opened, in_opened);
    Debug.println(F("inopened changed: %d"), in_opened );
  }

  if(in_barrier != !(debouncer_in_barrier.read()))
  {
    in_barrier = !(debouncer_in_barrier.read());
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

void T2() // 4ms
{
  // Start Reading drive current in buffer
  in_currents[in_currents_pointer] =  analogRead(IN_CURRENT);
  if(in_currents_pointer < 50)
    in_currents_pointer++;
  // End Reading drive current in buffer
}

void T3() // 100ms
{
  // Start Smoothing drive current
  int in_current_raw =  0;
  long in_current_sum = 0;
  
  for(int i = 0;i<in_currents_pointer;i++)
  {
    in_current_sum+=in_currents[i];
  }
  in_current_raw = in_current_sum / (in_currents_pointer+1);
  in_currents_pointer = 0;


  
  in_current_smoothed = in_current_raw/2 + in_current_raw_old1/4 + in_current_raw_old2/8 + in_current_raw_old3/8;

  in_current_raw_old3 = in_current_raw_old2;
  in_current_raw_old2 = in_current_raw_old1;
  in_current_raw_old1 = in_current_raw;
  
  in_current_smoothed_int = in_current_smoothed - zero_current_value;
  // End Smoothing drive current
  
  // Start Detection of Closing and Opening
  if(in_current_smoothed_int > param_drivecurrent_jit) // Opening
  {
	  opening_cnt++;
    closing_cnt = 0;
    stopped_cnt = 0;
  }
  else if(in_current_smoothed_int < -param_drivecurrent_jit) // Closing
  {
    opening_cnt = 0;
    closing_cnt++;
    stopped_cnt = 0;
  }
  else
  {
    opening_cnt = 0;
    closing_cnt = 0;
    stopped_cnt++;
  }

  if(moving_opening)
  {
    if(stopped_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving, false);
    }
    else if(closing_cnt > param_drivecurrent_num)
    {
      moving_closing = true;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving_direction, true);
    }
  }
  else if(moving_closing)
  {
    if(opening_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = true;
      Knx.write(COMOBJ_stat_moving_direction, false);
    }
    else if(stopped_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving, false);
    }
  }
  else
  {
    if(opening_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = true;
      Knx.write(COMOBJ_stat_moving, true);
      Knx.write(COMOBJ_stat_moving_direction, false);
    }
    else if(closing_cnt > param_drivecurrent_num)
    {
      moving_closing = true;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving, true);
      Knx.write(COMOBJ_stat_moving_direction, true);
    }
  } 
  
  if(opening_cnt > param_drivecurrent_num)
  {
    moving_closing = false;
    moving_opening = true;
  }
  else if(closing_cnt > param_drivecurrent_num)
  {
    moving_closing = true;
    moving_opening = false;
  }
  // End Detection of Closing and Opening
  
  
  
  if(test1)
  {
    Knx.write(COMOBJ_debug1, in_current_raw);
  }

  if(test2)
  {
    Knx.write(COMOBJ_debug2, in_current_smoothed);
  }



  
}

unsigned short initcnt = 0;
unsigned int eepromwritecnt = 0;
void T4() // 500ms
{
  // Start Feature Auto Adaption of Zero Current Value
  if(param_drivecurrent_autozero)
  {
	  eepromwritecnt++;
	  
	  if((!moving_closing && !moving_opening) || in_closed || in_opened)	// usage of the end position switches here will auto adapt the ZCV even if programm detects moving due to wrong ZCV
	  {
		not_move_count++;
	  }
	  else
	  {
		not_move_count = 0;
		if(zero_current_values_pointer >= 5)
			zero_current_values_pointer -= 5;
		else
			zero_current_values_pointer = zero_current_values_pointer + 30 - 5;
	  }

	  if(not_move_count > 5)
	  {
		zero_current_values[zero_current_values_pointer] = in_current_smoothed;

		long sum = 0;
		
		for(int i=0;i<25;i++)
		{
		  int arraypointer = zero_current_values_pointer - 5 - i;
		  if(arraypointer < 0)
			arraypointer += 30;
		  sum += zero_current_values[arraypointer];
		}
		
		if(initcnt < 30)
		  initcnt++;
		else
		{
		  zero_current_value = sum / 25;
		  Knx.write(COMOBJ_error_drivecurrentzero, zero_current_value);
		  WriteZeroCurrentValueToEEPROM(zero_current_value);
		  //ToDo Log (use return val)
		}    
		
		if(zero_current_values_pointer == 29)
		  zero_current_values_pointer = 0;
		else
		  zero_current_values_pointer++;
	  }
  }
  // End Feature Auto Adaption of Zero Current Value
  
}

unsigned int send_status_cyclic_cnt = 0;
void T5() // 10000ms = 10s
{
  Knx.write(COMOBJ_debug5, in_current_smoothed);
  Knx.write(COMOBJ_debug4, zero_current_value);
  //Debug.println(F("in_current_smoothed: %d"), in_current_smoothed );
  
  // Start Cyclic Sending of Status
  if(param_send_status_cyclic)
  {
	  if(send_status_cyclic_cnt-1 >= ((60*1000) / T5_CYCLETIME) * param_send_status_cyclic) // send every param_send_status_cyclic minutes
	  {
		Knx.write(COMOBJ_stat_closed, in_closed);
		Knx.write(COMOBJ_stat_opened, in_opened);
		Knx.write(COMOBJ_stat_barrier, in_barrier);
		Knx.write(COMOBJ_stat_moving, moving_closing || moving_opening);
		// ToDo Positioning Knx.write(COMOBJ_stat_position, ???);
		send_status_cyclic_cnt = 0;
	  }
	  else
		send_status_cyclic_cnt++;
  }
}

unsigned long calculateElapsedMillis(unsigned long lastrunMillis, unsigned long currentMillis)
{
  if(currentMillis >= lastrunMillis)
    return currentMillis-lastrunMillis;
  else // this means, there was an overflow
    return (0xFFFFFFFF-lastrunMillis)+currentMillis;
}

int GetZeroCurrentValueFromEEPROM()
{
	int freeEepromOffset = Konnekting.getFreeEepromOffset();
	
	if(EEPROM.read(freeEepromOffset + EEPROM_ADR_ZCV_VALID) == EEPROM_ADR_ZCV_VALID_MAGIC)
	{
		int lb = EEPROM.read(freeEepromOffset + EEPROM_ADR_ZCV_LB);
		int hb = EEPROM.read(freeEepromOffset + EEPROM_ADR_ZCV_HB);
		int value = lb + hb*0x100;
		if(value >=0 && value < 1023)
			return value;
		else
			return -2;
	}
	else
		return -1;
}

int WriteZeroCurrentValueToEEPROM(int value)
{
	if(value < 0)
		return -1;
	
	int oldvalue = GetZeroCurrentValueFromEEPROM;
	if(oldvalue > 0)
	{
		int minchange = param_drivecurrent_jit / 3;
		int diff;
		if(oldvalue > value)
			diff = oldvalue - value;
		else
			diff = value - oldvalue;
		if(diff <= minchange)
		{
			return 1;
		}
	}
	if(eepromwritecnt<7200)
	{
		return -2;
	}
	
	int freeEepromOffset = Konnekting.getFreeEepromOffset();
	int lb = value % 0x100;
	int hb = value / 0x100;
	
	EEPROM.update(freeEepromOffset + EEPROM_ADR_ZCV_LB, lb);
	EEPROM.update(freeEepromOffset + EEPROM_ADR_ZCV_HB, hb);
	eepromwritecnt = 0;
	return 0;
}

void ErrorCode(unsigned short errorid)
{
	Knx.write(COMOBJ_error_code, true);
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

