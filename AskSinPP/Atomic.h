
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#ifdef ARDUINO
  #include <Arduino.h>
  #include <util/atomic.h>
#else
  #define ATOMIC_BLOCK(a) {}
  #define ATOMIC_RESTORESTATE 1

  #include <stdint.h>
#endif



#endif
