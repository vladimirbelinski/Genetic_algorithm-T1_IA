#include "Class.h"

Class::Class(){}

Class::Class(int _number,Subject _subject,Professor _professor){
  this->number = _number;
  this->subject = _subject;
  this->professor = _professor;
}