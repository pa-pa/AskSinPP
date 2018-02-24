//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2018-01-07 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef SENSORS_H_
#define SENSORS_H_

namespace as {

enum Positions { NoPos=0, PosA, PosB, PosC };

class Sensor {
protected:
  bool _present;
public:
  Sensor () : _present(false) {}
  void init () {}
  bool present () { return _present; }
  void measure (__attribute__((unused)) bool async) {}
};

class Brightness : public virtual Sensor {
protected:
  uint16_t _brightness;
public:
  Brightness () : _brightness(0) {}
  // TODO - define value range for brightness
  uint16_t brightness () { return _brightness; }
};

class Temperature : public virtual Sensor {
protected:
  int16_t         _temperature;
public:
  Temperature () : _temperature(0) {}
  // temperature value multiplied by 10
  int16_t temperature () { return _temperature; }
};

class Humidity : public virtual Sensor {
protected:
  uint8_t   _humidity;
public:
  Humidity () : _humidity(0) {}
  // humidity value
  uint8_t humidity () { return _humidity; }
};

class Pressure : public virtual Sensor {
protected:
  uint16_t        _pressure;
public:
  Pressure () : _pressure(0) {}
  // pressure value
  uint16_t pressure () { return _pressure; }
};

class PositionSensor : public virtual Sensor {
protected:
  uint8_t  _position;
  uint16_t _updatecycle;
public:
  PositionSensor () : _position(0), _updatecycle(1) {}
  uint8_t position () { return _position; }
  uint16_t updatecycle () { return _updatecycle; }
};

} // end namespace

#endif
