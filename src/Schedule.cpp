#include "Schedule.h"

Schedule::Schedule(){}

Schedule::Schedule(Subject _subject,int _room,int _schedule,Professor _professor){
  this->subject = _subject;
  this->professor = _professor;
  this->room = _room, this->schedule = _schedule;  
}