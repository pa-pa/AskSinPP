#include "PhaseCut.h"


PhaseCut phasecut;

PhaseCut::PhaseCut()
{
    isInit = false;
	delaycount = 0;
}

void PhaseCut::init(uint8_t zero_crossing_pin, uint8_t output_pin, bool mode)
{
    if (isInit)
        return;
    isInit = true;

    ZERO_CROSS_PIN = zero_crossing_pin;
    OUTPUT_PIN = output_pin;
	_mode = mode;
	running = false;

    pinMode(OUTPUT_PIN, OUTPUT);
    pinMode(ZERO_CROSS_PIN, INPUT);
    // Arduino have some default options in init() that we dont want (this may broken further calls to analogWrite())
    TCCR2A = 0;
    TCCR2B = 0;
    uint8_t oldSREG = SREG;
    cli();
    CalcDelay();
    SREG = oldSREG;
    // Enable COMPA and COMPB interruptions of TIMER1
    TIMSK2 |= (1 << OCIE2A) | (1 << OCIE2B);
    _valid_zero_crossing = true;
}

bool PhaseCut::Start()
{
    if (!isInit)
        return false;

    uint8_t oldSREG = SREG;
    cli();
    CalcDelay();
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
bool PhaseCut::SetDimmer(uint8_t value)
{
    if (!isInit)
        return false;
    _dim_value = value;
    return true;
}

void PhaseCut::CalcDelay()
{
	if (_mode){
		delaycount = map(_dim_value,0,200,75,7);
		OCR2A = delaycount;
	}
	else{
		delaycount = map(_dim_value,0,200,7,75);
		OCR2A = delaycount; 
	}
}

void ZeroCrossEventCaller()
{
    phasecut.ZeroCrossEvent();
}

ISR(TIMER2_COMPA_vect)
{
    phasecut.ComparatorAEvent();
}

void PhaseCut::ZeroCrossEvent()
{
    if (!_valid_zero_crossing)
        return;
    _valid_zero_crossing = false;
	if (_mode){
		if ( _dim_value > 0 ) digitalWrite(OUTPUT_PIN, LOW);
	}
	else{
		if ( _dim_value > 0 ) digitalWrite(OUTPUT_PIN, HIGH);
	}
    TCNT2 = 0;
    TCCR2B |= (1 << WGM21) | (1 << CS20) | (1 << CS21) | (1 << CS22) ;     // Enable/start CTC and set prescaler to 1024

}

void PhaseCut::ComparatorAEvent()
{	
		if (_mode){
			digitalWrite(OUTPUT_PIN, HIGH);
		}
		else{
			digitalWrite(OUTPUT_PIN, LOW);
		}
		TCCR2B = 0; // Disable/stop CTC
		CalcDelay();
		_valid_zero_crossing = true; // Next zero cross is a valid one
}
