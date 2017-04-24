/* Arquivo: main.cpp
   Autores: Gabriel Batista Galli, Matheus Antonio Venancio Dall'Rosa e Vladimir Belinski
   Descrição: o presente arquivo faz parte da resolução do Trabalho I do CCR Inteligência Artificial, 2017-1, do curso de
              Ciência da Computação da Universidade Federal da Fronteira Sul - UFFS, o qual consiste na implementação de
              um algoritmo genético para calcular a tabela de horários dos CCRs do curso de Ciência da Computação da UFFS.
              --> main.cpp é o arquivo principal do trabalho, onde é encontrada a implementação do algoritmo genético
*/

// DEFINE PARA EXIBIÇÃO DE MENSAGENS DE DEPURAÇÃO
// #define DEBUG

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

using namespace std;

// PARÂMETROS CONSTANTES PARA O CÓDIGO GENÉTICO
const int POPULATION_SIZE = 50; // TAMANHO DAS POPULAÇÕES
const int MUTATION_RATE = 20; // TAXA DE MUTAÇÃO ENTRE 0 E 100
const int MAX_GENERATIONS = 1000; // NÚMERO MÁXIMO DE GERAÇÕES
const int MAX_UNMODIFIED_GENERATIONS = 100; // NÚMERO MÁXIMO DE GERAÇÕES SEM ALTERAÇÃO DE FITNESS

// "Constantes" de tempo de execução.
int max_person_fitness = -1, max_population_fitness = -1;

// Professores indexados para as permutações.
vector<string> professor_index;

// Map de nomes de professores para os
// horários nos quais ele não deseja
// ministrar aulas.
map<string, vector<int> > professors;

// Map entre professor e cursos que ele ministra.
map<string, vector<string> > professor_courses;

// Vector de todas as salas disponíveis.
vector<room_t> rooms;

// Map de cursos para suas salas.
map<string, vector<room_t> > courses;

// Map de cursos para seus CCRs.
map<string, vector<subject_t> > subjects;

// Embrião utilizado para cruzamento dos indivíduos
// Relaciona triplas de <Professor, Curso, CCR>
// com pares de <Sala, Horário>.
map<schedule_t, set<ii>, schedule_t_comp> embryo;

// Comparação entre duas pessoas para ordenação,
// de maneira crescente, em relação à fitness.
bool person_comp(person_t &a, person_t &b) {
  return a.fitness < b.fitness;
}

