//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2017-03-29 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
// 2019-01-11 scuba82: highly inspired by https://github.com/blackhack/ArduLibraries/tree/master/DimmerOne
//- -----------------------------------------------------------------------------------------------------------------------
/*
	Fires an interrupt, each time the zero-cross detection pin changes. Output pin will be held up high (trailing-edge phase cut) for a certain time,
	correspondig to the called dim value. The pre defined values are calculated for 50Hz mains, 8Mhz CPU frequence and a prescaler of 1024.
	PHASECUTMODE == 1 -> leading-edge phase cut; PHASECUTMODE == 0 	trailing-edge phase cut
*/

#include "PhaseCut.h"
namespace as {

	PhaseCut phaseCut;

	PhaseCut::PhaseCut()
	{
		isInit = false;
	}

	void PhaseCut::init(uint8_t output_pin)
	{
		if (isInit)
			return;
		isInit = true;
		ZERO_CROSS_PIN = ZEROPIN;
		OUTPUT_PIN = output_pin;
		_timer = 0;
		running = false;

		pinMode(OUTPUT_PIN, OUTPUT);
		pinMode(ZERO_CROSS_PIN, INPUT);

		TCCR2A = 0;
		TCCR2B = 0;

		uint8_t oldSREG = SREG;
		cli();
		SetTimer();
		SREG = oldSREG;

		TIMSK2 |= (1 << OCIE2A);     // Enable COMPA and COMPB interruptions of TIMER2
		_valid_zero_crossing = true;
	}

	bool PhaseCut::Start()
	{
		if (!isInit)
			return false;
		uint8_t oldSREG = SREG;
		cli();
		SREG = oldSREG;
		attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), ZeroCrossEventCaller, CHANGE);
		running = true;
		return true;
	}

	bool PhaseCut::Stop()
	{
		if (!isInit)
			return false;
		detachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN));
		digitalWrite(OUTPUT_PIN, LOW);
		running = false;
		return false;
	}

	bool PhaseCut::isrunning()
	{
		return running;
	}
	bool PhaseCut::SetDimValue(double value)
	{
		if (!isInit)
			return false;
		_timer = value;
		 
		return true;
	}

	void PhaseCut::SetTimer()
	{
			OCR2A = _timer; 
	}

	void ZeroCrossEventCaller()
	{
		phaseCut.ZeroCrossEvent();
	}


	ISR(TIMER2_COMPA_vect)
	{
		phaseCut.CmpAEvent();
	}

	void PhaseCut::ZeroCrossEvent()
	{
		if (!_valid_zero_crossing)
			return;
		_valid_zero_crossing = false;
		phaseCut.Fire();
		TCNT2 = 0;   // Restart counter (no need to call cli() inside an ISR)
		TCCR2B |= (1 << WGM21) | (1 << CS20) | (1 << CS21) | (1 << CS22) ; // Enable/start CTC and set prescaler to 1024
	}

	void PhaseCut::Fire()
	{
				#if PHASECUTMODE == 1
					digitalWrite(OUTPUT_PIN, LOW);
				#else
					if ( _timer > 0 ) digitalWrite(OUTPUT_PIN, HIGH);
				#endif
		
	}

	void PhaseCut::CmpAEvent()
	{
				#if PHASECUTMODE == 1
					if ( _timer < 75 )digitalWrite(OUTPUT_PIN, HIGH);
				#else
					digitalWrite(OUTPUT_PIN, LOW);
				#endif
				
			TCCR2B = 0; // Disable/stop CTC
			SetTimer();
			_valid_zero_crossing = true;
	}
}