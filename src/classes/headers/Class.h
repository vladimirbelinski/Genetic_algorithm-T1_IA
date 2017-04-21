#pragma once

#include "Professor.h"
#include "Subject.h"

class Class{
public:
  int number;
  Subject subject;
  Professor professor;
  Class();
  Class(int,Subject,Professor);
};
