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
  Subject(int,int,Group,Professor);
};