// Comparação entre duas salas para ordenação,
// de maneira crescente, em relação à quantidade
// de horários disponíveis, os horários disponíveis,
// o número da sala e o curso.
bool room_comp(room_t &a,room_t &b) {
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

// Função que realiza a leitura dos professores e horários que não desejam
// ministrar aulas a partir do arquivo de entrada.
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

// Função que realiza a leitura dos cursos/semestres, salas e horários
// disponíveis para as salas a partir do arquivo de entrada.
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

// Função que realiza a leitura dos CCRs, períodos, cursos/semestres e
// professores a partir do arquivo de entrada, ou seja, das associações
// dos professores aos CCRs e suas respectivas fases.
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

// Função que monta os horários de um professor (em uma estrutura), a fim
// de serem analisados na função de fitness.
map<string, set<int> > build_professor_schedules(const person_t person){
  map<string, set<int> > professor_schedules;
  for(auto& schedule : person.schedules){
    if (professor_schedules.find(schedule.subject.professor) == professor_schedules.end())
      professor_schedules[schedule.subject.professor] = set<int>();
    professor_schedules[schedule.subject.professor].insert(schedule.schedule);
  }
  return professor_schedules;
}

// Função auxiliar ao cálculo da fitness de um indivíduo.
// Calcula a quantidade de violações da preferência relacionada a
// um professor estar dando aulas em um horário em que não deseja.
int count_schedules_to_avoid_infringements(const string professor, const set<int> schedules){
  int count = 0;
  auto& schedules_to_avoid = professors[professor];
  for(auto& schedule : schedules)
    count += find(schedules_to_avoid.begin(), schedules_to_avoid.end(), schedule) != schedules_to_avoid.end();
  return count;
}

// Função auxiliar ao cálculo da fitness de um indivíduo.
// Calcula a quantidade de violações da preferência relacionada a
// um professor ministrar aulas no período matutino e noturno no
// mesmo dia.
int count_morning_night_infringements(const set<int> schedules){
  int count = 0;
  for(int i = 0; i < 9; i += 2)
    count += (schedules.find(i) != schedules.end() || schedules.find(i + 1) != schedules.end()) &&
      (schedules.find(i + 20) != schedules.end() || schedules.find(i + 21) != schedules.end());
  return count;
}

// Função auxiliar ao cálculo da fitness de um indivíduo.
// Calcula a quantidade de violações da preferência relacionada à
// inexistência de 2 horários consecutivos para um mesmo professor.
int count_consecutive_schedules_infringements(const set<int> schedules){
  int count = 0;
  for(int i = 0; i < 29; i += 2)
    count += schedules.find(i) != schedules.end() && schedules.find(i + 1) != schedules.end();
  return count;
}

// Função auxiliar ao cálculo da fitness de um indivíduo.
// Calcula a quantidade de violações da restrição relacionada à
// existência de aulas de um mesmo professor no último horário de
// uma noite e no primeiro horário da manhã seguinte.
int count_restriction_infringements(const set<int> schedules){
  int count = 0;
  count += schedules.find(21) != schedules.end() && schedules.find(2) != schedules.end();
  count += schedules.find(23) != schedules.end() && schedules.find(4) != schedules.end();
  count += schedules.find(25) != schedules.end() && schedules.find(6) != schedules.end();
  count += schedules.find(27) != schedules.end() && schedules.find(8) != schedules.end();
  return count;
}

// Função que calcula a fitness de um indivíduo.
int fitness(const person_t person){
  int fit = max_person_fitness;
  map<string, set<int> > professor_schedules = build_professor_schedules(person);
  for(auto& professor_schedule : professor_schedules){
    int schedules_to_avoid_infringements = count_schedules_to_avoid_infringements(professor_schedule.first, professor_schedule.second);
    fit -= schedules_to_avoid_infringements;

    int morning_night_infringements = count_morning_night_infringements(professor_schedule.second);
    fit -= (morning_night_infringements * 6);

    int consecutive_schedules_infringements = count_consecutive_schedules_infringements(professor_schedule.second);
    fit -= (consecutive_schedules_infringements + consecutive_schedules_infringements); // * 2

    int restriction_infringements = count_restriction_infringements(professor_schedule.second);
    fit -= (restriction_infringements * 90);
  }
  return fit;
}

// Função que calcula a fitness de uma população.
// A fitness da população é a soma da fitness
// de cada indivíduo. Após isso, ordena os
// indivíduos pela fitness deles.
long set_population_fitness(population_t &population){
  long fit = 0;
  for(auto& person : population.people)
    fit += person.fitness;
  population.fitness = fit;
  sort(population.people.begin(), population.people.end(), person_comp);
  return population.fitness;
}

// Função auxiliar à função que gera a população inicial a ser utilizada ao
// longo do programa.
vector< vector<int> > permutations;
void generate_people_permutation(){
  for(int i = 0; i < (int)professors.size(); i++){
    // A lista check contém todos os professores ainda não sorteados.
    list<int> check;
    // Adcionar à lista de não sorteados todos os professores diferentes de i.
    for(int j = 0; j < (int)professors.size(); j++)
      if(j != i) check.push_back(j);
    // O vector perm representa uma permutação de professores.
    vector<int> perm;
    perm.push_back(i);
    while (!check.empty()) {
      // Sorteio do próximo professor.
      knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
      uniform_int_distribution<int> distribution(0,((int)check.size())-1);
      auto dice = bind(distribution, generator);
      auto it = check.begin();
      // Retirar professor da lista de não sorteados.
      advance(it,dice());
      perm.push_back(*it);
      check.erase(it);
    }
    permutations.push_back(perm);
  }
}

// Função que gera a população inicial a ser utilizada ao longo do programa.
population_t population;
void generate_population(int group_size) {
  #ifdef DEBUG
    cout << "Permutação inicial:" << endl;
    for (auto& p: permutations) {
      cout << "Permutação ";
      for (auto& p2: p) cout << p2 << " ";
      cout << endl;
    }
  #endif
  for (int i = 0,group; i < (int)permutations.size(); i++,group = 0) {
    do {
      #ifdef DEBUG
        cout << "Próxima permutação: ";
        for (auto& p: permutations[i]) cout << p << " ";
        cout << endl;
      #endif
      group++;
      person_t person;
      set<ii> allocated_room_schedules;
      map<subject_t, int, subject_t_comp> allocated_subject_schedules;
      for(int j = 0; j < (int)permutations[i].size(); j++){
        set<int> allocated_professor_schedules;
        string professor = professor_index[permutations[i][j]];
        #ifdef DEBUG
          cout << "Professor: " << professor << endl;
        #endif
        for(auto& room : rooms){
          #ifdef DEBUG
            cout << "Sala: "; print_room(room);
          #endif
          for(auto& course : professor_courses[professor]){
            if(course.compare(room.course)) continue;
            #ifdef DEBUG
              cout << "Curso: " << course << endl;
            #endif
            for(auto& subject : subjects[course]){
              if(professor.compare(subject.professor)) continue;
              if (allocated_subject_schedules.find(subject) == allocated_subject_schedules.end())
                allocated_subject_schedules[subject] = 0;
              #ifdef DEBUG
                cout << "CCR: "; print_subject(subject);
              #endif
              for(auto& available_schedule : room.available_schedules){
                if(allocated_subject_schedules[subject] >= subject.period_quantity) break;
                ii room_schedule(room.number, available_schedule);
                if(allocated_room_schedules.find(room_schedule) != allocated_room_schedules.end()
                    || allocated_professor_schedules.find(available_schedule) != allocated_professor_schedules.end()) continue;
                #ifdef DEBUG
                  cout << "Horário disponível: " << available_schedule << endl;
                #endif
                allocated_room_schedules.insert(room_schedule);
                allocated_professor_schedules.insert(available_schedule);
                schedule_t schedule = schedule_t({room,subject,allocated_subject_schedules[subject]++,available_schedule});
                person.schedules.push_back(schedule);
                #ifdef DEBUG
                  cout << "Montado horário: ";
                  print_schedule(schedule);
                #endif
              }
            }
          }
        }
      }
      #ifdef DEBUG
        cout << "Novo indivíduo criado com sucesso!" << endl << endl;
      #endif
      person.fitness = fitness(person);
      population.people.push_back(person);
    }while(next_permutation(permutations[i].begin(),permutations[i].end()) && group < group_size);
  }
  set_population_fitness(population);
}

// Função de comparação da estrutura de horários diponíveis,
// de maneira crescente, em relação à quantidade de créditos
// e quantidade de horários.
bool available_schedules_comp(available_schedules &a,available_schedules &b) {
  if(a.schedule.subject.period_quantity != b.schedule.subject.period_quantity)
    return a.schedule.subject.period_quantity < b.schedule.subject.period_quantity;
  return a.room_schedule.size() > b.room_schedule.size();
}

// Função de cruzamento de dois indivíduos.
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
  #ifdef DEBUG
    cout << "Embrião:" << endl;
  #endif
  for(auto& e : embryo){
    available_schedules k(e.first);
    #ifdef DEBUG
      cout << e.first.subject.code << " " << e.first.subject.course << " " << e.first.subject.professor << " " << e.first.period << endl;
    #endif
    for(auto& s : e.second){
      k.add_room_schedule(ii(s.first, s.second));
      #ifdef DEBUG
        cout << "\t" << s.first << " " << s.second << endl;
      #endif
    }
    pool.push_back(k);
  }
  #ifdef DEBUG
    cout << endl;
  #endif
  person_t child;
  set<ii> allocated_room_schedules;
  sort(pool.begin(),pool.end(),available_schedules_comp);
  for(auto& e : pool){
    available_schedules k(e.schedule);
    #ifdef DEBUG
      cout << e.schedule.subject.code << " " << e.schedule.subject.course << " " << e.schedule.subject.professor << " " << e.schedule.period << endl;
      for(auto&s : e.room_schedule){
        cout << "\t" << s.first << " " << s.second << endl;
      }
    #endif
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

// Função auxiliar à mutação que verifica se, para dado
// indivíduo e sala, dado horário está disponível.
bool is_allocated_room_schedule(person_t person, room_t room, int available_schedule) {
  for (auto& schedule : person.schedules)
    if (room.number == schedule.room.number && available_schedule == schedule.schedule) return true;
  return false;
}

// Função de mutação de um indivíduo.
// Troca um horário de uma aula.
bool mutate(person_t &person) {
  bool done = false;
  map<room_t, set<int>, room_t_comp> available_room_schedules;
  for (auto& room : rooms)
    for (int available_schedule : room.available_schedules)
      if (!is_allocated_room_schedule(person, room, available_schedule)) {
        if (available_room_schedules.find(room) == available_room_schedules.end())
          available_room_schedules[room] = set<int>();
        available_room_schedules[room].insert(available_schedule);
      }
  for (auto& available_room_schedule : available_room_schedules) {
    for (auto& schedule : person.schedules) {
      if (available_room_schedule.first.course == schedule.subject.course) {
        done = true;
        #ifdef DEBUG
          cout << "\nSubstituindo horário: " << endl;
          print_schedule(schedule);
        #endif
        knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> distribution(0, available_room_schedule.second.size() - 1);
        auto dice = bind(distribution, generator);
        schedule.room = available_room_schedule.first;
        auto begin = available_room_schedule.second.begin();
        advance(begin, dice());
        schedule.schedule = *begin;
        #ifdef DEBUG
          cout << "pela sala " << schedule.room.number << " e horário " << schedule.schedule << endl;
        #endif
      }
      if (done) break;
    }
    if (done) break;
  }
  return done;
}

// Função auxiliar ao sorteio de indivíduos para reprodução.
// Encontra o indivíduo que está no intervalo de fitness sorteado
// (quanto maior a fitness, maior a probabilidade de ser escolhido).
int find_person_with_fitness(const population_t population, const int avoid, const int fitness) {
  long fit = 0;
  for (int p = 0; p < (int) population.people.size(); p++) {
    fit += population.people[p].fitness;
    if (fit >= fitness) {
      if (p != avoid) return p;
      else {
        if (p + 1 < (int) population.people.size()) return p + 1;
        else if (p - 1 >= 0) return p - 1;
      }
    }
  }
  return -1;
}

// Função que evolui a população por dadas gerações.
// Encerra sua execução antes do número dado de gerações
// caso um indivíduo com fitness máxima seja encontrado
// ou o número especificado de gerações sem alteração
// de fitness seja alcançado.
void evolve(int generations) {
  int unmodified_generations = 0;
  bool reached_max_person_fitness = false, reached_max_unmodified_generations = false, max_permutations = false;
  knuth_b mutation_generator(chrono::system_clock::now().time_since_epoch().count());
  uniform_int_distribution<int> mutation_distribution(1, 100);
  auto mutation = bind(mutation_distribution, mutation_generator);
  for(int g = 0; g < generations; g++){
    population_t new_population;
    knuth_b generator(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> distribution(0,population.fitness-1);
    auto dice = bind(distribution, generator);
    while(new_population.people.size() < population.people.size()){
      int f = find_person_with_fitness(population, -1, dice()),
          m = find_person_with_fitness(population, f, dice());
      if (f == -1 || m == -1) {
        cout << "Não há mais permutações possíveis de cruzamento." << endl;
        max_permutations = true;
        break;
      }
      #ifdef DEBUG
        cout << "Índices sorteados: " << f << " " << m << endl;
      #endif
      person_t father = population.people[f];
      person_t mother = population.people[m];
      person_t child = cross(father, mother);
      if (mutation() <= MUTATION_RATE) {
        #ifdef DEBUG
          cout << "\nAntes de mutação:" << endl;
          print_person(child);
        #endif
        #ifndef DEBUG
          mutate(child);
        #endif
        #ifdef DEBUG
          bool mutated = mutate(child);
          if (mutated) {
            cout << "\nDepois de mutação:" << endl;
            print_person(child);
          }
        #endif
      }
      #ifdef DEBUG
        cout << father.schedules.size() << " " << mother.schedules.size() << " " << child.schedules.size() << endl;
      #endif

      if(child.schedules.size() < mother.schedules.size()) {
        if (father.fitness > mother.fitness) new_population.people.push_back(father);
        else new_population.people.push_back(mother);
      } else {
        child.fitness = fitness(child);
        if (child.fitness == max_person_fitness) {
          cout << "\nEncontrado indivíduo com fitness máxima:" << endl;
          print_person(child);
          cout << "Encerrando..." << endl;
          reached_max_person_fitness = true;
          break;
        }
        new_population.people.push_back(child);
      }
    }
    if (reached_max_person_fitness || max_permutations) break;
    set_population_fitness(new_population);
    cout << "Fitness da " << g + 1 << "-ésima geração: " << population.fitness << " de " << max_population_fitness << endl;
    if (new_population.fitness == population.fitness) unmodified_generations++;
    else unmodified_generations = 0;
    if (unmodified_generations == MAX_UNMODIFIED_GENERATIONS) {
      reached_max_unmodified_generations = true;
      cout << "\nMáximo de " << MAX_UNMODIFIED_GENERATIONS << " gerações sem alteração de fitness alcançado." << endl;
      done_print(new_population);
      break;
    }
    population = new_population;
  }
  if (!reached_max_person_fitness && !reached_max_unmodified_generations) done_print(population);
}

// Mensagem exibida ao término de execução.
void done_print(population_t population) {
  cout << "Indivíduo atual com melhor fitness:" << endl;
  print_person(population.people.back());
  cout << "Encerrando..." << endl;
}

// Função que imprime informações referentes a uma sala de aula.
// Utilizada para debug.
void print_room(room_t room){
  cout << room.course << " " << room.number << " ";
  for(auto& available_schedule: room.available_schedules)
    cout << available_schedule << " ";
  cout << endl;
}

// Função que imprime informações referentes às salas de aula.
// Utilizada para debug.
void print_rooms(vector<room_t> rooms) {
  for(auto& room: rooms) print_room(room);
}

// Função que imprime informações referentes aos cursos.
// Utilizada para debug.
void print_courses(void) {
  for(auto& course: courses){
    cout << course.first << endl;
    print_rooms(course.second);
  }
}

// Função que imprime informações referentes a um horário.
// Utilizada para debug.
void print_schedule(schedule_t schedule) {
  cout << schedule.period << " " << schedule.subject.code << " " << schedule.subject.professor << " " << schedule.subject.course << " " << schedule.room.number << " " << schedule.schedule << endl;
}

// Função que imprime informações referentes a um indivíduo.
// Utilizada para debug.
void print_person(person_t person){
  cout << "Indivíduo com fitness " << person.fitness << " de " << max_person_fitness << " e horários:" << endl;
  for(auto& schedule : person.schedules){
    print_schedule(schedule);
  }
}

// Função que imprime informações referentes a população.
// Utilizada para debug.
void print_population(population_t population){
  for(auto& person : population.people)
    print_person(person);
  cout << "Fitness: " << population.fitness << endl;
}

// Função que imprime informações referentes a um CCR.
// Utilizada para debug.
void print_subject(subject_t subject){
  cout << subject.course << " " << subject.code << " " << subject.professor << " " << subject.period_quantity << endl;
}

// Função que imprime informações referentes aos CCRs.
// Utilizada para debug.
void print_subjects(void){
  for(auto& course : subjects){
    cout << course.first << endl;
    for(auto& subject: course.second)
      print_subject(subject);
  }
}

// Função que imprime informações referentes aos professores.
// Utilizada para debug.
void print_professors(void){
  for(auto& professor : professors){
    cout << professor.first << " ";
    for(auto& schedule_to_avoid : professor.second)
      cout << schedule_to_avoid << " ";
    cout << endl;
  }
}

// Função principal do programa.
int main(void){
  read_professors();
  max_person_fitness =
    (   30  // nota pelo professor não dar aula nos horários que não deseja, -1 por horário infringido
      + 30  // nota pelo professor não dar aula no matutino e noturno do mesmo dia, -6 por dia infringido
      + 30  // nota pelo professor não dar aula-faixa, -2 por turno infringido
      + 360 // nota pelo professor não dar aula no último horário de uma noite e no primeiro da manhã seguinte, -210 por infração
    ) * professors.size(); // para cada professor
  max_population_fitness = max_person_fitness * POPULATION_SIZE;

  #ifdef DEBUG
    cout << "Professores:" << endl; print_professors(); cout << endl;
  #endif

  read_room_schedules();
  #ifdef DEBUG
    cout << "Salas:" << endl; print_rooms(rooms); cout << endl;
  #endif

  read_subjects();
  #ifdef DEBUG
    cout << "CCRs:" << endl; print_subjects(); cout << endl;
  #endif

  cout << "Iniciando execução do algoritmo com populações de tamanho " << POPULATION_SIZE << " por, no máximo, " << MAX_GENERATIONS << " gerações." << endl;

  generate_people_permutation();
  generate_population(POPULATION_SIZE / (int)professors.size());
  #ifdef DEBUG
    cout << "População inicial:" << endl;
    print_population(population);
  #endif
  evolve(MAX_GENERATIONS);
  return 0;
}
