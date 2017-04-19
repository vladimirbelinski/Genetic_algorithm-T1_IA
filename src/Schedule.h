#pragma once

#include "Professor.h"
#include "Subject.h"

class Schedule{
public:
  Subject subject;
  int room,schedule;
  Professor professor;  
  Schedule();
  Schedule(Subject,int,int,Professor);
};