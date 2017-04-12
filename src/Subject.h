#pragma once

using namespace std;

#include <string>

#include "Professor.h"
#include "Group.h"

class Subject{  
  string code;
  Group group;
  Professor professor;
  int id, period_quantity;  
public:  
  Subject();
  Subject(int,int,string,Group,Professor);
};