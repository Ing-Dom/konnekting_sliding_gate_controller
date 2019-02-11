/* Changelog 07:
- send_status_cyclic_cnt changed +1 to other side (1min was 1:20 => 8 instead of 6 cycles)
- knxEvents COMOBJ_cmd_stop: removed ! for moving bits
- T4: glob_last_command_valid-- only when >0 (underflow..)
- T1: removed !glob_last_command_valid from if param_close_ext
- T1 AutoClose: glob_AutoClose_Cnt2Close = param_close_time_nocontact * 6; //(60000 / T5_CYCLETIME); changed to fix 6 due to possible overflows....
- T1 AutoClose: only set globs when correct param is enabled
- setup: added type casts for all params

Changelog 08:
- T1 AutoClose: Added Debug Info
- setup(): Reading Parameters, fixed bug, read correct parameters for autoclose
- Open() ans Close(): Added open and close from intermediate position when last direction was oposite

Changelog 09:
- finished feature start from middle pos in any case
- T1 start autoclose bugfix for leave open not working when external command is enabled
- Position reporting
- Go To Position // in Work ToDo but testable

Changelog 10:
- Bugfix in Pos calculation when opening
- send more positions 5% instaed of 10%
- stop earlier when going to pos
- cancel position monitoriung when knx command comes in

ToDo:



*/




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
unsigned int param_close_time_contact = 0; // in sec
unsigned int param_close_time_nocontact = 0; // in min
bool param_close_ext = false;

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
bool glob_last_moving_direction = false;

#define OPENING false
#define CLOSING true




// Start AutoClose

bool glob_AutoClose_Wait4Barrier = false;
unsigned int glob_AutoClose_Cnt2CloseAfterBarrier = 0;
unsigned int glob_AutoClose_Cnt2Close = 0;

#define GLOB_LAST_COMMAND_VALID_START 5
unsigned int glob_last_command = 0;
unsigned int glob_last_command_valid = 0; // last command is valid when >0

// End AutoClose

// Start Position Detection
#define POSITION_SEND_DISTANCE 13 // ca. 5%
double glob_position = -1; // 0 = opened, 255 = closed
double glob_position_step100ms = 256.0 / 300; // ToDo make this a param
double glob_position_sent = -1;
// End Position Detection

