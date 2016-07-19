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
works!
*/


/*
Notes:
stdio.h doesn't matter
buf location doesn't matter
BUFFER OF SIZE 4 is BAD
*/


#include <stdint.h>
#include <stdio.h>
//#include "resource_table_1.h"  // Conflicts at link with #include in store_readings.c

//#include "ths1206_control.h"
//#include "store_readings.h"

// #include "pin_control.h"
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include "resource_table_1.h"

#define SR_MAX_BUFFER_SIZE 1750

volatile register uint32_t __R31;
volatile register uint32_t __R30;

// Defines mostly lifted from pru-support-package lab5
#define HOST_INT        ((uint32_t) 1 << 31)

#define TO_ARM_HOST     18
#define FROM_ARM_HOST   19
#define CHAN_NAME       "rpmsg-pru"
#define CHAN_DESC       "Channel 31"
#define CHAN_PORT       31

#define VIRTIO_CONFIG_S_DRIVER_OK   4
// End lifted defines

// TODO: Figure out best size limit
// Looks like 512B is the total size including header
// Header size uncertain - might be 16B?
#define MAX_ELEMENTS_TO_TRANSFER 100

//static uint32_t buffer[ SR_MAX_BUFFER_SIZE ];
//static size_t head = 0;

#define MSG_LEN 400
char xmsg[MSG_LEN];

uint8_t dummy_buffer[ RPMSG_BUF_SIZE ];

#define READS_PER_ROUND 2000

int main(void)
{
   int x;
   for( x = 0; x < MSG_LEN; x++ )
   {
      xmsg[x] = 'A';
   }

   struct pru_rpmsg_transport transport;
   uint16_t src, dst;
   // clear_pin(0);
   // clear_pin(1);
   __R30 = 0;

   __delay_cycles(100000000);

   volatile uint8_t* status;

   
   uint16_t dummy_len;

   // Enable OCP master port access for the PRU
   CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

   // Clear incoming message flag
   CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

   // Wait for linux driver ready
   status = &resourceTable.rpmsg_vdev.status;
   while(!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

   pru_rpmsg_init( &transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST );

   // Create RPMsg channel
   while( pru_rpmsg_channel( RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS );

   // Wait for incoming message to grab src and dst
//   while( (!read_pin(HOST_INT))
//         && (pru_rpmsg_receive( &transport, &dst, &src, dummy_buffer, &dummy_len ) == PRU_RPMSG_SUCCESS) );

//   set_pin(1);
   __R30 = 1;
   while(1)
   {
      if( __R31 & HOST_INT )
      {
         CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

         if( pru_rpmsg_receive( &transport, &src, &dst, dummy_buffer, &dummy_len ) == PRU_RPMSG_SUCCESS )
         {
            break;
         }
      }
   }
   // set_pin(0);
   __R30 = 2;

   while( pru_rpmsg_send(&transport, dst, src, xmsg, sizeof xmsg) != PRU_RPMSG_SUCCESS )
   {
      __delay_cycles(1000000);
   }
//   pru_rpmsg_send(&transport, dst, src, xmsg, sizeof xmsg);
   // set_pin(1);

//   while(1);

  __halt();

//   SR_init();
//   set_pin(0);
//   TC_init();

//   set_pin(1);


//   while(1)
//   {
////      TC_store_next_n_reads( READS_PER_ROUND );
////      set_pin(0);
////      SR_transfer_readings();
////      set_pin(1);
//   }

//   __halt();

   /* Should never return */
   return 0;
}
