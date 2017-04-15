#include "Subject.h"

Subject::Subject(){}

Subject::Subject(int id,int period_quantity,string code,string course){
    this->id = id;
    this->code = code;
    this->course = course;
    this->period_quantity = period_quantity;
}