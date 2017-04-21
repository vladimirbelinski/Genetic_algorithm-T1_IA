void generate_population(int group_size){
  for(int i = 0,g; i < (int)permutations.size(); i++,g = 0){
    do{
      g++;
      vector<Schedule> person;
      set<ii> is_allocated_room_schedule;
      for(int j = 0; j < (int)permutations[i].size(); j++){
        Professor p = professors[professor_index[permutations[i][j]]];
        set<int> is_allocated_professor_schedule;
        for(auto& subjects : p.subjects){//turnos e semestres para um professor.
          for(auto& subject : subjects.second){//disciplinas para turnos e semestres.
            int periods = 0;
            vector< room_schedules > rooms_from_curr_course;
            for(auto& c : courses[subject.course]){
              rooms_from_curr_course.push_back(room_schedules(c.first,c.second));
            }
            sort(rooms_from_curr_course.begin(),rooms_from_curr_course.end(),room_comp);

            for(auto& c : rooms_from_curr_course){//turnos e semestres.
              for(auto& class_room_schedule : c.second){//salas de cada curso.
                ii schedule = ii(c.first,class_room_schedule);
                if(is_allocated_room_schedule.find(schedule) == is_allocated_room_schedule.end()){
                  //cout << p.name << " " << subject.code << " " << subject.course << " " << c.first << " " << class_room_schedule << endl;
                  is_allocated_room_schedule.insert(schedule);
                  person.push_back(Schedule(Class(periods++,subject,p),c.first,class_room_schedule));
                  is_allocated_professor_schedule.insert(class_room_schedule);
                }
                if(periods == subject.period_quantity) break;
              }
              if(periods == subject.period_quantity) break;
            }
            if(periods == subject.period_quantity) break;
          }
        }
      }
      population.push_back(person);
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && g < group_size);
  }
}
