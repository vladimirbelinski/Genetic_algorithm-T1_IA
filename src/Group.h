using namespace std;

#include <vector>

class Group{
  int room;
  vector<int> schedule;
public:
  Group(int);
  void add_schedule(int);
};