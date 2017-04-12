using namespace std;

#include <string>

class Subject{
  int id, np;  
  string code;
  Group group;
  Professor professor;
public:  
  Subject(int,int,Group,Professor);
};