// Start GoTo Position
#define POSITION_OFFSET_STOP 12
int glob_goto_target_position = -1;
int glob_goto_start_position = -1;
// End GoTo Position 


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
    {
      if (Knx.read(COMOBJ_cmd_close))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on cmd_close"));
        #endif
    		glob_last_command = index;
    		glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
        glob_goto_target_position = -1;
        Close();
      }
      break;
    }
    case COMOBJ_cmd_open:
    {
      if (Knx.read(COMOBJ_cmd_open))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on cmd_open"));
        #endif
				glob_last_command = index;
				glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
        glob_goto_target_position = -1;
        Open(false);
      }
      break;
    }
    case COMOBJ_cmd_leave_open:
    {
      if (Knx.read(COMOBJ_cmd_leave_open))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on COMOBJ_cmd_leave_open"));
        #endif
				glob_last_command = index;
				glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
        glob_goto_target_position = -1;
				Open(true);
      }
      break;
    }
    case COMOBJ_cmd_partly_open:
    {
      if (Knx.read(COMOBJ_cmd_partly_open))
      {
        Debug.println(F("Received 1 on COMOBJ_cmd_partly_open"));
				glob_last_command = index;
				glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
        glob_goto_target_position = -1;
        out_openpart_cnt = 20; // 20x 25ms = 500ms set OUT_OPENPART to 1 for 500ms
				//ToDo: use OpenPart()
      }
      break;
    }
    case COMOBJ_cmd_stop:
    {
      if (Knx.read(COMOBJ_cmd_stop))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on COMOBJ_cmd_stop"));
        #endif
				glob_last_command = index;
				glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
        glob_goto_target_position = -1;
				Stop();
      }
      break;
    }
    case COMOBJ_cmd_position:
    {
      #ifdef KDEBUG
      Debug.println(F("Received %u on COMOBJ_cmd_position"),Knx.read(COMOBJ_cmd_position));
      #endif
      glob_last_command = index;
      glob_last_command_valid = GLOB_LAST_COMMAND_VALID_START;
      glob_goto_target_position = -1;
			MoveToPosition(Knx.read(COMOBJ_cmd_position));
      break;
    }
    case COMOBJ_test1:
    {
      if (Knx.read(COMOBJ_test1))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on COMOBJ_test1"));
        #endif
        test1 = true;
      }
      else
      {
        #ifdef KDEBUG
        Debug.println(F("Received 0 on COMOBJ_test1"));
        #endif
        test1 = false;
      }
      break;
    }
    case COMOBJ_test2:
    {
      if (Knx.read(COMOBJ_test2))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on COMOBJ_test2"));
        #endif
        test2 = true;
      }
      else
      {
        #ifdef KDEBUG
        Debug.println(F("Received 0 on COMOBJ_test2"));
        #endif
        test2 = false;
      }
      break;
    }
    case COMOBJ_test3:
    {
      if (Knx.read(COMOBJ_test3))
      {
        #ifdef KDEBUG
        Debug.println(F("Received 1 on COMOBJ_test3"));
        #endif
        out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENPART to 1 for 500ms
      }
      break;
    }
    default:
    {
      break;
    }
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
	  param_send_status_cyclic = (unsigned short)(Konnekting.getUINT8Param(PARAM_send_status_cyclic));
    param_drivecurrent_zero = (int)(Konnekting.getUINT16Param(PARAM_drivecurrent_zero));
    param_drivecurrent_jit = (int)(Konnekting.getUINT16Param(PARAM_drivecurrent_jit));
    param_drivecurrent_num = (int)(Konnekting.getUINT16Param(PARAM_drivecurrent_num));
  	param_drivecurrent_autozero = (bool)Konnekting.getUINT8Param(PARAM_drivecurrent_autozero);
  	param_close_time_contact = (unsigned int)Konnekting.getUINT8Param(PARAM_close_time_contact);
  	param_close_time_nocontact = (unsigned int)Konnekting.getUINT8Param(PARAM_close_time_nocontact);
  	param_close_ext = (bool)Konnekting.getUINT8Param(PARAM_close_ext);
	
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
  	
  	T1_last_run = T2_last_run = T3_last_run = T4_last_run = T5_last_run = millis();  // start well-defined into the time slices.. (init can take long)
  }
  #ifdef KDEBUG
  Debug.println(F("Setup is ready. go to loop..."));
  #endif
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
  
  in_closed = !digitalRead(IN_CLOSED);
  in_opened = !digitalRead(IN_OPENED);
  in_barrier = !digitalRead(IN_BARRIER);
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
  // Start Handling of Inputs
  // open and close and barrier are Low-active signals
  if(in_closed != !(debouncer_in_closed.read()))
  {
    in_closed = !(debouncer_in_closed.read());
    Knx.write(COMOBJ_stat_closed, in_closed);
    #ifdef KDEBUG
    Debug.println(F("inclosed changed: %d"), in_closed );
    #endif

    // Start AutoClose
  	if(!in_closed)
  	{
  	  // detected opening action
  		if((param_close_ext && !glob_last_command_valid) ||
  		(glob_last_command_valid && (glob_last_command == COMOBJ_cmd_open || glob_last_command == COMOBJ_cmd_partly_open || glob_last_command == COMOBJ_cmd_position))) //// when there was no command (=external opening) and the parameter for closing on external command is set or when last command is valid and was open, openpart or position 
  		{
        if(param_close_time_contact)
    	  glob_AutoClose_Wait4Barrier = true;
        if(param_close_time_nocontact)
    	  glob_AutoClose_Cnt2Close = param_close_time_nocontact * (6000 / (T5_CYCLETIME/10));
        ErrorCode(20,0, glob_AutoClose_Cnt2Close);
        ErrorCode(20,1, param_close_time_nocontact);
  	  }
  	}
    // End AutoClose
  }

  if(in_opened != !(debouncer_in_opened.read()))
  {
    in_opened = !(debouncer_in_opened.read());
    Knx.write(COMOBJ_stat_opened, in_opened);
    #ifdef KDEBUG
    Debug.println(F("inopened changed: %d"), in_opened );
    #endif
  }

  if(in_barrier != !(debouncer_in_barrier.read()))
  {
    in_barrier = !(debouncer_in_barrier.read());
    Knx.write(COMOBJ_stat_barrier, in_barrier);
    #ifdef KDEBUG
    Debug.println(F("inbarrier changed: %d"), in_barrier );
    #endif
  
  	// Start AutoClose
  	if(!in_barrier && glob_AutoClose_Wait4Barrier)
  	{
  		glob_AutoClose_Cnt2CloseAfterBarrier = param_close_time_contact * (1000 / T4_CYCLETIME);
  		glob_AutoClose_Cnt2Close = 0;
  	}
    // End AutoClose
  }
  // End Handling of Inputs

  // Start Position Detection
  if(in_opened)
    glob_position = 0;
  else if(in_closed)
    glob_position = 255;
  // End Position Detection
  
  // Start Handling of Outputs
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
  // End Handling of Outputs
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
	  SendPosition();
    }
    else if(closing_cnt > param_drivecurrent_num)
    {
      moving_closing = true;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving_direction, true);
      glob_last_moving_direction = CLOSING;
	  SendPosition();
    }
  }
  else if(moving_closing)
  {
    if(opening_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = true;
      Knx.write(COMOBJ_stat_moving_direction, false);
      glob_last_moving_direction = OPENING;
	  SendPosition();
    }
    else if(stopped_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving, false);
	  SendPosition();
    }
  }
  else
  {
    if(opening_cnt > param_drivecurrent_num)
    {
      moving_closing = false;
      moving_opening = true;
      Knx.write(COMOBJ_stat_moving_direction, false);
      glob_last_moving_direction = OPENING;
      Knx.write(COMOBJ_stat_moving, true);
	  SendPosition();
    }
    else if(closing_cnt > param_drivecurrent_num)
    {
      moving_closing = true;
      moving_opening = false;
      Knx.write(COMOBJ_stat_moving_direction, true);
      glob_last_moving_direction = CLOSING;
      Knx.write(COMOBJ_stat_moving, true);
	  SendPosition();
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

  MovingCheck_Cyclic();

  // Start Position Calculation
  if(!in_opened && !in_closed)
  {
    if(moving_closing && glob_position < 252)
    {
      glob_position = glob_position + glob_position_step100ms;
    }
    else if(moving_opening && glob_position > 3)
    {
      glob_position = glob_position - glob_position_step100ms;
    }
  }
  SendPositionOnDemand();
  // End Position Calculation

  // Start Position Monitoring
  if(glob_goto_target_position >= 0) // position monitoring active
  {
    if(glob_goto_target_position > glob_goto_start_position) // closing
    {
      if(moving_closing)
      {
        if(glob_position >= glob_goto_target_position - POSITION_OFFSET_STOP)
        {
          glob_goto_target_position = -1;
          Stop();
        }
      }
    }
    else // opening
    {
      if(moving_opening)
      {
        if(glob_position <= glob_goto_target_position + POSITION_OFFSET_STOP)
        {
          glob_goto_target_position = -1;
          Stop();
        }
      }
    }
  }
  // End Position Monitoring
  
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
bool firstrunT4 = true;
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
		  int retval = WriteZeroCurrentValueToEEPROM(zero_current_value);
		  switch(retval)
		  {
			  case  0: ErrorCode(ERROR_ZCV_New_written_to_EEPROM, 0, zero_current_value);
			  case  1: ErrorCode(ERROR_ZCV_New_Minor_change, 0, zero_current_value);
			  case -1: ErrorCode(ERROR_ZCV_Invalid, 0, zero_current_value);
			  case -2: ErrorCode(ERROR_ZCV_New_EEPROM_Cooldown, 0, zero_current_value);
		  }
		}
		
		if(zero_current_values_pointer == 29)
		  zero_current_values_pointer = 0;
		else
		  zero_current_values_pointer++;
	  }
  }
  // End Feature Auto Adaption of Zero Current Value
  
  // Start Send Status once at Startup
  if(firstrunT4)
  {
    Knx.write(COMOBJ_stat_barrier, in_barrier);
	  Knx.write(COMOBJ_stat_closed, in_closed);
	  Knx.write(COMOBJ_stat_opened, in_opened);
	  firstrunT4 = false;
  }
  // End Send Status once at Startup
  
  // Start AutoClose
  if(glob_last_command_valid > 0)
    glob_last_command_valid--;
  if(param_close_time_contact || param_close_time_nocontact)
  {
	  if(glob_AutoClose_Cnt2CloseAfterBarrier > 0)
	  {
		  glob_AutoClose_Cnt2CloseAfterBarrier--;
		  if(glob_AutoClose_Cnt2CloseAfterBarrier == 0)
		  {
			glob_AutoClose_Cnt2Close = 0;
			Close();
		  }
	  }
  }
  // End AutoClose
  
}

