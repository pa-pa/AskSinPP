
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#ifdef ARDUINO
  #include <Arduino.h>

#if ARDUINO_ARCH_AVR
  #include <util/atomic.h>
#else

/*
 * http://www.stm32duino.com/viewtopic.php?f=3&t=258&hilit=ATOMIC_BLOCK&start=10
 */

#ifdef __arm__
#ifndef _CORTEX_M3_ATOMIC_H_
#define _CORTEX_M3_ATOMIC_H_

static __inline__ uint32_t __get_primask(void) \
{ uint32_t primask = 0; \
  __asm__ volatile ("MRS %[result], PRIMASK\n\t":[result]"=r"(primask)::); \
  return primask; } // returns 0 if interrupts enabled, 1 if disabled

static __inline__ void __set_primask(uint32_t setval) \
{ __asm__ volatile ("MSR PRIMASK, %[value]\n\t""dmb\n\t""dsb\n\t""isb\n\t"::[value]"r"(setval):); \
  __asm__ volatile ("" ::: "memory");}

static __inline__ uint32_t __iSeiRetVal(void) \
{ __asm__ volatile ("CPSIE i\n\t""dmb\n\t""dsb\n\t""isb\n\t"); \
  __asm__ volatile ("" ::: "memory"); return 1; }

static __inline__ uint32_t __iCliRetVal(void) \
{ __asm__ volatile ("CPSID i\n\t""dmb\n\t""dsb\n\t""isb\n\t"); \
  __asm__ volatile ("" ::: "memory"); return 1; }

static __inline__ void    __iSeiParam(const uint32_t *__s) \
{ __asm__ volatile ("CPSIE i\n\t""dmb\n\t""dsb\n\t""isb\n\t"); \
  __asm__ volatile ("" ::: "memory"); (void)__s; }

static __inline__ void    __iCliParam(const uint32_t *__s) \
{ __asm__ volatile ("CPSID i\n\t""dmb\n\t""dsb\n\t""isb\n\t"); \
  __asm__ volatile ("" ::: "memory"); (void)__s; }

static __inline__ void    __iRestore(const  uint32_t *__s) \
{ __set_primask(*__s); __asm__ volatile ("dmb\n\t""dsb\n\t""isb\n\t"); \
  __asm__ volatile ("" ::: "memory"); }


#define ATOMIC_BLOCK(type) \
for ( type, __ToDo = __iCliRetVal(); __ToDo ; __ToDo = 0 )

#define ATOMIC_RESTORESTATE \
uint32_t primask_save __attribute__((__cleanup__(__iRestore))) = __get_primask()

#define ATOMIC_FORCEON \
uint32_t primask_save __attribute__((__cleanup__(__iSeiParam))) = 0

#define NONATOMIC_BLOCK(type) \
for ( type, __ToDo = __iSeiRetVal(); __ToDo ;  __ToDo = 0 )

#define NONATOMIC_RESTORESTATE \
uint32_t primask_save __attribute__((__cleanup__(__iRestore))) = __get_primask()

#define NONATOMIC_FORCEOFF \
uint32_t primask_save __attribute__((__cleanup__(__iCliParam))) = 0

#endif
#endif

#endif

#else  // ARDUINO

#define ATOMIC_BLOCK(type)
#include <stdint.h>

#endif

#endif
