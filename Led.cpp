//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------


#include <Pins.h>
#include <AlarmClock.h>
#include <Led.h>

namespace as {

const LedStates::BlinkPattern LedStates::single [9] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 20, {5, 5,} },  // 1; define pairing string
    {2, 1, {10, 1,} },   // 2; define send indicator
    {0, 0, {0, 0,} },   // 3; define ack indicator
    {0, 0, {0, 0,} },   // 4; define no ack indicator
    {6, 3, {5, 1, 1, 1, 1, 10} }, // 5; define battery low indicator
    {6, 1, {1, 1, 5, 1, 5, 10} }, // 6; define welcome indicator
    {2, 6, {2, 2, } },  // 7; key long indicator
    {5, 1, {8, 4, 4, 4, 4} },  // 8; failure
};

const LedStates::BlinkPattern LedStates::dual1 [9] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 20, {5, 5,} },  // 1; define pairing string
    {2, 1, {20, 1,} },   // 2; define send indicator
    {2, 1, {4, 0,} },   // 3; define ack indicator
    {0, 0, {0, 0,} },   // 4; define no ack indicator
    {6, 3, {5, 1, 1, 1, 1, 10} }, // 5; define battery low indicator
    {6, 1, {1, 1, 5, 1, 5, 10} }, // 6; define welcome indicator
    {2, 6, {2, 2, } },  // 7; key long indicator
    {5, 1, {8, 4, 4, 4, 4} },  // 8; failure
};
const LedStates::BlinkPattern LedStates::dual2 [9] PROGMEM = {
    {0, 0, {0 , 0 } },  // 0; define nothing
    {2, 20, {5, 5,} },  // 1; define pairing string
    {2, 1, {20, 1,} },   // 2; define send indicator
    {0, 0, {0, 0,} },   // 3; define ack indicator
    {2, 1, {4, 0,} },   // 4; define no ack indicator
    {6, 3, {5, 1, 1, 1, 1, 10} }, // 5; define battery low indicator
    {6, 1, {7, 0, 0, 6, 0, 10} }, // 6; define welcome indicator
    {2, 6, {2, 2, } },  // 7; key long indicator
    {5, 1, {8, 4, 4, 4, 4} },  // 8; failure
};

}
