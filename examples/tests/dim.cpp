
// g++ -g -I .. -o dim dim.cpp ../AlarmClock.cpp ../Storage.cpp

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


#include <Debug.h>
#include <AlarmClock.h>
//#include <Dimmer.h>


/*
as::DimmerStateMachine sm;

class Ping : public as::Alarm {
public:
  Ping () : as::Alarm(seconds2ticks(1)) {}
  virtual void trigger (as::AlarmClock& clock) {
    set(seconds2ticks(1));
	  clock.add(*this);
	  DPRINT(".");
  }
};

class OnOff : public as::Alarm {
  uint8_t cnt;
  as::DimmerList3 l1,l2, l3;
public:
  OnOff () : as::Alarm(seconds2ticks(15)), cnt(0), l1(20),l2(20+l1.size()),l3(20+l1.size()+l2.size()) {
    l1.even();
    l1.sh().rampOnTime(3);
    l1.sh().rampOffTime(3);
    l2.odd();
    l2.sh().rampOnTime(3);
    l2.sh().rampOffTime(3);
    l3.single();
    l3.sh().rampOnTime(10);
    l3.sh().rampOffTime(10);
    l3.sh().onLevel(100);
  }
  virtual void trigger (as::AlarmClock& clock) {
    set(seconds2ticks(5));
    clock.add(*this);
    if( cnt < 3 ) {
      if( (cnt & 0x01) == 0x01 ) {
        sm.remote(l1.sh(),cnt);
      }
      else {
        sm.remote(l2.sh(),cnt);
      }
    }
    else {
      sm.remote(l3.sh(),cnt);
    }
    ++cnt;
  }
};
*/

class Al : public as::Alarm {
public:
  Al(uint32_t t) : as::Alarm(t) {}
  virtual ~Al () {}
  virtual void trigger (as::AlarmClock& c) {}
};

void print(as::Alarm* t) {
  while( t != 0 ) {
    std::cout << t << ": " << t->tick << std::endl;
    t = (as::Alarm*)t->select();
  }
  std::cout << std::endl;
}

int main () {

  Al a1(5), a2(8), a3(8);

  as::sysclock.add(a1);
  as::sysclock.add(a2);
  as::sysclock.add(a3);

  print(as::sysclock.first());

  as::sysclock.cancel(a2);
  print(as::sysclock.first());

  a2.tick=8;
  as::sysclock.add(a2);
  print(as::sysclock.first());

  as::sysclock.cancel(a1);
  print(as::sysclock.first());

  a1.tick=10;
  as::sysclock.add(a1);
  print(as::sysclock.first());


  /*
  Ping ping;
  as::sysclock.add(ping);
  OnOff oo;
  as::sysclock.add(oo);

  sm.setLevel(200,0x41,0x83);
  
  while( 1 ) {
    usleep(100000);
	  --as::sysclock;
	  as::sysclock.runready();
  }
  */
  return 0;
}