int send_status_cyclic_cnt = 0;
void T5() // 10000ms = 10s
{
  Knx.write(COMOBJ_debug5, in_current_smoothed);
  Knx.write(COMOBJ_debug4, zero_current_value);
  //Debug.println(F("in_current_smoothed: %d"), in_current_smoothed );
  
  // Start Cyclic Sending of Status
  if(param_send_status_cyclic)
  {
	  if(send_status_cyclic_cnt+1 >= (((60*100) / (T5_CYCLETIME/10)) * param_send_status_cyclic)) // send every param_send_status_cyclic minutes
	  {
		Knx.write(COMOBJ_stat_closed, in_closed);
		Knx.write(COMOBJ_stat_opened, in_opened);
		Knx.write(COMOBJ_stat_barrier, in_barrier);
		Knx.write(COMOBJ_stat_moving, moving_closing || moving_opening);
		SendPosition();
		send_status_cyclic_cnt = 0;
	  }
	  else
		send_status_cyclic_cnt++;
  }
  
  // Start AutoClose
  if(param_close_time_contact || param_close_time_nocontact)
  {
	  if(glob_AutoClose_Cnt2Close > 0)
	  {
		  glob_AutoClose_Cnt2Close--;
		  if(glob_AutoClose_Cnt2Close == 0)
		  {
			glob_AutoClose_Cnt2CloseAfterBarrier = 0;
			Close();
		  }
	  }
  }
  // End AutoClose
}

