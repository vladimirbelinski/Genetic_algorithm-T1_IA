#pragma once

using namespace std;

#include <string>
#include <vector>
#include <map>

#include "Subject.h"

class Professor{
public:
  string name;
  /*
    Chave: Turno/Semestre Ex: V1
    Valor: disciplinas que o professor vai lecionar para determinado turno/semestre
  */
  map<string, vector<Subject> > subjects;
  vector<int> schedules_to_avoid;
  Professor();
  Professor(string name);
  void add_schedule_to_avoid(int);
};