using namespace std;

#include <string>
#include <vector>

class Professor{
  string name;
  vector<int> schedules_to_avoid;
public:
  Professor(string name);
  void add_schedule_to_avoid(int);
};