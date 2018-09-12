//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de>, <dirk@hfma.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _CM_H
	#define _CM_H

	#define GET_2_BYTE_VALUE(n) ( ((uint16_t)n >> 8) | ((uint16_t)n << 8) )			// macro for changing byte order for ARV little endian

	#define AS_CM_JT_NONE                                     0
	#define AS_CM_JT_ONDELAY                                  1
	#define AS_CM_JT_REFON                                    2						// Ignore for dimmer
	#define AS_CM_JT_ON                                       3
	#define AS_CM_JT_OFFDELAY                                 4
	#define AS_CM_JT_REFOFF                                   5						// Ignore for dimmer
	#define AS_CM_JT_OFF                                      6
	#define AS_CM_JT_RAMPON                                   8
	#define AS_CM_JT_RAMPOFF                                  9

	#define AS_CM_ACTIONTYPE_INACTIVE                         0
	#define AS_CM_ACTIONTYPE_JUMP_TO_TARGET                   1
	#define AS_CM_ACTIONTYPE_TOGGLE_TO_COUNTER                2
	#define AS_CM_ACTIONTYPE_TOGGLE_INVERSE_TO_COUNTER        3
	#define AS_CM_ACTIONTYPE_UPDIM                            4	// check
	#define AS_CM_ACTIONTYPE_DOWNDIM                          5	// check
	#define AS_CM_ACTIONTYPE_TOGGLEDIM                        6	// check
	#define AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER             7	// check
	#define AS_CM_ACTIONTYPE_TOGGLEDIM_TO_COUNTER_INVERSE     8	// check

	#define AS_CM_CT_X_GE_COND_VALUE_LO                       0
	#define AS_CM_CT_X_GE_COND_VALUE_HI                       1
	#define AS_CM_CT_X_LT_COND_VALUE_LO                       2
	#define AS_CM_CT_X_LT_COND_VALUE_HI                       3
	#define AS_CM_CT_COND_VALUE_LO_LE_X_LT_COND_VALUE_HI      4
	#define AS_CM_CT_X_LT_COND_VALUE_LO_OR_X_GE_COND_VALUE_HI 5

	#define AS_CM_DRIVINGMODE_DRIVE_DIRECTLY                  0
	#define AS_CM_DRIVINGMODE_DRIVE_VIA_UPPER_END_POSITION    1
	#define AS_CM_DRIVINGMODE_DRIVE_VIA_LOWER_END_POSITION    2
	#define AS_CM_DRIVINGMODE_DRIVE_VIA_NEXT_END_POSITION     3

	#define AS_CM_STATETOSEND_NONE                            0
	#define AS_CM_STATETOSEND_ACK                             1
	#define AS_CM_STATETOSEND_STATE                           2

	#define AS_CM_EXTSTATE_NONE                               0x00
	#define AS_CM_EXTSTATE_UP                                 0x10
	#define AS_CM_EXTSTATE_DOWN                               0x20
	#define AS_CM_EXTSTATE_RUNNING                            0x40
	#define AS_CM_EXTSTATE_LOWBAT                             0x80

  #define AS_CM_EXTSTATE_OVERLOAD                           0x02
  #define AS_CM_EXTSTATE_OVERHEAT                           0x04
  #define AS_CM_EXTSTATE_REDUCED                            0x08

	#define AS_CM_KEYCODE_NONE                                0
	#define AS_CM_KEYCODE_SHORT                               1
	#define AS_CM_KEYCODE_LONG                                2

#endif
