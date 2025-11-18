#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <climits> // INT_MAX
using namespace std;


struct Movimiento {
    int examen;
    int Bloque; // +1 o -1
    int Sala;   // 0 o +1 (ciclo)
};

struct Evaluacion {
    int bloques;        // objetivo principal
    int penalizacion;   // objetivo secundario
};

inline bool operator<(const Evaluacion& a, const Evaluacion& b) {
    if (a.bloques != b.bloques) return a.bloques < b.bloques;
    return a.penalizacion < b.penalizacion;
}

bool verificarBloqueValido( int examen,int bloqueNuevo,const vector<vector<int>>& AlumnosXPruebas,const map<int, vector<int>>& PruebasXAlumnos, const vector<pair<int,int>>& instancia) {
    if (bloqueNuevo < 0){
        return false;
    }
    for (int alumno : AlumnosXPruebas[examen]) {

        auto i = PruebasXAlumnos.find(alumno);
        if (i == PruebasXAlumnos.end())
            continue;

        const vector<int>& examenesAlumno = i->second;

        for (int otroExamen : examenesAlumno) {
            // Ignorar el mismo examen
            if (otroExamen == examen) continue;

            // Si tiene otro examen en el mismo bloque → NO válido
            if (instancia[otroExamen].first == bloqueNuevo) {
                return false;
            }
        }
    }

    // No hubo conflictos → válido
    return true;
}

// funcion que evalua que tan buena es una instancia
Evaluacion FuncionEvaluacion( const vector<pair<int,int>>& instancia, const vector<map<int,bool>>& bloques, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos){
    vector<int> Penal = {16,8,4,2,1};

    Evaluacion fe;
    fe.bloques = bloques.size();
    fe.penalizacion = 0;

    for (int examen = 0 ; examen < instancia.size() ; examen++) {
        int bloque = instancia[examen].first;
        const vector<int>& alumnos = AlumnosXPruebas[examen];

        for (int x = 1; x <= 5; x++) {
            int bloqueComparar = bloque + x;
            if (bloqueComparar >= bloques.size()) break;

            const auto& examenes = bloques[bloqueComparar];

            for (int alumno : alumnos) {
                auto itPA = PruebasXAlumnos.find(alumno);
                if (itPA == PruebasXAlumnos.end()) continue;
                
                for (int ex2 : itPA->second) {
                    auto it = examenes.find(ex2);
                    if (it != examenes.end() && it->second) {
                        fe.penalizacion += Penal[x-1];
                        goto siguienteBloque;
                    }
                }
            }
            siguienteBloque:;
        }
    }

    return fe;
}

// Calcula la evaluacion de un examen especifico
int FExExamen(const vector<pair<int,int>>& instancia, const vector<map<int,bool>>& bloques, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int examen){
    vector<int> Penal = {16,8,4,2,1};

    const vector<int>& alumnos = AlumnosXPruebas[examen];
    int bloque = instancia[examen].first;
    int FE = 0;

    // bloques hacia atrás
    for (int x = 1; x <= 5; x++) {
        int b = bloque - x;
        if (b < 0) break;

        const auto& examenes = bloques[b];

        for (int alumno : alumnos) {
            auto itPA = PruebasXAlumnos.find(alumno);
            if (itPA == PruebasXAlumnos.end()) continue;

            for (int ex2 : itPA->second) {
                auto it = examenes.find(ex2);
                if (it != examenes.end() && it->second) {
                    FE += Penal[x-1];
                    goto atras_end;
                }
            }
        }
        atras_end:;
    }

    // bloques hacia adelante
    for (int x = 1; x <= 5; x++) {
        int b = bloque + x;
        if (b >= bloques.size()) break;

        const auto& examenes = bloques[b];

        for (int alumno : alumnos) {
            auto itPA = PruebasXAlumnos.find(alumno);
            if (itPA == PruebasXAlumnos.end()) continue;

            for (int ex2 : itPA->second) {
                auto it = examenes.find(ex2);
                if (it != examenes.end() && it->second) {
                    FE += Penal[x-1];
                    goto adelante_end;
                }
            }
        }
        adelante_end:;
    }

    return FE;
}

// Crea una solucion inicial estandar
vector<pair<int,int>> SolucionInicial( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas){
    vector<pair<int,int>> instancia(NE, {-1,-1});
    bloques.push_back({}); 

    for(int examen=0; examen<NE; examen++){
        bool asignado = false;

        for(int bloque=0; !asignado; bloque++){
            if(bloque == (int)bloques.size()){
                bloques.push_back({});
            }

            // intentar todas las salas
            for(int sala=0; sala<NS; sala++){
                bool okSala   = !UsoSalas[sala].count(bloque);
                bool okBloque = verificarBloqueValido(examen, bloque, AlumnosXPruebas, PruebasXAlumnos, instancia);

                if(okSala && okBloque){
                    // asignar
                    instancia[examen] = {bloque, sala};
                    bloques[bloque][examen] = true;
                    UsoSalas[sala][bloque]  = true;
                    asignado = true;
                    break;
                }
            }
        }
    }
    
    return instancia;
}

