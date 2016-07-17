/*
* store_readings.c
*
* Description:
*
* (C) 2016 Visaoni
* Licensed under the MIT License.
*/


#include "store_readings.h"

//#include <pru_cfg.h>
//#include <pru_intc.h>
//#include <rsc_types.h>
//#include <pru_rpmsg.h>
//#include "resource_table_1.h"

#define SR_MAX_BUFFER_SIZE 1750

#include "pin_control.h"

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

static uint32_t buffer[ SR_MAX_BUFFER_SIZE ];
static size_t head = 0;

static struct pru_rpmsg_transport transport;
static uint16_t src, dst;

uint8_t xmsg[] = {0x41};

void SR_init()
{
  volatile uint8_t* status;

  uint8_t dummy_buffer[ RPMSG_BUF_SIZE ];
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
  while(1)
  {
     if( read_pin( 31 ) )
     {
        CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

        if( pru_rpmsg_receive( &transport, &dst, &src, dummy_buffer, &dummy_len ) == PRU_RPMSG_SUCCESS )
        {
           break;
        }
     }
  }
  set_pin(0);

//   while( pru_rpmsg_send(&transport, dst, src, xmsg, sizeof xmsg) != PRU_RPMSG_SUCCESS );
  pru_rpmsg_send(&transport, dst, src, xmsg, sizeof xmsg);
  set_pin(1);

  while(1);
}

uint32_t SR_store( uint32_t reading )
{
  // TODO: Added in shared RAM too? Currently using just 7KB of PRU1 RAM
  // Limited size is a problem. Even with all of PRU1 RAM and Shared PRU RAM only ~5k readings
  // Max coverage of ~0.8 ms, rough estimate of average tof is ~1.5ms
  // Current coverage is just under 0.3ms with 1750 readings
  // Will need to delay recording

  if( head < SR_MAX_BUFFER_SIZE )
  {
     buffer[head++] = 0x41424344;
     return 0;
  }
  return 1;   // Buffer full
}

void SR_transfer_readings()
{
  if( head <= 0 ) return;

//   size_t i = 0;
//   while( i < head )
//   {
//      uint16_t len = (i + MAX_ELEMENTS_TO_TRANSFER > head ? head - i : MAX_ELEMENTS_TO_TRANSFER) * sizeof buffer[0];
//
//      // TODO: Probably ignores some conditions that can't be fixed by waiting
//      while( pru_rpmsg_send( &transport, src, dst, buffer + i, len ) != PRU_RPMSG_SUCCESS );
//
//      i += MAX_ELEMENTS_TO_TRANSFER;
//
//      __delay_cycles( 1000000 );
//   }
//
//   __delay_cycles( 1000000 );

  size_t pi, pe;
  for( pi = 0, pe = MAX_ELEMENTS_TO_TRANSFER; pi < SR_MAX_BUFFER_SIZE; pi += MAX_ELEMENTS_TO_TRANSFER, pe += MAX_ELEMENTS_TO_TRANSFER )
  {
     uint16_t len = ((pe < SR_MAX_BUFFER_SIZE ? pe : SR_MAX_BUFFER_SIZE) - pi) * sizeof buffer[0];
     while( pru_rpmsg_send(&transport, dst, src, buffer + pi, len) != PRU_RPMSG_SUCCESS );
     {
        __delay_cycles(1000000);
     }
     __delay_cycles(1000000);
  }

  head = 0;
  return;
}


#undef HOST_INT
#undef TO_ARM_HOST
#undef CHAN_NAME
#undef CHAN_DESC
#undef CHAN_PORT
#undef VIRTIO_CONFIG_S_DRIVER_OK
#undef MAX_ELEMENTS_TO_TRANSFER
