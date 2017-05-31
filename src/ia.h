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

// Par de inteiros
typedef pair<int,int> ii;
// Par <Sala, Lista de horários>
typedef pair< int, vector<int> > room_schedules;

// Estrutura para armazenar e representar uma sala de aula.
typedef struct Room {
  int number;
  string course;
  vector<int> available_schedules;
} room_t;

// Estrutura para armazenar e representar um CCR.
typedef struct Subject {
  int period_quantity;
  string code, course, professor;
} subject_t;

// Estrutura para armazenar e representar um horário de aula.
// period é o número da aula (de 1 a N, inclusive, dado na entrada)
// schedule é o horário da semana, de 0 a 29
typedef struct Schedule {
  room_t room;
  subject_t subject;
  int period, schedule;
} schedule_t;

// Estrutura para armazenar e representar um indivíduo.
typedef struct Person {
  int fitness = 0, schedules_to_avoid_infringements = 0, morning_night_infringements = 0, consecutive_schedules_infringements = 0, restriction_infringements = 0;
  vector<schedule_t> schedules;
} person_t;

// Estrutura para armazenar e representar uma população.
typedef struct Population {
  long fitness = 0;
  vector<person_t> people;
} population_t;

// Estrutura para armazenar e representar as
// salas e horários disponíveis para
// uma tripla de <Professor, Curso, CCR>.
struct available_schedules {
  schedule_t schedule;
  vector<ii> room_schedule;

  available_schedules(schedule_t _schedule) {
    schedule = _schedule;
  }

  void add_room_schedule(ii k) {
    room_schedule.push_back(k);
  }
};

void print_professors(void);

void print_room(room_t);
void print_rooms(vector<room_t>);
void print_courses(void);

void print_subject(subject_t subject);
void print_subjects(void);

void print_schedule(schedule_t);
void print_person(person_t);
void print_population(population_t);

void done_print(population_t);

bool room_comp(room_t &,room_t &);

// Estrutura auxiliar de comparação
// de salas para os maps
struct room_t_comp {
  bool operator()(room_t a, room_t b){
    return room_comp(a, b);
  }
};

// Estrutura auxiliar de comparação
// de horários para os maps
struct schedule_t_comp {
  bool operator()(schedule_t a, schedule_t b){
    int diff = a.subject.professor.compare(b.subject.professor);
    if(diff) return diff < 0;
    diff = a.subject.course.compare(b.subject.course);
    if(diff) return diff < 0;
    diff = a.subject.code.compare(b.subject.code);
    if(diff) return diff < 0;
    return a.subject.period_quantity < b.subject.period_quantity;
  }
};

// Estrutura auxiliar de comparação
// de CCRs para os maps
struct subject_t_comp {
  bool operator()(subject_t a, subject_t b){
    int diff = a.course.compare(b.course);
    if (diff) return diff < 0;
    diff = a.code.compare(b.code);
    if (diff) return diff < 0;
    diff = a.professor.compare(b.professor);
    if(diff) return diff < 0;
    return a.period_quantity < b.period_quantity;
  }
};
