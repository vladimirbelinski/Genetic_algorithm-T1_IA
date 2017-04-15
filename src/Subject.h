#pragma once

using namespace std;

#include <string>

class Subject{
public:
  string code;
  string course;
  int id, period_quantity;
  Subject();
  Subject(int,int,string,string);
};