bool estaEnTabu(int examen, const deque<int>& listaTabu) {
    for (const auto& m : listaTabu)
        if (m == examen)
            return true;
    return false;
}

bool verificarSalaValida(int examen,int nuevaSala,int nuevoBloque,const vector<int>& Salas,const vector<vector<int>>& AlumnosXPruebas,const vector<map<int,bool>>& UsoSalas) {
    int Nalumnos = AlumnosXPruebas[examen].size();

    // 1) Verificar capacidad
    if (Salas[nuevaSala] < Nalumnos) {
        return false;
    }

    // 2) Verificar si la sala está libre en ese bloque
    //    UsoSalas[sala][bloque] = true si está ocupada
    auto it = UsoSalas[nuevaSala].find(nuevoBloque);
    if (it != UsoSalas[nuevaSala].end() && it->second == true) {
        return false;
    }

    // Sala válida
    return true;
}

Movimiento iteracion( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas, vector<pair<int,int>>& instancia, const deque<int>& listaTabu) {
    Movimiento mejorvecino = {-1, 0, 0};
    Evaluacion mejorFE = {INT_MAX, INT_MAX};
    int FE;
    for(int examen = 0; examen < NE; ++examen){
        if(estaEnTabu(examen, listaTabu)){
            continue;
        }
        
        int bloqueActual  = instancia[examen].first; 
        int salaActual    = instancia[examen].second;

        int bloqueInv = bloques.size() - bloqueActual - 1 ;
        int bloqueSig = bloqueActual + 1;

        int salaSig = (salaActual + 1 + rand() % NS) % NS;

        int resta = FExExamen(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos, examen);
        // mov 1 y mov 3
        bool bloqueSigVal = verificarBloqueValido(examen, bloqueSig , AlumnosXPruebas, PruebasXAlumnos, instancia);

        vector<map<int,bool>> bloquesMod = bloques;

        if (bloqueSigVal){
            bloquesMod[bloqueActual][examen]= false;
            if(bloqueSig == bloques.size()){
                bloquesMod.push_back({});
            }
            bloquesMod[bloqueSig][examen]= true;
            if (verificarSalaValida(examen, salaActual, bloqueSig, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] ={bloqueSig, salaActual};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                Evaluacion FEv = FuncionEvaluacion(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos);
                if (FEv < mejorFE){
                    mejorFE = FEv;
                    mejorvecino = {examen, bloqueSig,salaActual};  // e, bloque, sala
                }
            }
            if (verificarSalaValida(examen, salaSig, bloqueSig, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] ={bloqueSig, salaSig};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                Evaluacion FEv = FuncionEvaluacion(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos);
                if (FEv < mejorFE){
                    mejorFE = FEv;
                    mejorvecino = {examen, bloqueSig, salaSig};
                }
            }
            bloquesMod[bloqueActual][examen]= true;
            bloquesMod[bloqueSig].erase(examen);
        }
        // mov 2 mov 4
        bool bloqueInvVal = verificarBloqueValido(examen, bloqueInv , AlumnosXPruebas, PruebasXAlumnos, instancia);

        if (bloqueInvVal){
            bloquesMod[bloqueActual][examen]= false;
            bloquesMod[bloqueInv][examen]= true;
            if (verificarSalaValida(examen, salaActual, bloqueInv, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] ={bloqueInv, salaActual};
                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta ;
                Evaluacion FEv = FuncionEvaluacion(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos);
                if (FEv < mejorFE){
                    mejorFE = FEv;
                    mejorvecino = {examen, bloqueInv, salaActual};   // 0 o +1 (ciclo)}
                }
            }
            if (verificarSalaValida(examen, salaSig, bloqueInv, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] = {bloqueInv, salaSig};
                FE =  FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen)- resta;
                Evaluacion FEv = FuncionEvaluacion(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos);
                if (FEv < mejorFE){
                    mejorFE = FEv;
                    mejorvecino = {examen, bloqueInv, salaSig};
                }
            }   
            bloquesMod[bloqueActual][examen]= true;
            bloquesMod[bloqueInv].erase(examen);
        }
    }

   
    // Aplicar mejor movimiento seleccionado
    int e = mejorvecino.examen;
    if ( e == -1){
        return mejorvecino;
    }
    int bloqueActual = instancia[e].first;
    int salaActual   = instancia[e].second;
    int nuevoBloque  =  mejorvecino.Bloque;
    int nuevaSala    =  mejorvecino.Sala;

    if (nuevoBloque == (int)bloques.size()) bloques.emplace_back();

    // liberar antigua ocupación
    UsoSalas[salaActual][bloqueActual] = false;
    bloques[bloqueActual].erase(e);

    // marcar nueva ocupación
    UsoSalas[nuevaSala][nuevoBloque] = true;
    bloques[nuevoBloque][e] = true;

    // aplicar cambio
    instancia[e] = {nuevoBloque, nuevaSala};


    while (!bloques.empty()) {
        const auto& last = bloques.back();
        if (last.empty()) {
            bloques.pop_back();
        } else {
            // si hay entries pero todas son false (si manejas false en lugar de erase),
            // hay que comprobar values
            bool flag = false;
            for (const auto &p : last) {
                if (p.second) { flag = true; break; }
            }
            if (!flag) bloques.pop_back();
            else break;
        }
    }

    return mejorvecino;
}

