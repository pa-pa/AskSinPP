
#ifndef __LINK_H__
#define __LINK_H__

#include "Atomic.h"

class Link {
  // successor element
  Link* link;
public:
  Link () : link(0) {}
  Link (Link* item) : link(item) {}

  // return successor
  Link* select () const {
    Link* result = 0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
	  result = link;
	}
    return result;
  }

  // define successor
  void select (Link* item) { 
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      link=item; 
    }
  }

  // add successor
  void append (Link& item) {
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ) {
      item.select(select());
      select(&item);
    }
  }

  // return tail item
  Link* ending () const {
	Link* item=0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
      item=(Link*)this;
      while( item->select() != 0 ) {
        item = item->select();
      }
    }
    return item;
  }

  // remove and return successor
  Link* unlink () {
    Link* item=0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
      item=select();
      if( item!=0 ) {
        detach();
      }
    }
    return item;
  }

  // remove all, return successor
  Link* remove () {
	Link* item=0;
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
      item=select();
      select(0);
    }
    return item;
  }

  // remove successor
  void detach () {
    ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
      select(select()->select());
    }
  }

  // Link* search (const Link*) const;     // return container instance
  // void   remove (const Link&);           // remove item from chain
};

#endif
