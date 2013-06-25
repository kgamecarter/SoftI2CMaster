/* Arduino SoftI2C library. 
 *
 * This is a very fast and very light-weight software I2C-master library 
 * written in assembler. It is based on Peter Fleury's I2C software
 * library: http://homepage.hispeed.ch/peterfleury/avr-software.html
 *
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* In order to use the libraray, you need to define SDA_PIN, SCL_PIN,
 * SDA_PORT and SCL_PORT before including this file.  Have a look at
 * http://www.arduino.cc/en/Reference/PortManipulation for finding out
 * which values to use. For example, if you use digital pin 3 for
 * SDA and digital pin 13 for SCL you have to use the following
 * definitions: 
 * #define SDA_PIN 3 
 * #define SDA_PORT PORTB 
 * #define SCL_PIN 5
 * #define SCL_PORT PORTB
 *
 * You can also define the following constants (see also below):
 * - I2C_CPUFREQ, when changing CPU clock frequency dynamically
 * - I2C_FASTMODE = 1 meaning that the I2C bus allows speeds up to 400 kHz
 * - I2C_SLOWMODE = 1 meaning that the I2C bus will allow only up to 25 kHz 
 * - I2C_NOINTERRUPT = 1 in order to prohibit interrupts while 
     communicating (see below). This can be useful if you use the library 
     for communicationg with SMbus devices, which have timeouts.
 * - I2C_CLOCK_STRETCH = 1 if there are devices that can stretch clock cycles
 */

#include <avr/io.h>
#include <Arduino.h>

#ifndef _SOFTI2C_H
#define _SOFTI2C_H   1

// Init function. Needs to be called once in the beginning.
void __attribute__ ((noinline)) i2c_init(void);

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit). 
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_start(uint8_t addr); 

// Similar to start function, but wait for an ACK! Be careful, this can 
// result in an infinite loop!
void  __attribute__ ((noinline)) i2c_start_wait(uint8_t addr);

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening 
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_rep_start(uint8_t addr);

// Issue a stop condition, freeing the bus.
void __attribute__ ((noinline)) i2c_stop(void) asm("ass_i2c_stop");

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_write(uint8_t value) asm("ass_i2c_write");

// Read one byte. If <last> is true, we send a NAK after having received 
// the byte in order to terminate the read sequence. 
uint8_t __attribute__ ((noinline)) i2c_read(bool last);

// You can set I2C_CPUFREQ independently of F_CPU if you 
// change the CPU frequency on the fly. If do not define it,
// it will use the value of F_CPU
#ifndef I2C_CPUFREQ
#define I2C_CPUFREQ F_CPU
#endif

// If I2C_FASTMODE is set to 1, then the highest possible frequency below 400kHz
// is selected. Be aware that not all slave chips may be able to deal with that!
#ifndef I2C_FASTMODE
#define I2C_FASTMODE 0
#endif
// If I2C_FASTMODE is not defined or defined to be 0, then you can set
// I2C_SLOWMODE to 1. In this case, the I2C frequency will not be higher 
// than 25KHz. This could be useful for problematic buses.
#ifndef I2C_SLOWMODE
#define I2C_SLOWMODE 0
#endif

// if I2C_NOINTERRUPT is 1, then the I2C routines are not interruptable.
// This is most probably only necessary if you are using a 1MHz system clock,
// you are communicating with a SMBus device, and you want to avoid timeouts.
// Be aware that the interrupt bit is enabled after each call. So the
// I2C functions should not be called in interrupt routines or critical regions.
#ifndef I2C_NOINTERRUPT
#define I2C_NOINTERRUPT 0
#endif

// If I2C_CLOCK_STRETCHING is 1, then a slave might stretch the clock pulse.
// Most slave devices are not able to do this!
// If you enable clock stretching, then an open SCL line may
// lead to hanging the system.
#ifndef I2C_CLOCK_STRETCHING
#define I2C_CLOCK_STRETCHING 0
#endif

