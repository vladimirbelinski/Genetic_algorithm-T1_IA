#include <stdio.h>

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

#include "ia.h"

#define DEBUG 0

using namespace std;

typedef pair<int,int> ii;
typedef pair<ii,ii> _4i;
typedef pair< int, vector<int> > room_schedules;

const int POPULATION_SIZE = 28;

vector<string> professor_index;

// Map de nomes de professores para os
// horários que ele não quer dar aula
map<string, vector<int> > professors;

// Map entre professor e cursos que ele leciona.
map<string, vector<string> > professor_courses;

// Vetor de todas as salas disponíveis
vector<room_t> rooms;

// Map de cursos para suas salas
map<string, vector<room_t> > courses;

// Map de cursos para suas disciplinas
map<string, vector<subject_t> > subjects;

bool room_comp(room_t &a,room_t &b){
  if(a.available_schedules.size() != b.available_schedules.size())
    return a.available_schedules.size() < b.available_schedules.size();
  for(int i = 0; i < (int)a.available_schedules.size(); i++){
    if(a.available_schedules[i] != b.available_schedules[i]){
      return a.available_schedules[i] < b.available_schedules[i];
    }
  }
  if(a.number != b.number) return a.number < b.number;
  return a.course.compare(b.course) < 0;
}

void read_professors(){
  int professor_quantity;
  cin >> professor_quantity;
  for(int i = 0; i < professor_quantity; i++){
    string name;
    int schedule_quantity;
    cin >> name >> schedule_quantity;
    professor_index.push_back(name);
    professors[name] = vector<int>();
    professor_courses[name] = vector<string>();
    for(int j = 0; j < schedule_quantity; j++){
      int schedule_to_avoid;
      cin >> schedule_to_avoid;
      professors[name].push_back(schedule_to_avoid);
    }
  }
}

void read_room_schedules(){
  int room_quantity;
  cin >> room_quantity;
  for(int i = 0; i < room_quantity; i++){
    string course;
    int room_number, available_schedules_quantity;
    cin >> course >> room_number >> available_schedules_quantity;
    if(courses.find(course) == courses.end())
      courses[course] = vector<room_t>();
    room_t room;
    room.course = course;
    room.number = room_number;
    for(int j = 0; j < available_schedules_quantity; j++){
      int available_schedule;
      cin >> available_schedule;
      room.available_schedules.push_back(available_schedule);
    }
    sort(room.available_schedules.begin(),room.available_schedules.end());
    rooms.push_back(room);
  }
  sort(rooms.begin(), rooms.end(), room_comp);
  for(auto& room: rooms) courses[room.course].push_back(room);
}

void read_subjects() {
  int subject_quantity;
  cin >> subject_quantity;
  for(int i = 0; i < subject_quantity; i++){
    int period_quantity;
    string code, course, professor;
    cin >> code >> period_quantity >> course >> professor;
    subjects[course].push_back(subject_t({period_quantity,code,course,professor}));
    professor_courses[professor].push_back(course);
  }
}
  
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

vector< vector<schedule_t> > population;
void generate_population(int group_size){
  for(int i = 0,group; i < (int)permutations.size(); i++,group = 0){
    do{
      group++;
      vector<schedule_t> person;
      set<ii> allocated_room_schedules;
      for(int j = 0; j < (int)permutations[i].size(); j++){
        set<int> allocated_professor_schedules;
        string professor = professor_index[permutations[i][j]];
        #ifdef DEBUG
          cout << "Professor: " << professor << endl;
        #endif
        for(auto& room : rooms){
          #ifdef DEBUG
            cout << "Room: "; print_room(room);
          #endif
          for(auto& course : professor_courses[professor]){
            if(course.compare(room.course)) continue;
            #ifdef DEBUG
              cout << "Course: " << course << endl;
            #endif
            for(auto& subject : subjects[course]){
              if(professor.compare(subject.professor) || !subject.period_quantity) continue;
              #ifdef DEBUG
                cout << "Subject: "; print_subject(subject);
              #endif
              for(auto& available_schedule : room.available_schedules){
                if(!subject.period_quantity) break;
                ii room_schedule(room.number,available_schedule);
                if(allocated_room_schedules.find(room_schedule) != allocated_room_schedules.end() || allocated_professor_schedules.find(available_schedule) != allocated_professor_schedules.end()) continue;
                #ifdef DEBUG
                  cout << "Available schedule: " << available_schedule << endl;
                #endif
                allocated_room_schedules.insert(room_schedule);
                allocated_professor_schedules.insert(available_schedule);
                schedule_t schedule = schedule_t({room,professor,subject,subject.period_quantity--,available_schedule});
                person.push_back(schedule);
                #ifdef DEBUG
                  print_schedule(schedule);
                #endif
              }
            }
          }
        }
      }
      population.push_back(person);
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && group < group_size);
  }
}

