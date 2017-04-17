#include <stdio.h>

using namespace std;

#include <functional>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <set>

#include "Professor.h"
#include "Subject.h"

const int POPULATION_SIZE = 28;

vector<string> professor_index;
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
    professor_index.push_back(name);    
    professors[name] = Professor(name);          
    for(int j = 0; j < schedule_number; j++){
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

vector< vector<int> > permutations;
void generate_people_permutation(){
  for(int i = 0; i < (int)professors.size(); i++){    
    list<int> check;
    for(int j = 0; j < (int)professors.size(); j++)
      if(j != i) check.push_back(j);
    vector<int> perm;
    perm.push_back(i);    
    while(!check.empty()){
      knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
      uniform_int_distribution<int> distribution(0,((int)check.size())-1);  
      auto dice = bind(distribution, generator);
      auto it = check.begin();
      advance(it,dice());
      perm.push_back(*it);
      check.erase(it);
    }
    permutations.push_back(perm);
  }
}

void generate_population(int group_size){    
  for(int i = 0,g; i < (int)permutations.size(); i++,g = 0){
    cout << "Current permutation:" << endl;
    for (auto& index: permutations[i])
      cout << index << ";";
    cout << endl;
    do{
      g++;      
      cout << "New person" << endl;
      set<ii,ii_cmp> is_alocated_room_schedule;
      for(int j = 0; j < (int)permutations[i].size(); j++){
        Professor p = professors[professor_index[permutations[i][j]]];        
        cout << p.name << endl;
        for(auto& subjects : p.subjects){//turnos e semestres para um professor.
          for(auto& subject : subjects.second){//disciplinas para turnos e semestres.
            int periods = 0;
            for(auto& c : courses[subject.course]){//turnos e semestres.
              for(auto& class_room_schedule : c.second){//salas de cada curso.

                ii schedule = ii(c.first,class_room_schedule);
                if(is_alocated_room_schedule.find(schedule) == is_alocated_room_schedule.end()){
                  periods++;
                  cout << p.name << " " << subject.code << " " << c.first << " " << class_room_schedule << endl;
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
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && g < group_size);
    cout << endl;
  }
}

int main(void){     
  read_professors();
  read_room_schedules();
  read_subjects();
  generate_people_permutation();
  generate_population(POPULATION_SIZE / (int)professors.size());
  
  return 0;
}