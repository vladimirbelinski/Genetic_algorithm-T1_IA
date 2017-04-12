using namespace std;

#include <vector>

class Course{
  int room;
  vector<int> schedule;
public:
  Course(int);
  void add_schedule(int);
};