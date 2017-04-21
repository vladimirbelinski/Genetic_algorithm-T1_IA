#include "headers/Room.h"

Room::Room(){}

Room::Room(int _number){
  this->number = _number;
}

Room::Room(int _number, vector<int> _schedules){
  this->number = _number;
  this->schedules = _schedules;
}
