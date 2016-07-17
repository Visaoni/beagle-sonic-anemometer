/*
 * pin_control.h
 *
 * Description:
 *
 * (C) 2016 Visaoni
 * Licensed under the MIT License.
 */

// TODO: Get rid of this duplication nonsense with PRU0/1

#ifndef PIN_CONTROL_H_
#define PIN_CONTROL_H_

#include <stdint.h>

volatile register uint32_t __R30;
volatile register uint32_t __R31;

//inline void set_pin( uint32_t bit );
//
//inline void clear_pin( uint32_t bit );
//
//// TODO: Convert to 0/1?
//inline uint32_t read_pin( uint32_t bit );
//
//inline void write_reg( uint32_t value );
//
//inline uint32_t read_reg();

#define set_pin( bit ) ( __R30 |= (uint32_t)1 << bit )
#define clear_pin( bit ) ( __R30 &= (uint32_t)~(1 << bit ))
#define read_pin( bit ) ( __R31 & (uint32_t)(1 << bit) )
#define write_reg( value ) ( __R30 = value )
#define read_reg() ( __R31 )

#endif /* PIN_CONTROL_H_ */
