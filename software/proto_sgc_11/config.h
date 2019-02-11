// ################################################
// ### DEBUG CONFIGURATION
// ################################################
//#define KDEBUG // comment this line to disable DEBUG mode
#ifdef KDEBUG
#include <DebugUtil.h>
#define APPDEBUG
#endif

// ################################################
// ### KONNEKTING Configuration
// ################################################

/*
// Nano & Pro Mini
#include <SoftwareSerial.h>
SoftwareSerial softserial(6, 7); // RX, TX
#define DEBUGSERIAL softserial
#define KNX_SERIAL Serial
*/


// Micro & Pro Micro
#define DEBUGSERIAL Serial
#define KNX_SERIAL Serial1 // Leonardo/Micro etc. use Serial1


/*
// Mega2560
#define KNX_SERIAL Serial2
#define DEBUGSERIAL Serial //(USB)
*/

// ################################################
// ### IO Configuration
// ################################################

/*
//ProMini
#define PROG_LED_PIN 13
#define PROG_BUTTON_PIN 2
*/


// Micro
#define PROG_LED_PIN 13 //(32/PC7/D13)
#define PROG_BUTTON_PIN 2 //(19/PD1/D2)


/*
// ProMicro
#define PROG_LED_PIN 3 //(/P/D3)
#define PROG_BUTTON_PIN 2 //(19/PD1/D2)
*/

/*
// Mega 2560
#define PROG_LED_PIN 4      //(1/PG5)
#define PROG_BUTTON_PIN 18  //(46/PD3)
*/



// ################################################
// ### Global variables, sketch related
// ################################################

#define IN_CLOSED   7
#define IN_OPENED   8
#define IN_BARRIER  9
#define IN_CURRENT  A0

#define OUT_OPENCLOSE 10
#define OUT_OPENPART  16


enum ErrorID
{
  ERROR_Reserved = 0x00,
  ERROR_ZCV_Invalid = 0x01,
  ERROR_ZCV_New_written_to_EEPROM = 0x02,
  ERROR_ZCV_New_Minor_change = 0x03,
  ERROR_ZCV_New_EEPROM_Cooldown = 0x04
};

enum MoveState
{
  MOVESTATE_Reserved = 0x00,
  MOVESTATE_OPEN = 0x01,
  MOVESTATE_CLOSE = 0x02,
  MOVESTATE_STOP = 0x03
};