vector<pair<int,int>> Resolver( int NE, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas){
    vector<map<int,bool>> UsoSalas(NS), bloques;
    vector<pair<int,int>> instancia =
        SolucionInicial(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas);

    vector<pair<int,int>> MejorInstancia = instancia;
    vector<map<int,bool>> MejorBloques = bloques;    // ← AGREGADO

    Evaluacion FE = FuncionEvaluacion(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos);
    Evaluacion MejorSol = FE;

    cout <<"FE para solucion inicial: " << FE.penalizacion << endl;

    int tam = max(1, NE/2);
    deque<int> listaTabu;

    const int MAX_ITER = 60;
    for (int iter = 0; iter < MAX_ITER; ++iter) {

        Movimiento aplicado = iteracion(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas, instancia, listaTabu );

        FE = FuncionEvaluacion(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos);

        if (FE < MejorSol) {
            MejorSol = FE;
            MejorInstancia = instancia;
            MejorBloques = bloques;   // ← AGREGADO

            cout << "Iter " << iter
                 << " - Nueva mejor FE = " << MejorSol.penalizacion
                 << " bloques " << MejorBloques.size()
                 << " examen: " << aplicado.examen
                 << " bloque: " << aplicado.Bloque
                 << " clase: " << aplicado.Sala
                 << endl;
        }

        listaTabu.push_back(aplicado.examen);
        if ((int)listaTabu.size() > tam){
            listaTabu.pop_front();
        }
    }

    cout << "\nMejor FE final: " << MejorSol.penalizacion << " (bloques " << MejorBloques.size() << ")\n";   // ← usar MejorBloques
    return MejorInstancia;
}

void LeerArchivo(string Nombre, string i){
    ifstream in(Nombre);
    
    int NE;
    in >> NE;
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    cout<< " Numero de examenes: " << NE <<endl;

    map<int, vector<int>> PruebasXAlumnos;   // id_alumno -> lista de pruebas
    vector<vector<int>> AlumnosXPruebas(NE); // id_prueba -> lista de alumnos

    for (int e = 0; e < NE; e++) {
        string linea;
        getline(in, linea);
        stringstream ss(linea);
        string idStr;

        while (getline(ss, idStr, ',')) { // separar por comas
            stringstream ss_id(idStr);
            int id;
            ss_id >> id;
            AlumnosXPruebas[e].push_back(id);
            PruebasXAlumnos[id].push_back(e);
        }
    }

    int NS;
    in >> NS;

    cout<< " Numero de Salas: " << NS <<endl;
    vector<int> Salas(NS);

    for (int s = 0;  s < NS ; s++ ){
        in >> Salas[s];
    }
    in.close();

    vector<pair<int,int>> MejorInstancia = Resolver( NE, AlumnosXPruebas, PruebasXAlumnos, NS, Salas );

    string Salida = "instancias/i"; 
    Salida += i + ".out";
    ofstream out(Salida);

    for (int examen = 0; examen < MejorInstancia.size(); examen++) {
        int bloque = MejorInstancia[examen].first;
        int sala   = MejorInstancia[examen].second;
        out << examen << " " << bloque << " " << sala << "\n";
    }

    out.close();
}

int main() {
    for (int i = 1; i <= 8; i++) {
        cout << "archivo: "<< i <<endl;
        string Nombre = "instancias/i"; 
        Nombre += to_string(i) + ".in";
        LeerArchivo(Nombre , to_string(i));
    }
    return 0;
}