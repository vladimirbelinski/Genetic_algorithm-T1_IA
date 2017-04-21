#include "headers/Professor.h"

Professor::Professor(){}

Professor::Professor(string name){
  this->name = name;
}

void Professor::add_schedule_to_avoid(int schedule_to_avoid){
  this->schedules_to_avoid.push_back(schedule_to_avoid);
}
