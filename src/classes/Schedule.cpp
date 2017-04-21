#include "headers/Schedule.h"

Schedule::Schedule(){}

Schedule::Schedule(Class _class,int _room,int _schedule){
  this->_class = _class;
  this->room = _room, this->schedule = _schedule;
}
