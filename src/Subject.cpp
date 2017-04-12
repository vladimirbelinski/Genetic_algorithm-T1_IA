#include "Subject.h"

Subject::Subject(){}

Subject::Subject(int id,int period_quantity,string code,Group group,Professor professor){
    this->id = id;    
    this->code = code;
    this->group = group;
    this->professor = professor;
    this->period_quantity = period_quantity;
}