void Close()
{
	glob_AutoClose_Wait4Barrier = false;
	glob_AutoClose_Cnt2Close = 0;
	glob_AutoClose_Cnt2CloseAfterBarrier = 0;
	
	if(in_opened)
	{
		out_openclose_cnt = 500 / T1_CYCLETIME; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
	}
	else if(!in_closed && !moving_closing)
	{
    out_openclose_cnt = 500 / T1_CYCLETIME; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
    MovingCheck_Activate(MOVESTATE_CLOSE);
	}
}

void Open(bool leave_open)
{
	if(leave_open)
	{
		glob_AutoClose_Wait4Barrier = false;
		glob_AutoClose_Cnt2Close = 0;
		glob_AutoClose_Cnt2CloseAfterBarrier = 0;
	}
	
	if(in_closed)
	{
		out_openclose_cnt = 500 / T1_CYCLETIME; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
	}
	else if(!in_opened && !moving_opening)
	{
    out_openclose_cnt = 500 / T1_CYCLETIME; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
    MovingCheck_Activate(MOVESTATE_OPEN);
	}
}

void Stop()
{
  if(!in_closed && !in_opened && (moving_opening || moving_closing))
  {
    out_openclose_cnt = 20; // 20x 25ms = 500ms set OUT_OPENCLOSE to 1 for 500ms
  }
}


