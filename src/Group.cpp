#include "Group.h"

Group::Group(){}

Group::Group(int _room){
  this->room = _room;
}

void Group::add_schedule(int schedule){
  this->schedule.push_back(schedule);
}