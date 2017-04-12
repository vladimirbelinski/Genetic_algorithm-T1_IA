using namespace std;

#include <string>

#include "Professor.h"
#include "Group.h"

class Subject{
  int id, np;  
  string code;
  Group group;
  Professor professor;
public:  
  Subject(int,int,Group,Professor);
};