struct schedule_t_comp{
  bool operator()(schedule_t a,schedule_t b){
    int diff = a.professor.compare(b.professor);
    if(diff) return diff < 0;
    diff = a.subject.course.compare(b.subject.course);
    if(diff) return diff < 0;
    diff = a.subject.code.compare(b.subject.code);
    if(diff) return diff < 0;
    return a.room.number < b.room.number;
  }
};
/*
chave first: Sala
chave second; Horário.
Valor: professor, matéria e sala.
*/
set<schedule_t,schedule_t_comp> vis;
map< ii, schedule_t > match;
map<schedule_t,_4i,schedule_t_comp> embryo;

int Aug(schedule_t v){
  if(vis.find(v) != vis.end()) return 0;
  vis.insert(v);
  knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
  uniform_int_distribution<int> distribution(0,1);
  auto dice = bind(distribution, generator);
  if(dice()){    
    if(embryo[v].first != ii(-1,-1) && 
        match.find(embryo[v].first) == match.end() || Aug(match[embryo[v].first])){
      match[embryo[v].first] = v;
      return 1;
    }
    if(embryo[v].second != ii(-1,-1) && 
        match.find(embryo[v].second) == match.end() || Aug(match[embryo[v].second])){
      match[embryo[v].second] = v;
      return 1;
    }
  }
  else{
    if(embryo[v].second != ii(-1,-1) && 
        match.find(embryo[v].second) == match.end() || Aug(match[embryo[v].second])){
      match[embryo[v].second] = v;
      return 1;
    }
    if(embryo[v].first != ii(-1,-1) && 
        match.find(embryo[v].first) == match.end() || Aug(match[embryo[v].first])){
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

vector<schedule_t> cross(vector<schedule_t> & person1, vector<schedule_t> & person2){
  embryo.clear();

  map<string,set<int> > check_repeated_schedule;
  for(auto p : person1) {
    if(check_repeated_schedule.find(p.professor) == check_repeated_schedule.end()) 
      check_repeated_schedule[p.professor] = set<int>();    
    if(check_repeated_schedule[p.professor].find(p.schedule) == check_repeated_schedule[p.professor].end()){  
      embryo[p].first = ii(p.room.number,p.schedule);    
      check_repeated_schedule[p.professor].insert(p.schedule);
    }
    else embryo[p].first = ii(-1,-1);
  }
  for(auto p : person2) {
    if(check_repeated_schedule.find(p.professor) == check_repeated_schedule.end()) 
      check_repeated_schedule[p.professor] = set<int>();    
    if(check_repeated_schedule[p.professor].find(p.schedule) == check_repeated_schedule[p.professor].end()){  
      embryo[p].first = ii(p.room.number,p.schedule);    
      check_repeated_schedule[p.professor].insert(p.schedule);
    }
    else embryo[p].first = ii(-1,-1);
  }
  bipartite_matching();
  vector<schedule_t> schedule;
  for(auto& m : match){
    schedule.push_back(m.second);
  }
  return schedule;
}

void print_room(room_t room){
  cout << room.course << " " << room.number << " ";
  for(auto& available_schedule: room.available_schedules)
    cout << available_schedule << " ";
  cout << endl;
}

void print_rooms(vector<room_t> rooms) {
  for(auto& room: rooms) print_room(room);
}

void print_courses(void) {
  for(auto& course: courses){
    cout << course.first << endl;
    print_rooms(course.second);
  }
}

void print_schedule(schedule_t schedule) {
  cout << schedule.period << " " << schedule.subject.code << " " << schedule.professor << " " << schedule.room.course << " " << schedule.room.number << " " << schedule.schedule << endl;
}

void print_person(vector<schedule_t> person){
  for(auto& schedule : person){
    print_schedule(schedule);
  }
}

void print_population(void){
  for(auto& person : population)
    print_person(person);
}

void print_subject(subject_t subject){
  cout << subject.course << " " << subject.code << " " << subject.professor << " " << subject.period_quantity << endl;
}

void print_subjects(void){
  for(auto& course : subjects){
    cout << course.first << endl;
    for(auto& subject: course.second)
      print_subject(subject);
  }
}

void print_professors(void){
  for(auto& professor : professors){
    cout << professor.first << " ";
    for(auto& schedule_to_avoid : professor.second)
      cout << schedule_to_avoid << " ";
    cout << endl;
  }
}

int main(void){
  read_professors();
  #ifdef DEBUG
    cout << "Professors:" << endl; print_professors(); cout << endl;
  #endif

  read_room_schedules();
  #ifdef DEBUG
    cout << "Rooms:" << endl; print_rooms(rooms); cout << endl;
  #endif

  read_subjects();
  #ifdef DEBUG
    cout << "Subjects:" << endl; print_subjects(); cout << endl;
  #endif

  generate_people_permutation();
  generate_population(POPULATION_SIZE / (int)professors.size());
  #ifdef DEBUG
    print_population();
  #endif
  vector<schedule_t> person = cross(population.front(),population.back());    
  printf("\nPerson 1, size:%d\n",(int)population.front().size());
  print_person(population.front());
  printf("\nPerson 2, size:%d\n",(int)population.back().size());
  print_person(population.back());
  printf("\nChild, size:%d\n",(int)person.size());
  print_person(person);
  return 0;
}
