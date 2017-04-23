/* Arquivo: ia.h
   Autores: Gabriel Batista Galli, Matheus Antonio Venancio Dall'Rosa e Vladimir Belinski
   Descrição: o presente arquivo faz parte da resolução do Trabalho I do CCR Inteligência Artificial, 2017-1, do curso de
              Ciência da Computação da Universidade Federal da Fronteira Sul - UFFS, o qual consiste na implementação de
              um algoritmo genético para calcular a tabela de horários dos CCRs do curso de Ciência da Computação da UFFS.
              --> ia.h é o arquivo cabeçalho de main.cpp
*/

#include <map>
#include <vector>

using namespace std;

typedef struct Room {
  int number;
  string course;
  vector<int> available_schedules;
} room_t;

typedef struct Subject {
  int period_quantity;
  string code, course, professor;
} subject_t;

/*
  period é o número da aula (de 1 a N, inclusive, dado na entrada)
  schedule é o horário da semana, de 0 a 29
*/
typedef struct Schedule {
  room_t room;
  subject_t subject;
  int period, schedule;
} schedule_t;

typedef struct Person {
  int fitness = 0;
  vector<schedule_t> schedules;
} person_t;

typedef struct Population {
  long fitness = 0;
  vector<person_t> people;
} population_t;

void print_professors(void);

void print_room(room_t);
void print_rooms(vector<room_t>);
void print_courses(void);

void print_subject(subject_t subject);
void print_subjects(void);

void print_schedule(schedule_t);
void print_person(person_t);
void print_population(population_t);
