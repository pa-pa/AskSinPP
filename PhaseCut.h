#ifndef _PHASECUT_h
#define _PHASECUT_h

#include "Arduino.h"

class PhaseCut
{
public:
    PhaseCut();

    void init(uint8_t zero_crossing_pin, uint8_t output_pin, bool mode);

    bool Start();
    bool Stop();
	bool isrunning();

    bool SetDimmer(uint8_t value);
    
    uint8_t GetDimmer() { return _dim_value; }
	bool 	GetMode() { return _mode; }
    
    void ZeroCrossEvent();
    void ComparatorBEvent();
    void ComparatorAEvent();

private:
    void CalcDelay();

    uint8_t ZERO_CROSS_PIN;
    uint8_t OUTPUT_PIN;
	uint8_t delaycount;
    uint8_t _dim_value;
    
    bool _valid_zero_crossing;
    bool isInit;
	bool running;
	bool _mode; // false = trailing-edge phase cut; true = leading-edge phase cut
};

void ZeroCrossEventCaller();

extern PhaseCut phasecut;

#endif