#if I2C_FASTMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/400000L)/2-12)/3)
#else
#if I2C_SLOWMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/25000L)/2-12)/3)
#else
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/100000L)/2-12)/3)
#endif
#endif

// Table of I2C bus frequencies:
// CPU clock:           1MHz   2MHz    4MHz   8MHz   16MHz   20MHz
// Fast I2C mode       33kHz  66kHz  135kHz 280kHz  400kHz  400kHz
// Standard I2C mode   33kHz  66kHz  100kHz 100kHz  100kHz  100kHz
// Slow I2C mode       25kHz  25kHz   25kHz  25kHz   25kHz   25kHz

// constants for reading & writing
#define I2C_READ    1
#define I2C_WRITE   0

// map the IO register back into the IO address space
#define SDA_DDR       	(_SFR_IO_ADDR(SDA_PORT) - 1)
#define SCL_DDR       	(_SFR_IO_ADDR(SCL_PORT) - 1)
#define SDA_OUT       	_SFR_IO_ADDR(SDA_PORT)
#define SCL_OUT       	_SFR_IO_ADDR(SCL_PORT)
#define SDA_IN		(_SFR_IO_ADDR(SDA_PORT) - 2)
#define SCL_IN		(_SFR_IO_ADDR(SCL_PORT) - 2)

#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif

 
// Internal delay function.
void __attribute__ ((noinline)) i2c_delay_half(void) asm("ass_i2c_delay_half");

void  i2c_init(void)
{
  __asm__ __volatile__ 
    (" cbi %0,%4 ; release SDA \n\t" 
     " cbi %1,%5 ; release SCL \n\t" 
     " cbi %2,%4 ; \n\t" 
     " cbi %0,%5 \n\t" : :
     "I" (SDA_DDR), "I" (SCL_DDR),  
     "I" (SDA_OUT), "I" (SCL_OUT), 
     "I" (SDA_PIN), "I" (SCL_PIN)); 
}

void  i2c_delay_half(void)
{ // function call 3 cycles => 3C
#if I2C_DELAY_COUNTER < 1
  __asm__ __volatile__ (" ret");
  // 7 cycles for call and return
#else
  __asm__ __volatile__ (" ldi r25, %[DELAY] ;; 4C \n\t"
			"_Lidelay: dec r25 ;; 4C+xC \n\t"
			" brne _Lidelay  ;; 5C+(x-1)2C+xC \n\t"
			" ret ;; 9C+(x-1)2C+xC = 7C+xC" 
			: : [DELAY] "I" I2C_DELAY_COUNTER : "r25");
  // 7 cycles + 3 times x cycles
#endif
}

bool  i2c_start(uint8_t addr)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
     "sbi %[SDADDR],%[SDAPIN] ; force SDA low  \n\t" 
     "rcall ass_i2c_delay_half  \n\t"
     "rjmp ass_i2c_write "
     : : [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN)); 
  return true; // we never return here!
}

bool  i2c_rep_start(uint8_t addr)
{
  __asm__ __volatile__ 

    (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
     "sbi	%[SCLDDR],%[SCLPIN]	;force SCL low \n\t" 
     "rcall 	ass_i2c_delay_half	;delay  T/2 \n\t" 
     "cbi	%[SDADDR],%[SDAPIN]	;release SDA \n\t" 
     "rcall	ass_i2c_delay_half	;delay T/2 \n\t" 
     "cbi	%[SCLDDR],%[SCLPIN]	;release SCL \n\t" 
     "rcall 	ass_i2c_delay_half	;delay  T/2 \n\t" 
     "sbi 	%[SDADDR],%[SDAPIN]	;force SDA low \n\t" 
     "rcall 	ass_i2c_delay_half	;delay	T/2 \n\t" 
     "rjmp ass_i2c_write "
	: : [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN),
         [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN)); 
  return true; // just to fool the compiler
}

