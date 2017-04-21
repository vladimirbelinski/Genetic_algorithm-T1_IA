#pragma once

#include "Class.h"

class Schedule{
public:
  Room room;
  int number;
  Class _class;
  Schedule();
  Schedule(Room,int,Class);
};
