/* Arquivo: main.cpp
   Autores: Gabriel Batista Galli, Matheus Antonio Venancio Dall'Rosa e Vladimir Belinski
   Descrição: o presente arquivo faz parte da resolução do Trabalho I do CCR Inteligência Artificial, 2017-1, do curso de
              Ciência da Computação da Universidade Federal da Fronteira Sul - UFFS, o qual consiste na implementação de
              um algoritmo genético para calcular a tabela de horários dos CCRs do curso de Ciência da Computação da UFFS.
              --> main.cpp é o arquivo principal do trabalho, onde é encontrada a implementação do algoritmo genético
*/

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

#define DEBUG

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

map<string, set<int> > build_professor_schedules(const person_t person){
  map<string, set<int> > professor_schedules;
  for(auto& schedule : person.schedules){
    if (professor_schedules.find(schedule.subject.professor) == professor_schedules.end())
      professor_schedules[schedule.subject.professor] = set<int>();
    professor_schedules[schedule.subject.professor].insert(schedule.schedule);
  }
  return professor_schedules;
}

int count_schedules_to_avoid_infringements(const string professor, const set<int> schedules){
  int count = 0;
  for(auto& schedule : schedules){
    auto& schedules_to_avoid = professors[professor];
    count += find(schedules_to_avoid.begin(), schedules_to_avoid.end(), schedule) != schedules_to_avoid.end();
  }
  return count;
}

int count_morning_night_infringements(const set<int> schedules){
  int count = 0;
  for(int i = 0; i < 9; i += 2)
    count += (schedules.find(i) != schedules.end() || schedules.find(i + 1) != schedules.end()) &&
      (schedules.find(i + 20) != schedules.end() || schedules.find(i + 21) != schedules.end());
  return count;
}

int count_consecutive_schedules_infringements(const set<int> schedules){
  int count = 0;
  for(int i = 0; i < 29; i += 2)
    count += schedules.find(i) != schedules.end() && schedules.find(i + 1) != schedules.end();
  return count;
}

int count_restriction_infringements(const set<int> schedules){
  int count = 0;
  count += schedules.find(21) != schedules.end() && schedules.find(2) != schedules.end();
  count += schedules.find(23) != schedules.end() && schedules.find(4) != schedules.end();
  count += schedules.find(25) != schedules.end() && schedules.find(6) != schedules.end();
  count += schedules.find(27) != schedules.end() && schedules.find(8) != schedules.end();
  return count;
}

int fitness(const person_t person){
  int fit =
        (   30  // nota pelo professor não dar aula nos horários que não deseja, -1 por horário infringido
          + 30  // nota pelo professor não dar aula no matutino e noturno do mesmo dia, -6 por dia infringido
          + 30  // nota pelo professor não dar aula-faixa, -2 por turno infringido
          + 840 // nota pelo professor não dar aula no último horário de uma noite e no primeiro da manhã seguinte, -210 por infração
        ) * professors.size(); // para cada professor
  map<string, set<int> > professor_schedules = build_professor_schedules(person);
  for(auto& professor_schedule : professor_schedules){
    int schedules_to_avoid_infringements = count_schedules_to_avoid_infringements(professor_schedule.first, professor_schedule.second);
    fit -= schedules_to_avoid_infringements;

    int morning_night_infringements = count_morning_night_infringements(professor_schedule.second);
    fit -= (morning_night_infringements * 6);

    int consecutive_schedules_infringements = count_consecutive_schedules_infringements(professor_schedule.second);
    fit -= (consecutive_schedules_infringements + consecutive_schedules_infringements); // * 2

    int restriction_infringements = count_restriction_infringements(professor_schedule.second);
    fit -= (restriction_infringements * 210);
  }
  return fit;
}

long fitness(population_t population){
  long fit = 0;
  for(auto& person : population.people) fit += fitness(person);
  return fit;
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

population_t population;
void generate_population(int group_size){
  for(int i = 0,group; i < (int)permutations.size(); i++,group = 0){
    do{
      group++;
      person_t person;
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
              if(professor.compare(subject.professor)) continue;
              int period = 0;
              #ifdef DEBUG
                cout << "Subject: "; print_subject(subject);
              #endif
              for(auto& available_schedule : room.available_schedules){
                if(period >= subject.period_quantity) break;
                ii room_schedule(room.number,available_schedule);
                if(allocated_room_schedules.find(room_schedule) != allocated_room_schedules.end()
                    || allocated_professor_schedules.find(available_schedule) != allocated_professor_schedules.end()) continue;
                #ifdef DEBUG
                  cout << "Available schedule: " << available_schedule << endl;
                #endif
                allocated_room_schedules.insert(room_schedule);
                allocated_professor_schedules.insert(available_schedule);
                schedule_t schedule = schedule_t({room,subject,period++,available_schedule});
                person.schedules.push_back(schedule);
                #ifdef DEBUG
                  print_schedule(schedule);
                #endif
              }
            }
          }
        }
      }
      population.people.push_back(person);
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && group < group_size);
  }
}

struct schedule_t_comp{
  bool operator()(schedule_t a,schedule_t b){
    int diff = a.subject.professor.compare(b.subject.professor);
    if(diff) return diff < 0;
    diff = a.subject.course.compare(b.subject.course);
    if(diff) return diff < 0;
    diff = a.subject.code.compare(b.subject.code);
    return diff < 0;
  }
};
/*
chave first: Sala
chave second; Horário.
Valor: professor, matéria e sala.
*/
//set<schedule_t,schedule_t_comp> vis;
//map< ii, schedule_t > match;
map<schedule_t,set<ii>,schedule_t_comp> embryo;

