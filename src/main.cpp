#include <stdio.h>

using namespace std;

#include <iosteam>
#include <vector>
#include <string>
#include <map>

#include "Professor.h"
#include "Group.h"

map<string,Group> groups;
map<string,Professor> professors;

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
  string group_code;
  int room_quantity, room_number, available_schedules;
  cin >> room_quantity;
  for(int i = 0; i < room_quantity; i++){
    cin >> group_code >> room_number >> available_schedules;
    if(groups.find(group_code) == groups.end())
      groups[group_code] = Group(room_number);
    for(int schedule, j = 0; j < available_schedules; j++){
      cin >> schedule;      
      groups[group_code].add_schedule(schedule);
    }
  }
}

void read_subjects() {
  int subject_quantity;
  cin >> subject_quantity;
  for(int i = 0; i < subject_quantity; i++){    
    int period_quantity;
    string subject_code, group_code, professor_name;
    cin >> subject_code >> period_quantity >> group_code >> professor_name;
  }
}

int main(void){
  read_professors();  
  read_room_schedules();
  read_subjects();
  return 0;
}