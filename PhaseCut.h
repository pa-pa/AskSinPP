//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-29 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-01-11 scuba82: highly inspired by https://github.com/blackhack/ArduLibraries/tree/master/DimmerOne
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _PHASECUT_h
#define _PHASECUT_h

#ifndef PHASECUTMODE
#define PHASECUTMODE 0
#endif 

#ifndef ZEROPIN
#define ZEROPIN 2
#endif

#include "Arduino.h"
//#include "Debug.h"

namespace as {
	class PhaseCut
	{
	public:
		PhaseCut();

		void init(uint8_t output_pin);

		bool Start();
		bool Stop();
		bool isrunning();

		bool SetDimValue(double value);

		double GetDimValue() { return _timer; }

		// Do not call this!
		void ZeroCrossEvent();
		void CmpAEvent();
		void Fire();

	private:
		void SetTimer();

		uint8_t ZERO_CROSS_PIN;
		uint8_t OUTPUT_PIN;
		double _timer;

		bool _valid_zero_crossing;
		bool isInit;
		bool running;
	};

	void ZeroCrossEventCaller();

	extern PhaseCut phaseCut;
}
#endif