void  i2c_start_wait(uint8_t addr)
{
 __asm__ __volatile__ 
   (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
    " mov	__tmp_reg__,r24 \n\t"
    "_Li2c_start_wait1: \n\t"
    " sbi 	%[SDADDR],%[SDAPIN]	;force SDA low \n\t" 
    " rcall 	ass_i2c_delay_half	;delay T/2 \n\t" 
    " mov	r24,__tmp_reg__ \n\t" 
    " rcall 	ass_i2c_write	;write address \n\t" 
    " tst	r24		;if device not busy -> done \n\t" 
    " brne	_Li2c_start_wait_done \n\t" 
    " rcall	ass_i2c_stop	;terminate write operation \n\t" 
    " rjmp	_Li2c_start_wait1	;device busy, poll ack again \n\t" 
    "_Li2c_start_wait_done: \n\t"
    "ret"
     : : [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN)); 
}

void  i2c_stop(void)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
     " sbi %[SCLDDR],%[SCLPIN] ; force SCL low \n\t" 
     " sbi %[SDADDR],%[SDAPIN] ; force SDA low \n\t" 
     " rcall ass_i2c_delay_half ;; X cycles \n\t"
     " cbi %[SCLDDR],%[SCLPIN] ; release SCL \n\t" 
     " rcall ass_i2c_delay_half ;; X cycles \n\t"
     " cbi %[SDADDR],%[SDAPIN] ; release SDA \n\t" 
     " rcall ass_i2c_delay_half ;; X cycles \n\t"
#if I2C_NOINTERRUPT
     "sei \n\t"
#endif
     : : [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN),
         [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN)); 
}

bool i2c_write(uint8_t value)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
     " sec ; set carry flag ;; 1C \n\t"
     " rol r24 ; shift in carry and shift out MSB ;; +1 = 2C \n\t"
     " rjmp _Li2c_write_first ;; +2 = 4C preliminary \n\t"
     "_Li2c_write_bit:\n\t"
     " lsl r24 ; left shift into carry ;; 1C\n\t"
     "_Li2c_write_first:\n\t"
     " breq _Li2c_get_ack ; jump if transmit reg is empty ;; +1 = 2C (+2 in the end) \n\t"
     " sbi %[SCLDDR],%[SCLPIN] ; force SCL low ;; +2 = 4C \n\t"
     " brcc _Li2c_write_low ;; +1/+2 = 5/6C \n\t"
     " nop ;; +1 = 7C \n\t"
     " cbi %[SDADDR],%[SDAPIN]	;release SDA ;; +2 = 9C \n\t"
     " rjmp _Li2c_write_high ;; +2 = 11C \n\t"
     "_Li2c_write_low: \n\t"
     " sbi	%[SDADDR],%[SDAPIN]	;force SDA low ;; +2 = 9C \n\t"
     " rjmp	_Li2c_write_high ;; +2 = 11C \n\t"
     "_Li2c_write_high: \n\t"
     " rcall 	ass_i2c_delay_half	;delay T/2 ;; +X = 11C+X \n\t"
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL ;; +2 = 13C+X \n\t"
     " rcall	ass_i2c_delay_half	;delay T/2 ;; +X = 13C+2X \n\t"
     " rjmp	_Li2c_write_bit ;; +2 = 15C +2X for one bit-loop \n\t"
     "_Li2c_get_ack: \n\t"
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low ;; +2 = 5C \n\t"
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA ;;+2 = 7C \n\t"
     " rcall	ass_i2c_delay_half	;delay T/2 ;; +X = 7C+X \n\t"
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL ;; +2 = 9C+X\n\t"
     "_Li2c_ack_wait: \n\t"
#if I2C_CLOCK_STRETCHING
     " sbis	%[SCLIN],%[SCLPIN]	;wait SCL high (in case wait states are inserted) ;; +1/2/3 = 11C+X \n\t"
     " rjmp	_Li2c_ack_wait \n\t"
#else
     " nop ;; 10C +X \n\t"
     " nop ;; 11C +X \n\t"
