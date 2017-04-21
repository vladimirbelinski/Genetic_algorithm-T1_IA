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
#include "Schedule.h"
#include "Subject.h"

const int POPULATION_SIZE = 28;

vector<string> professor_index;
map<string,Professor> professors;
map<string, map<int, vector<int> > > courses;

typedef pair<int,int> ii;
typedef pair<ii,ii> _4i;

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
    /*
      A lista check contém todos os professores ainda não sorteados.
    */
    list<int> check;
    /*
      Adcionar à lista de não sorteados todos os professores diferentes de i.
    */
    for(int j = 0; j < (int)professors.size(); j++)
      if(j != i) check.push_back(j);

    /*
      O vector perm representa uma permutação de professores.
    */
    vector<int> perm;
    perm.push_back(i);
    while(!check.empty()){
      /*
        Sorteio do próximo professor.
      */
      knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
      uniform_int_distribution<int> distribution(0,((int)check.size())-1);
      auto dice = bind(distribution, generator);
      auto it = check.begin();
      /*
        Retirar professor da lista de não sorteados.
      */
      advance(it,dice());
      perm.push_back(*it);
      check.erase(it);
    }
    permutations.push_back(perm);
  }
}

vector< vector<Schedule> > population;
void generate_population(int group_size){
  for(int i = 0,g; i < (int)permutations.size(); i++,g = 0){
    do{
      g++;
      set<ii,ii_cmp> is_alocated_room_schedule;
      vector<Schedule> person;
      for(int j = 0; j < (int)permutations[i].size(); j++){
        Professor p = professors[professor_index[permutations[i][j]]];
        for(auto& subjects : p.subjects){//turnos e semestres para um professor.
          for(auto& subject : subjects.second){//disciplinas para turnos e semestres.
            int periods = 0;
            for(auto& c : courses[subject.course]){//turnos e semestres.
              for(auto& class_room_schedule : c.second){//salas de cada curso.
                ii schedule = ii(c.first,class_room_schedule);
                if(is_alocated_room_schedule.find(schedule) == is_alocated_room_schedule.end()){
                  //cout << p.name << " " << subject.code << " " << subject.course << " " << c.first << " " << class_room_schedule << endl;
                  is_alocated_room_schedule.insert(schedule);
                  person.push_back(Schedule(Class(periods++,subject,p),c.first,class_room_schedule));
                }
                if(periods == subject.period_quantity) break;
              }
              if(periods == subject.period_quantity) break;
            }
            if(periods == subject.period_quantity) break;
          }
        }
      }
      population.push_back(person);
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && g < group_size);
  }
}

struct class_comp{
  bool operator()(Class a,Class b){
    int diff = a.professor.name.compare(b.professor.name);
    if(diff) return diff < 0;
    diff = a.subject.course.compare(b.subject.course);
    if(diff) return diff < 0;
    diff = a.subject.code.compare(b.subject.code);
    if(diff) return diff < 0;
    return a.number < b.number;
  }
};

/*
chave first: Sala
chave second; Horário.
Valor: professor, matéria e sala.
*/
set<Class,class_comp> vis;
map< ii, Class > match;
map<Class,_4i,class_comp> embryo;
int Aug(Class v){
  if(vis.find(v) != vis.end()) return 0;
  vis.insert(v);
  knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
  uniform_int_distribution<int> distribution(0,1);
  auto dice = bind(distribution, generator);
  if(dice()){
    if(match.find(embryo[v].first) == match.end() || Aug(match[embryo[v].first])){
      match[embryo[v].first] = v;
      return 1;
    }
    if(match.find(embryo[v].second) == match.end() || Aug(match[embryo[v].second])){
      match[embryo[v].second] = v;
      return 1;
    }
  }
  else{
    if(match.find(embryo[v].second) == match.end() || Aug(match[embryo[v].second])){
      match[embryo[v].second] = v;
      return 1;
    }
    if(match.find(embryo[v].first) == match.end() || Aug(match[embryo[v].first])){
      match[embryo[v].first] = v;
      return 1;
    }
  }
  return 0;
}

bool bipartite_matching(){
  int m = 0;
  for(auto& e : embryo){
    vis.clear();
    m += Aug(e.first);
  }
  return m == (int)embryo.size();
}

vector<Schedule> cross(vector<Schedule> & person1, vector<Schedule> & person2){
  embryo.clear();

  for(auto& p : person1) embryo[p._class].first = ii(p.room,p.schedule);
  for(auto& p : person2) embryo[p._class].second = ii(p.room,p.schedule);
  /*
  printf("Embryo\n");
  for(auto& e : embryo){
    cout << e.first.professor.name << " " << e.first.subject.code << " " << e.first.subject.course << " " << e.first.number << " (" << e.second.first.first << ", " << e.second.first.second << "), " << " (" << e.second.second.first << ", " << e.second.second.second << ") " << endl;
  }*/

  bipartite_matching();
  vector<Schedule> schedule;
  for(auto& m : match){
    schedule.push_back(Schedule(m.second,m.first.first,m.first.second));
  }
  return schedule;
}

void print_schedule(vector<Schedule>& person){
  for(auto& schedule : person){
    cout << schedule._class.professor.name << " " << schedule._class.subject.code << " " << schedule._class.subject.course << " " << schedule.room << " " << schedule.schedule << endl;
  }
}

int main(void){
  read_professors();
  read_room_schedules();
  read_subjects();
  generate_people_permutation();
  generate_population(POPULATION_SIZE / (int)professors.size());
  vector<Schedule> person = cross(population.front(),population.back());
  printf("\nPerson 1, size:%d\n",(int)population.front().size());
  print_schedule(population.front());
  printf("\nPerson 2, size:%d\n",(int)population.back().size());
  print_schedule(population.back());
  printf("\nChild, size:%d\n",(int)person.size());
  print_schedule(person);
  return 0;
}
