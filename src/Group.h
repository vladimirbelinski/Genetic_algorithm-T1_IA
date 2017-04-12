#pragma once

using namespace std;

#include <vector>

class Group{
  int room;
  vector<int> schedule;
public:
  Group();
  Group(int);
  void add_schedule(int);
};