struct available_schedules{
  schedule_t schedule;
  vector<ii> room_schedule;
  available_schedules(schedule_t _schedule){
    schedule = _schedule;
  }

  void add_room_schedule(ii k){
    room_schedule.push_back(k);
  }
};

bool available_schedules_comp(available_schedules a,available_schedules b){
  if(a.schedule.subject.period_quantity != b.schedule.subject.period_quantity)
    return a.schedule.subject.period_quantity < b.schedule.subject.period_quantity;
  return a.room_schedule.size() < b.room_schedule.size();
}

/*
int Aug(schedule_t v){
  if(vis.find(v) != vis.end()) return 0;
  vis.insert(v);
  knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
  uniform_int_distribution<int> distribution(0,1);
  auto dice = bind(distribution, generator);
  if(dice()){
    cout << "sorteado 1: " << v.subject.professor << " " << v.period << " " << v.room.number << " " << v.schedule << endl;
    for(int k = 0; k < (int)embryo[v].size(); k++){
      if(embryo[v][k] != ii(-1,-1) &&
          (match.find(embryo[v][k]) == match.end() || Aug(match[embryo[v][k]]))){
        match[embryo[v][k]] = v;
        return 1;
      }
    }
  }
  else{
    cout << "sorteado 0: " << v.subject.professor << " " << v.period << " " << v.room.number << " " << v.schedule << endl;
    for(int k = (int)embryo[v].size() - 1; k >= 0; k--){
      if(embryo[v][k] != ii(-1,-1) &&
          (match.find(embryo[v][k]) == match.end() || Aug(match[embryo[v][k]]))){
        match[embryo[v][k]] = v;
        return 1;
      }
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
*/
person_t cross(person_t & father, person_t & mother){
  embryo.clear();
  map<string, set<int> > check_repeated_schedule;
  for(auto schedule : father.schedules) {
    string professor = schedule.subject.professor;
    if(check_repeated_schedule.find(professor) == check_repeated_schedule.end())
      check_repeated_schedule[professor] = set<int>();
    if(check_repeated_schedule[professor].find(schedule.schedule) == check_repeated_schedule[professor].end()){
      check_repeated_schedule[professor].insert(schedule.schedule);
      embryo[schedule].insert(ii(schedule.room.number,schedule.schedule));
    }
  }
  for(auto schedule : mother.schedules) {
    string professor = schedule.subject.professor;
    if(check_repeated_schedule.find(professor) == check_repeated_schedule.end())
      check_repeated_schedule[professor] = set<int>();
    if(check_repeated_schedule[professor].find(schedule.schedule) == check_repeated_schedule[professor].end()){
      check_repeated_schedule[professor].insert(schedule.schedule);
      embryo[schedule].insert(ii(schedule.room.number,schedule.schedule));
    }
  }
  vector<available_schedules> pool;
  cout << "Embryo" << endl;
  for(auto& e : embryo){
    available_schedules k(e.first);
    cout << e.first.subject.code << " " << e.first.subject.professor << " " << e.first.period << endl;
    for(auto& s : e.second){
      k.add_room_schedule(ii(s.first, s.second));
      cout << "   " << s.first << " " << s.second << endl;
    }
    pool.push_back(k);
  }
  cout << endl;

  set<ii> allocated_room_schedules;
  sort(pool.begin(),pool.end(),available_schedules_comp);
  person_t child;
  for(auto& e : pool){
    available_schedules k(e.schedule);
    cout << e.schedule.subject.code << " " << e.schedule.subject.professor << " " << e.schedule.period << endl;
    for(auto&s : e.room_schedule){
      cout << "   " << s.first << " " << s.second << endl;
    }
    for(int i = 0; i < e.schedule.subject.period_quantity && e.room_schedule.size();){
      knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
      uniform_int_distribution<int> distribution(0,((int)e.room_schedule.size())-1);
      auto dice = bind(distribution, generator);
      int idx = dice();
      if(allocated_room_schedules.find(e.room_schedule[idx]) == allocated_room_schedules.end()){
        allocated_room_schedules.insert(e.room_schedule[idx]);
        int r = e.room_schedule[idx].first, s = e.room_schedule[idx].second;
        child.schedules.push_back(schedule_t({room_t({r}),e.schedule.subject,i,s}));
        i++;
      }
      e.room_schedule.erase(e.room_schedule.begin()+idx);
    }
  }
  return child;
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
  cout << schedule.period << " " << schedule.subject.code << " " << schedule.subject.professor << " " << schedule.room.course << " " << schedule.room.number << " " << schedule.schedule << endl;
}

void print_person(person_t person){
  cout << "Person with fitness " << person.fitness << " and schedules:" << endl;
  for(auto& schedule : person.schedules){
    print_schedule(schedule);
  }
}

void print_population(population_t population){
  for(auto& person : population.people)
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
    print_population(population);
  #endif
  //for(int i = 1; i < (int)population.people.size(); i++){
    person_t father = population.people.front();
    person_t mother = population.people.back();
    person_t child = cross(father, mother);
    printf("\nFather, size: %d\n",(int)father.schedules.size());
    print_person(father);
    printf("\nMother, size: %d\n",(int)mother.schedules.size());
    print_person(mother);
    printf("\nChild, size: %d\n",(int)child.schedules.size());
    print_person(child);
    cout << "Fitness of the father: " << fitness(father) << endl;
    cout << "Fitness of the mother: " << fitness(mother) << endl;
    cout << "Fitness of the child: " << fitness(child) << endl;
  //  if(child.schedules.size() == mother.schedules.size()) break;
//  }
  return 0;
}
