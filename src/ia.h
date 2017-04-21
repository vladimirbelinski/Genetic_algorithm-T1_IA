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

typedef struct Schedule {
  room_t room;
  string professor;
  subject_t subject;
  int period, schedule;
} schedule_t;

void print_professors(void);

void print_room(room_t);
void print_rooms(vector<room_t>);
void print_courses(void);

void print_subject(subject_t subject);
void print_subjects(void);

void print_schedule(schedule_t);
void print_person(vector<schedule_t>);
void print_population(void);
