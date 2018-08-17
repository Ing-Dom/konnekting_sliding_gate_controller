// ################################################
// ### DEBUG CONFIGURATION
// ################################################
#define KDEBUG // comment this line to disable DEBUG mode
#ifdef KDEBUG
#include <DebugUtil.h>
#define APPDEBUG
#endif

// ################################################
// ### KONNEKTING Configuration
// ################################################

// Nano & Pro Mini
#include <SoftwareSerial.h>
SoftwareSerial softserial(6, 7); // RX, TX
#define DEBUGSERIAL softserial
#define KNX_SERIAL Serial


/*
// Micro & Pro Micro
#define DEBUGSERIAL Serial
#define KNX_SERIAL Serial1 // Leonardo/Micro etc. use Serial1
*/

/*
// Mega2560
#define KNX_SERIAL Serial2
#define DEBUGSERIAL Serial //(USB)
*/

// ################################################
// ### IO Configuration
// ################################################

//ProMini
#define PROG_LED_PIN 13
#define PROG_BUTTON_PIN 2


/*
// Micro
#define PROG_LED_PIN 13 //(32/PC7/D13)
#define PROG_BUTTON_PIN 2 //(19/PD1/D2)
#define OUTPIN_A 9 //(29/PB5/D9)
#define OUTPIN_B 10 //(30/PB6/D10)
#define OUTPIN_C 11 //(12/PB7/D11)
#define OUTPIN_D 5 //(31/PC6/D5)
*/

/*
// ProMicro
#define PROG_LED_PIN 3 //(/P/D3)
#define PROG_BUTTON_PIN 2 //(19/PD1/D2)
#define OUTPIN_A 9 //(29/PB5/D9)
#define OUTPIN_B 10 //(30/PB6/D10)
#define OUTPIN_C 11 //(12/PB7/D11)
#define OUTPIN_D 5 //(31/PC6/D5)
*/

/*
// Mega 2560
#define PROG_LED_PIN 4      //(1/PG5)
#define PROG_BUTTON_PIN 18  //(46/PD3)
#define ONE_WIRE_BUS 9 
*/



// ################################################
// ### Global variables, sketch related
// ################################################

#define IN_CLOSED   10
#define IN_OPENED   11
#define IN_BARRIER  12
#define IN_CURRENT  A0

#define OUT_OPENCLOSE 8
#define OUT_OPENPART  9

