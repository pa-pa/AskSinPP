

#include <Debug.h>
#include <unistd.h>
#include <AlarmClock.h>
#include <Dimmer.h>

class Ping : public as::Alarm {
public:
  Ping () : as::Alarm(10) { async(true); }
  virtual void trigger (as::AlarmClock& clock) {
    tick=10;
	clock.add(*this);
	DPRINTLN("Ping");
  }
};

int main () {

  Ping ping;
  as::aclock.add(ping);

  as::DimmerStateMachine sm;
  sm.status(200,0x83);
  
  while( 1 ) {
    usleep(100000);
	--as::aclock;
	as::aclock.runready();
  }
  return 0;
}