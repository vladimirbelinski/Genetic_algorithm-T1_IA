#include <stdio.h>

using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>

#include "Professor.h"
#include "Subject.h"

map<string,Professor> professors;
map<string, map<int, vector<int> > > courses;

typedef pair<int,int> ii;

void read_professors(){
  int professor_number;
  cin >> professor_number;
  for(int i = 0; i < professor_number; i++){
    string name;
    int schedule_number, schedule_to_avoid;
    cin >> name >> schedule_number;
    if(professors.find(name) == professors.end())
      professors[name] = Professor(name);
    for(int i = 0; i < schedule_number; i++){
      cin >> schedule_to_avoid;
      professors[name].add_schedule_to_avoid(schedule_to_avoid);
    }
  }
}

void read_room_schedules(){
  string course_code;
  int room_quantity, room_number, available_schedules;
  cin >> room_quantity;
  for(int i = 0; i < room_quantity; i++){
    cin >> course_code >> room_number >> available_schedules;
    if(courses.find(course_code) == courses.end())
      courses[course_code] = map<int, vector<int> >();

    for(int schedule, j = 0; j < available_schedules; j++){
      cin >> schedule;
      courses[course_code][room_number].push_back(schedule);
    }
  }
}

void read_subjects() {
  int subject_quantity;
  cin >> subject_quantity;
  for(int i = 0; i < subject_quantity; i++){
    int period_quantity;
    string subject_code, course_code, professor_name;
    cin >> subject_code >> period_quantity >> course_code >> professor_name;

    Subject s = Subject(i,period_quantity,subject_code,course_code);
    professors[professor_name].subjects[course_code].push_back(s);
  }
}

struct ii_cmp {
  bool operator()(ii a, ii b){
    return (a.first != b.first) ? a.first < b.first : a.second < b.second;
  }
};

void generate_population(){
  set<ii,ii_cmp> is_alocated_room_schedule;
  for(auto& p : professors){
    cout << p.second.name << endl;
    for(auto& subjects : p.second.subjects){//turnos e semestres para um professor.
      for(auto& subject : subjects.second){//disciplinas para turnos e semestres.
        int periods = 0;
        for(auto& c : courses[subject.course]){//turnos e semestres.
          for(auto& class_room_schedule : c.second){//salas de cada curso.

            ii schedule = ii(c.first,class_room_schedule);
            if(is_alocated_room_schedule.find(schedule) == is_alocated_room_schedule.end()){
              periods++;
              cout << p.second.name << " " << subject.code << " " << c.first << " " << class_room_schedule << endl;
              is_alocated_room_schedule.insert(schedule);
            }
            if(periods == subject.period_quantity) break;
          }
          if(periods == subject.period_quantity) break;
        }
        if(periods == subject.period_quantity) break;
      }
    }
    cout << endl;
  }
}

int main(void){
  read_professors();
  read_room_schedules();
  read_subjects();
  generate_population();
  return 0;
}