unsigned short glob_MovingCheck_timer = 0;
MoveState glob_MovingCheck_MoveStateFrom = 0;
MoveState glob_MovingCheck_MoveStateTo = 0;
#define MOVINGCHECK_TIMER_TICKS 20

void MovingCheck_Activate(MoveState arg_MoveState)
{
	glob_MovingCheck_MoveStateFrom = GetMoveState();
	if(glob_MovingCheck_MoveStateFrom != arg_MoveState)
	{
		glob_MovingCheck_timer = MOVINGCHECK_TIMER_TICKS;
		glob_MovingCheck_MoveStateTo = arg_MoveState;
	}
}

void MovingCheck_Cyclic()
{
	if(glob_MovingCheck_timer > 0)
	{
		glob_MovingCheck_timer--;
		if(glob_MovingCheck_timer == 0)
		{
			// Timer expired, check if to state is reached
			MovingCheck_Retrigger();
    }
    else
    {
      if(GetMoveState() != glob_MovingCheck_MoveStateFrom) // there has been a state change
      {
        // state changed, look it is the right one
        MovingCheck_Retrigger();
      }
    }
  }
}

void MoveToPosition(unsigned short knx_position)
{
	if(knx_position == 0)
		Open(false);
	else if (knx_position == 255)
		Close();
	else if(glob_position > -1) // dont move when position is unknown
	{
		// check if diff is big enough
		// decide if Open() oder Close() has to be choosen
		double diff = glob_position - knx_position;
		if(diff > 15)
		{
			Open(false);
		}
		else if (diff < -15)
		{
			Close();
		}
		//activate monitoring
		glob_goto_target_position = knx_position;
    glob_goto_start_position = glob_position;
		

		
		//ToDo somewhere else:
		// cancel monitoring when external command comes in
	}		
	return;
}


void MovingCheck_Retrigger()
{
  //deativate
  glob_MovingCheck_timer = 0;
  
  if(GetMoveState() != glob_MovingCheck_MoveStateTo)
  {
    switch (glob_MovingCheck_MoveStateTo)
    {
      case MOVESTATE_OPEN:
        Open(false); // if it was leave open, then the first execution will already have deleted running closing timers, so it doesnt matter..
        break;
      case MOVESTATE_CLOSE:
        Close(); // if it was leave open, then the first execution will already have deleted running closing timers, so it doesnt matter..
        break;
      case MOVESTATE_STOP: // this is dead, because Stop will never need to activate a moving check ?
        Stop(); // if it was leave open, then the first execution will already have deleted running closing timers, so it doesnt matter..
        break;
    }
  }
}

MoveState GetMoveState()
{
	if(moving_opening)
		return MOVESTATE_OPEN;
	else if(moving_closing)
		return MOVESTATE_CLOSE;
	else
		return MOVESTATE_STOP;
}

void SendPosition()
{
	if((glob_position - ((int)glob_position) ) > 0.5)
	{
		Knx.write(COMOBJ_stat_position, (int)glob_position+1);
	}
	else
	{
		Knx.write(COMOBJ_stat_position, (int)glob_position);
	}
	
	glob_position_sent = glob_position;
	return;
}

void SendPositionOnDemand()
{
	double diff = glob_position - glob_position_sent;
	if(diff > POSITION_SEND_DISTANCE || diff < -POSITION_SEND_DISTANCE)
	{
		SendPosition();
	}
	return;
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
	
	int oldvalue = GetZeroCurrentValueFromEEPROM();
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



void ErrorCode(ErrorID errorid, unsigned short info, unsigned int data)
{
	unsigned long sendvalue = 0;
	sendvalue = errorid;
	sendvalue *= 0x1000000;	
	sendvalue += ((unsigned long)info)*0x10000;	
	sendvalue += data;
	
	Knx.write(COMOBJ_error_code, sendvalue);
}
