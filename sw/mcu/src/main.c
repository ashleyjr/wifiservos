//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "SI_C8051F850_Register_Enums.h"
#include "SI_C8051F850_Defs.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

#define TIMER_50HZ   1000
#define FALL_MIN     50 
#define FALL_MAX     110
#define FALL_MID     ((FALL_MAX + FALL_MIN)/2) 
#define FALL_STEP    5

SBIT(SRV0, SFR_P1, 0);  
SBIT(SRV1, SFR_P1, 1);  
SBIT(SRV2, SFR_P1, 2);  
SBIT(SRV3, SFR_P1, 3);  

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

volatile U16 timer;
volatile U16 fall_srv0;
volatile U16 fall_srv1;
volatile U16 fall_srv2;
volatile U16 fall_srv3;

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------

void main (void){    
  //--------------------------------------------------------------------------
   // Initialise
   //--------------------------------------------------------------------------
   timer = 0;
   
   //--------------------------------------------------------------------------
   // Setup micro
   //--------------------------------------------------------------------------
   
   // Disabled watchdog
   WDTCN    = 0xDE;
   WDTCN    = 0xAD;
   // Clock
	CLKSEL   = CLKSEL_CLKSL__HFOSC 	      |     // Use 24.5MHz interal clock
			     CLKSEL_CLKDIV__SYSCLK_DIV_1;      // Do not divide       
   // Setup XBAR         
   P0MDOUT  = P0MDOUT_B4__PUSH_PULL;            // UART output
   P1MDOUT  = P1MDOUT_B0__PUSH_PULL|            // Servo 0         
              P1MDOUT_B1__PUSH_PULL;            // Servo 1
              P1MDOUT_B2__PUSH_PULL|            // Servo 2            
              P1MDOUT_B3__PUSH_PULL;            // Servo 3
   XBR0     = XBR0_URT0E__ENABLED;              // Route out UART P0.4 
   XBR2     = XBR2_XBARE__ENABLED;
   // UART
	SCON0    |= SCON0_REN__RECEIVE_ENABLED;      // UART rx 
   // Timer 1: UART baud gen 115200 
	CKCON    |= CKCON_T1M__SYSCLK;
	TMOD     |= TMOD_T1M__MODE2;
	TCON     |= TCON_TR1__RUN;  
   TH1      = 0x96;                             // Magic values from datasheet for 115200
	TL1      = 0x96;
   // Timer 2: Counter 10KHz
	TMR2CN   = TMR2CN_TR2__RUN;
   TMR2L    = 0xD7;
   TMR2H    = 0xFF;
   TMR2RLL  = 0xD7;
   TMR2RLH  = 0xFF;
   // Interrupts
   IE       = IE_EA__ENABLED | 
              IE_ET2__ENABLED;
 
   //--------------------------------------------------------------------------
   // Main loop
   //--------------------------------------------------------------------------
   fall_srv0 = FALL_MID;
   fall_srv1 = FALL_MID;
   fall_srv2 = FALL_MID;
   fall_srv3 = FALL_MID;
   while(1){

      if(SCON0_RI){ 
         switch(SBUF0){ 
            // SRV0
            case 'q':   fall_srv0 += FALL_STEP; 
                        if(fall_srv0 > FALL_MAX)  
                           fall_srv0 = FALL_MAX; 
                        break;
            case 'a':   fall_srv0 -= FALL_STEP;
                        if(fall_srv0 < FALL_MIN)  
                           fall_srv0 = FALL_MIN; 
                        break;
            // SRV1
            case 'w':   fall_srv1 += FALL_STEP;
                        if(fall_srv1 > FALL_MAX)  
                           fall_srv1 = FALL_MAX; 
                        break;
            case 's':   fall_srv1 -= FALL_STEP;
                        if(fall_srv1 < FALL_MIN)  
                           fall_srv1 = FALL_MIN; 
                        break;
            // SRV2
            case 'e':   fall_srv2 += FALL_STEP;
                        if(fall_srv2 > FALL_MAX)  
                           fall_srv2 = FALL_MAX; 
                        break;
            case 'd':   fall_srv2 -= FALL_STEP;
                        if(fall_srv2 < FALL_MIN)  
                           fall_srv2 = FALL_MIN; 
                        break;
            // SRV3
            case 'r':   fall_srv3 += FALL_STEP;
                        if(fall_srv3 > FALL_MAX)  
                           fall_srv3 = FALL_MAX; 
                        break;
            case 'f':   fall_srv3 -= FALL_STEP;
                        if(fall_srv3 < FALL_MIN)  
                           fall_srv3 = FALL_MIN; 
                        break; 
         }
         SCON0_RI = 0;
      }
   }
}

//-----------------------------------------------------------------------------
// Interrupts
//-----------------------------------------------------------------------------

INTERRUPT (TIMER2_ISR, TIMER2_IRQn){            
   timer++; 
   timer %= TIMER_50HZ; 
   SRV0 = (fall_srv0 >= timer) ? 1 : 0;
   SRV1 = (fall_srv1 >= timer) ? 1 : 0;
   SRV2 = (fall_srv2 >= timer) ? 1 : 0;
   SRV3 = (fall_srv3 >= timer) ? 1 : 0;
   TMR2CN &= ~TMR2CN_TF2H__SET;
}


