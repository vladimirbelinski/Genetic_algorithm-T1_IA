#pragma once

using namespace std;

#include <vector>

class Room{
public:
  int number;
  vector<int> schedules;
  Room();
  Room(int);
  Room(int, vector<int>);
};