#endif
     " clr	r24		;return 0 ;; +1 = 12C+X \n\t"
     " sbis	%[SDAIN],%[SDAPIN] ;if SDA low -> return 1;; + 1 = 13C+X \n\t"
     " ldi	r24,1 ;; + 1 = 14C+X \n\t"
     " rcall	ass_i2c_delay_half	;delay T/2 ;; +X = 13C + 2X \n\t"
     " clr	r25 ;; 14C+2X \n\t"
#if I2C_NOINTERRUPT
     "sei \n\t"
#endif
     " ret ;; + 4 = 18C + 2X for acknowldge bit"
     ::
      [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN), [SCLIN] "I" (SCL_IN),
      [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN), [SDAIN] "I" (SDA_IN)); 
  return true; // fooling the compiler
}

uint8_t i2c_read(bool last)
{
  __asm__ __volatile__ 
    (
#if I2C_NOINTERRUPT
     "cli \n\t"
#endif
     " ldi	r23,0x01 \n\t"
     "_Li2c_read_bit: \n\t"
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low ;; 2C \n\t" 
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA (from previous ACK) ;; 4C \n\t" 
     " rcall	ass_i2c_delay_half	;delay T/2 4C+X \n\t" 
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL 6C + X \n\t" 
     " rcall	ass_i2c_delay_half	;delay T/2 6C + 2X \n\t" 
     "_Li2c_read_stretch:  \n\t" 
#if I2C_CLOCK_STRETCHING
     " sbis %[SCLIN], %[SCLPIN]        ;loop until SCL is high (allow slave to stretch SCL) ;; 8C +2X \n\t" 
     " rjmp	_Li2c_read_stretch ;; 8C + 2X \n\t" 
#else
     " nop \n\t"
     " nop \n\t"
#endif
     " clc			;clear carry flag ;; 9C + 2X \n\t" 
     " sbic	%[SDAIN],%[SDAPIN]	;if SDA is high ;; 11C + 2X\n\t" 
     " sec			;  set carry flag ;; 12C + 2X \n\t" 
     " rol	r23		;store bit ;; 13C + 2X\n\t" 
     " brcc	_Li2c_read_bit	;while receive register not full ;; 15C + 2X for one bit loop \n\t" 
     
     "_Li2c_put_ack: \n\t" 
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low ;; 2C	 \n\t" 
     " cpi	r24,0 ;; 3C \n\t" 
     " breq	_Li2c_put_ack_low	;if (ack=0) ;; 5C \n\t" 
     " cbi	%[SDADDR],%[SDAPIN]	;      release SDA \n\t" 
     " rjmp	_Li2c_put_ack_high \n\t" 
     "_Li2c_put_ack_low:                ;else \n\t" 
     " sbi	%[SDADDR],%[SDAPIN]	;      force SDA low ;; 7C \n\t" 
     "_Li2c_put_ack_high: \n\t" 
     " rcall	ass_i2c_delay_half	;delay T/2 ;; 7C + X \n\t" 
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL ;; 9C +X \n\t" 
     "_Li2c_put_ack_wait: \n\t" 
#if I2C_CLOCK_STRETCHING
     " sbis	%[SCLIN],%[SCLPIN]	;wait SCL high ;; 11C + X\n\t" 
     " rjmp	_Li2c_put_ack_wait \n\t" 
#else
     " nop \n\t"
     " nop \n\t"
#endif
     " rcall	ass_i2c_delay_half	;delay T/2 ;; 11C + 2X\n\t" 
     " mov	r24,r23 ;; 12C + 2X \n\t"   
     " clr	r25 ;; 13 C + 2X\n\t" 
#if I2C_NOINTERRUPT
     "sei \n\t"
#endif
     " ret ;; 17C + X"
     ::
      [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN), [SCLIN] "I" (SCL_IN),
      [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN), [SDAIN] "I" (SDA_IN) 
     : "r23"); 
  return ' '; // fool the compiler!
}

#endif


