//- -----------------------------------------------------------------------------------------------------------------------
// 2020-01-19 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

namespace as {

/**
 * Implements a stack with SIZE elements. New elements will be stored on top.
 * If the number of elements exceeds SIZE the oldest element will be removed/overwritten.
 */
template<class TYPE,int SIZE>
class RingStack {
  TYPE  _buffer[SIZE];
  TYPE* _current;
  int   _count;
  /**
   * Get reference to first position.
   * \return first position in buffer
   */
  TYPE& first () {
    return _buffer[0];
  }
  /**
   * Get reference to last position.
   * \return last position in buffer
   */
  TYPE& last () {
    return _buffer[SIZE-1];
  }

public:
  /**
   * Construct and initialize the stack
   */
  RingStack () {
    clear();
  }
  /**
   * Clear the stack. After this call the stack is empty.
   */
  void clear () {
    _current = &first();
    _count = 0;
  }
  /**
   * Return the maximal size of elements which can be stored
   * \return maximal number of elements
   */
  int size () const { return SIZE; }
  /**
   * Return the actual number of stored elements
   * \return number of elements currently stored
   */
  int count () const { return _count; }
  /**
   * Shift the internal structure to the next element
   * \return true if an old elements is overwritten
   */
  bool shift () {
    if( _count < SIZE ) _count++;
    _current--;
    if( _current < _buffer ) _current = &last();
    return _count == SIZE;
  }
  /**
   * Shift the stack to the next element and store the given value on the top
   * \param data value to store
   * \return true if an old elements is overwritten
   */
  bool shift (const TYPE& data) {
    bool result = shift();
    *_current = data;
    return result;
  }
  /**
   * Array-operator to access an element on the stack
   * \param index the index of the element on the stack to return
   * \return reference to the element on the stack
   */
  TYPE& operator [] (int index) {
    index = index <_count-1 ? index : _count-1;
    return *(_buffer + ((_current - _buffer + index) % SIZE));
  }
  /**
   * Array-operator to access an element on the stack
   * \param index the index of the element on the stack to return
   * \return reference to the element on the stack
   */
  const TYPE& operator [] (int index) const {
    index = index <_count-1 ? index : _count-1;
    return *(_buffer + ((_current - _buffer + index) % SIZE));
  }
};

}

#endif /* RINGBUFFER_H_ */
