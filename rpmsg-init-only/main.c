/***************************************************************************************
 * MAIN.C
 *
 * Description: Driver for PRU1 - THS1206
 *
 * (C) 2016 Visaoni
 * Licensed under the MIT License.
 **************************************************************************************/


/*
Results:
This doesn't lock up the board, unlike when the contents of SR_init() are copied in.
It also still doesn't work.
*/



#include <stdint.h>
//#include "resource_table_1.h"  // Conflicts at link with #include in store_readings.c

//#include "ths1206_control.h"
#include "store_readings.h"

#include "pin_control.h"
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>


#define READS_PER_ROUND 2000

int main(void)
{
   clear_pin(0);
   clear_pin(1);

   __delay_cycles(100000000);


  SR_init();
  set_pin(0);
//   TC_init();

  set_pin(1);


//   while(1)
//   {
////      TC_store_next_n_reads( READS_PER_ROUND );
////      set_pin(0);
////      SR_transfer_readings();
////      set_pin(1);
//   }

  __halt();

   /* Should never return */
   return 0;
}
