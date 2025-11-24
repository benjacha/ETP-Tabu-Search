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
#include <chrono>
#include <climits>

using namespace std;


// =============================================================================
// ESTRUCTURAS DE DATOS
// =============================================================================

/*
Representa un movimiento de un examen a otro bloque y/o sala
*/
struct Movimiento {
    int examen;
    int Bloque; // +1 o -1
    int Sala;   // 0 o +1 (ciclo)
};

/**
 * Representa un movimiento de un examen a otro bloque y/o sala
 */
struct Evaluacion {
    int bloques;        // objetivo principal - cantidad de bloques utilizados
    int penalizacion;   // objetivo secundario - penalización por proximidad + bloques en el que estan
};

/**
 * Representa la evaluación de una solución (función objetivo)
 */
inline bool operator<(const Evaluacion& a, const Evaluacion& b) {
    if (a.bloques != b.bloques) return a.bloques < b.bloques;
    return a.penalizacion < b.penalizacion;
}

Evaluacion operator-(const Evaluacion& a, const Evaluacion& b) {
    Evaluacion r;
    r.penalizacion = a.penalizacion - b.penalizacion;
    r.bloques      = a.bloques - b.bloques;
    return r;
}

// =============================================================================
// FUNCIONES DE VALIDACIÓN
// =============================================================================

/**
 * Verifica si un bloque es válido para un examen (sin conflictos de alumnos)
 * @param examen Examen a verificar
 * @param bloqueNuevo Bloque destino
 * @param AlumnosXPruebas Lista de alumnos por examen
 * @param PruebasXAlumnos Mapa de exámenes por alumno
 * @param instancia Solución actual
 * @return true si el bloque es válido, false si hay conflicto
 */
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

            // Si tiene otro examen en el mismo bloque, NO válido
            if (instancia[otroExamen].first == bloqueNuevo) {
                return false;
            }
        }
    }

    // No hubo conflictos, válido
    return true;
}

/**
 * Verifica si una sala es válida para un examen (capacidad y disponibilidad)
 * @param examen Examen a verificar
 * @param nuevaSala Sala destino
 * @param nuevoBloque Bloque destino
 * @param Salas Capacidades de las salas
 * @param AlumnosXPruebas Lista de alumnos por examen
 * @param UsoSalas Mapa de uso de salas por bloque
 * @return true si la sala es válida, false si no cumple capacidad o está ocupada
 */
bool verificarSalaValida(int examen,int nuevaSala,int nuevoBloque,const vector<int>& Salas,const vector<vector<int>>& AlumnosXPruebas,const vector<map<int,bool>>& UsoSalas) {
    int Nalumnos = AlumnosXPruebas[examen].size();

    // 1) Verificar capacidad
    if (Salas[nuevaSala] < Nalumnos) {
        return false;
    }

    // 2) Verificar si la sala está libre en ese bloque
    auto it = UsoSalas[nuevaSala].find(nuevoBloque);
    if (it != UsoSalas[nuevaSala].end() && it->second == true) {
        return false;
    }

    // Sala válida
    return true;
}

// =============================================================================
// FUNCIONES DE EVALUACIÓN
// =============================================================================

/**
 * Evalúa la calidad de una solución completa
 * @param instancia Solución a evaluar
 * @param bloques Mapa de bloques y exámenes asignados
 * @param AlumnosXPruebas Lista de alumnos por examen
 * @param PruebasXAlumnos Mapa de exámenes por alumno
 * @return Evaluación de la solución
 */
Evaluacion FuncionEvaluacion( const vector<pair<int,int>>& instancia, const vector<map<int,bool>>& bloques, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos){
    vector<int> Penal = {16,8,4,2,1};

    Evaluacion fe;
    fe.bloques = bloques.size();
    fe.penalizacion = 0;

    // Evaluar cada examen
    for (size_t examen = 0; examen < instancia.size(); examen++) {
        int bloque = instancia[examen].first;
        fe.penalizacion += bloque; // Factor de posición

        const vector<int>& alumnos = AlumnosXPruebas[examen];

        // Verificar proximidad con exámenes futuros
        for (int x = 1; x <= 5; x++) {
            int bloqueComparar = bloque + x;
            if (bloqueComparar >= static_cast<int>(bloques.size())) break;

            const auto& examenes = bloques[bloqueComparar];

            for (int alumno : alumnos) {
                auto itPA = PruebasXAlumnos.find(alumno);
                if (itPA == PruebasXAlumnos.end()) continue;
                
                for (int ex2 : itPA->second) {
                    auto it = examenes.find(ex2);
                    if (it != examenes.end() && it->second) {
                        fe.penalizacion += Penal[x-1]; // Aplicar penalización
                        goto siguienteBloque;
                    }
                }
            }
            siguienteBloque:;
        }
    }

    return fe;
}

/**
 * Evalúa el impacto de un examen específico (Evaluación Incremental)
 * @param instancia Solución actual
 * @param bloques Mapa de bloques
 * @param AlumnosXPruebas Lista de alumnos por examen
 * @param PruebasXAlumnos Mapa de exámenes por alumno
 * @param examen Examen a evaluar
 * @return Evaluación del examen específico
 */
Evaluacion FExExamen(const vector<pair<int,int>>& instancia, const vector<map<int,bool>>& bloques, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int examen){
    vector<int> Penal = {16,8,4,2,1};

    const vector<int>& alumnos = AlumnosXPruebas[examen];
    int bloque = instancia[examen].first;
    Evaluacion fe;
    fe.bloques = bloques.size();
    fe.penalizacion = bloque;

    // Verificar proximidad con exámenes en bloques anteriores
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
                    fe.penalizacion += Penal[x-1];
                    goto atras_end;
                }
            }
        }
        atras_end:;
    }

    // Verificar proximidad con exámenes en bloques posteriores
    for (int x = 1; x <= 5; x++) {
        int b = bloque + x;
        if (b >= static_cast<int>(bloques.size())) break;

        const auto& examenes = bloques[b];

        for (int alumno : alumnos) {
            auto itPA = PruebasXAlumnos.find(alumno);
            if (itPA == PruebasXAlumnos.end()) continue;

            for (int ex2 : itPA->second) {
                auto it = examenes.find(ex2);
                if (it != examenes.end() && it->second) {
                    fe.penalizacion += Penal[x-1];
                    goto adelante_end;
                }
            }
        }
        adelante_end:;
    }

    return fe;
}

// =============================================================================
// GENERACIÓN DE SOLUCIONES INICIALES
// =============================================================================

/**
 * Genera solución inicial usando estrategia Greedy (compacta)
 */
vector<pair<int,int>> SolucionInicialGreedy( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas){
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
                bool okSala   = verificarSalaValida(examen, sala, bloque, Salas, AlumnosXPruebas, UsoSalas);
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

/**
 * Genera solución inicial Naive (un bloque por examen)
 */
vector<pair<int,int>> SolucionInicialNaive( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas){
    (void)PruebasXAlumnos;
    vector<pair<int,int>> instancia(NE, {-1,-1});
    for(int i=0; i<NE; i++){
        bloques.push_back({});
        int Nalumnos = AlumnosXPruebas[i].size();

        for(int sala=0; sala<NS; sala++){
            if (Salas[sala] < Nalumnos) {
                continue;
            }

            instancia[i] = {i, sala};
            bloques[i][i] = true;
            UsoSalas[sala][i]  = true;
            break;   
        }
    }
    return instancia;
}

/**
 * Genera solución inicial aleatoria factible
 */
vector<pair<int,int>> SolucionInicialAleatorio( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas){
    vector<pair<int,int>> instancia(NE, {-1,-1});
    bloques.push_back({}); 

    for(int examen=0; examen<NE; examen++){
        bool asignado = false;

        while (!asignado) {
            int bloque = rand() % max(0, (int)bloques.size() + 1);  
            if (bloque == static_cast<int>(bloques.size())) bloques.push_back({});
            int sala = rand() % NS;

            bool okBloque = verificarBloqueValido( examen, bloque, AlumnosXPruebas, PruebasXAlumnos, instancia);
            bool okSala   = verificarSalaValida( examen, sala, bloque, Salas, AlumnosXPruebas, UsoSalas);

            if (okBloque && okSala) {
                instancia[examen] = {bloque, sala};
                bloques[bloque][examen] = true;
                UsoSalas[sala][bloque] = true;
                asignado = true;
            }
        }
    }
    return instancia;
}

// =============================================================================
// FUNCIONES DE BÚSQUEDA TABÚ
// =============================================================================

/**
 * Verifica si un examen está en la lista tabú
 */
bool estaEnTabu(int examen, const deque<int>& listaTabu) {
    for (const auto& m : listaTabu)
        if (m == examen)
            return true;
    return false;
}

/**
 * Realiza una iteración de la búsqueda tabú
 * @return Mejor movimiento aplicado
 */
Movimiento iteracion( int NE, vector<map<int,bool>>& bloques, vector<map<int,bool>>& UsoSalas , const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas, vector<pair<int,int>>& instancia, const deque<int>& listaTabu) {
    Movimiento mejorvecino = {-1, 0, 0};
    Evaluacion mejorFE = {INT_MAX, INT_MAX};
    Evaluacion FE;
    for(int examen = 0; examen < NE; ++examen){
        if(estaEnTabu(examen, listaTabu)){
            continue;
        }
        
        int bloqueActual  = instancia[examen].first; 
        int salaActual    = instancia[examen].second;

        int bloqueInv = bloques.size() - bloqueActual - 1 ;
        int bloqueSig = bloqueActual + 1;
        int bloqueAnt = bloqueActual - 1;

        int salaSig = (salaActual + 1 + rand() % NS) % NS;

        Evaluacion resta = FExExamen(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos, examen);
        // mov 1 y mov 3
        bool bloqueSigVal = verificarBloqueValido(examen, bloqueSig , AlumnosXPruebas, PruebasXAlumnos, instancia);

        vector<map<int,bool>> bloquesMod = bloques;

        if (bloqueSigVal){
            bloquesMod[bloqueActual][examen]= false;
            if(bloqueSig == static_cast<int>(bloques.size())){
                bloquesMod.push_back({});
            }
            bloquesMod[bloqueSig][examen]= true;
            if (verificarSalaValida(examen, salaActual, bloqueSig, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] ={bloqueSig, salaActual};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                
                if (FE < mejorFE){
                    mejorFE = FE;
                    mejorvecino = {examen, bloqueSig,salaActual};  // e, bloque, sala
                }
            }
            if (verificarSalaValida(examen, salaSig, bloqueSig, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] ={bloqueSig, salaSig};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                
                if (FE < mejorFE){
                    mejorFE = FE;
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
                
                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                
                if (FE < mejorFE){
                    mejorFE = FE;
                    mejorvecino = {examen, bloqueInv, salaActual};  
                }
            }
            if (verificarSalaValida(examen, salaSig, bloqueInv, Salas, AlumnosXPruebas, UsoSalas)){
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] = {bloqueInv, salaSig};
                
                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;
                
                if (FE < mejorFE){
                    mejorFE = FE;
                    mejorvecino = {examen, bloqueInv, salaSig};
                }
            }   
            bloquesMod[bloqueActual][examen]= true;
            bloquesMod[bloqueInv].erase(examen);
        }
        // mov 5 mov 6
        bool bloqueAntVal = verificarBloqueValido(examen, bloqueAnt , AlumnosXPruebas, PruebasXAlumnos, instancia);
        if (bloqueAntVal) {
            bloquesMod[bloqueActual][examen] = false;
            bloquesMod[bloqueAnt][examen] = true;
            if (verificarSalaValida(examen, salaActual, bloqueAnt, Salas, AlumnosXPruebas, UsoSalas)) {
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] = {bloqueAnt, salaActual};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;

                if (FE < mejorFE) {
                    mejorFE = FE;
                    mejorvecino = {examen, bloqueAnt, salaActual};
                }
            }

            if (verificarSalaValida(examen, salaSig, bloqueAnt, Salas, AlumnosXPruebas, UsoSalas)) {
                vector<pair<int,int>> vecino = instancia;
                vecino[examen] = {bloqueAnt, salaSig};

                FE = FExExamen(vecino, bloquesMod, AlumnosXPruebas, PruebasXAlumnos, examen) - resta;

                if (FE < mejorFE) {
                    mejorFE = FE;
                    mejorvecino = {examen, bloqueAnt, salaSig};
                }
            }
            bloquesMod[bloqueActual][examen] = true;
            bloquesMod[bloqueAnt].erase(examen);
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

// =============================================================================
// FUNCIONES DE REGISTRO Y RESULTADOS
// =============================================================================

/**
 * Guarda los resultados de una ejecución en archivo
 */
void guardarResultados( int archivo, int inicial, double tiempoInicial, double tiempoIter, int mejorpenalizacion,int bloques , int iteraciones ) {
    string nombre;
    switch (inicial) {
        case 1: nombre = "Greedy.txt"; break;
        case 2: nombre = "Naive.txt"; break;
        case 3: nombre = "Aleatorio.txt"; break;
    }

    ofstream out(nombre , ios::app);

    out << "Archivo: " << archivo << "\n";
    out << "Tiempo Sol Inicial(ms): " << tiempoInicial << "\n";
    out << "Tiempo Iteraciones(ms): " << tiempoIter << "\n";
    out << "Iteraciones: " << iteraciones << "\n";
    out << "Mejor Sol: " << bloques << " "<< mejorpenalizacion << "\n";
    out << "-------------------------------------\n";

    out.close();
}

// =============================================================================
// ALGORITMO PRINCIPAL DE RESOLUCIÓN
// =============================================================================

/**
 * Algoritmo principal de Búsqueda Tabú para resolver el ETP
 * @param NE Número de exámenes
 * @param AlumnosXPruebas Lista de alumnos por examen
 * @param PruebasXAlumnos Mapa de exámenes por alumno
 * @param NS Número de salas
 * @param Salas Capacidades de las salas
 * @param inicial Estrategia de inicialización (1:Greedy, 2:Naive, 3:Aleatorio)
 * @param archivoID Identificador del archivo de instancia
 * @param maxIT Número máximo de iteraciones
 * @return Mejor solución encontrada
 */
vector<pair<int,int>> Resolver( int NE, const vector<vector<int>>& AlumnosXPruebas, const map<int, vector<int>>& PruebasXAlumnos, int NS, const vector<int>& Salas, int inicial, int archivoID, int maxIT){
    vector<map<int,bool>> UsoSalas(NS), bloques;
    vector<pair<int,int>> instancia;
    auto startInit = chrono::high_resolution_clock::now();
    switch (inicial) {
        case 1:
            instancia = SolucionInicialGreedy(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas);
            break;
        case 2:
            instancia = SolucionInicialNaive(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas);
            break;
        case 3:
            instancia = SolucionInicialAleatorio(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas);
            break;
        default:
            cout << "ERROR: numero de solucion inicial invalido" << endl;
            exit(1);
    }
    auto endInit = chrono::high_resolution_clock::now();
    double tiempoInicial = chrono::duration<double, milli>(endInit - startInit).count();

    vector<pair<int,int>> MejorInstancia = instancia;
    vector<map<int,bool>> MejorBloques = bloques;    

    Evaluacion FE = FuncionEvaluacion(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos);
    Evaluacion MejorSol = FE;

    cout <<"FE para solucion inicial: " << FE.penalizacion << endl;

    int tam = max(1, NE/2);
    deque<int> listaTabu;

    auto startIter = chrono::high_resolution_clock::now();
    for (int iter = 0; iter < maxIT; ++iter) {

        Movimiento aplicado = iteracion(NE, bloques, UsoSalas, AlumnosXPruebas, PruebasXAlumnos, NS, Salas, instancia, listaTabu );

        FE = FuncionEvaluacion(instancia, bloques, AlumnosXPruebas, PruebasXAlumnos);

        if (FE < MejorSol) {
            MejorSol = FE;
            MejorInstancia = instancia;
            MejorBloques = bloques;  

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
    auto endIter = chrono::high_resolution_clock::now();
    double tiempoIter = chrono::duration<double, milli>(endIter - startIter).count();
    guardarResultados( archivoID, inicial, tiempoInicial, tiempoIter, MejorSol.penalizacion,MejorBloques.size(), maxIT);

    cout << "\nMejor FE final: " << MejorSol.penalizacion << " (bloques " << MejorBloques.size() << ")\n";  
    return MejorInstancia;
}

// =============================================================================
// LECTURA DE ARCHIVOS Y FUNCIÓN MAIN
// =============================================================================

/**
 * Lee un archivo de instancia y ejecuta el algoritmo
 */
void LeerArchivo(string Nombre, string i, int Idarchivo, int iteraciones) {
    ifstream in(Nombre);
    
    int NE;
    in >> NE;
    in.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Numero de examenes: " << NE << endl;

    // Leer datos de alumnos y exámenes
    map<int, vector<int>> PruebasXAlumnos;  
    vector<vector<int>> AlumnosXPruebas(NE); 

    for (int e = 0; e < NE; e++) {
        string linea;
        getline(in, linea);
        stringstream ss(linea);
        string idStr;

        while (getline(ss, idStr, ',')) { 
            stringstream ss_id(idStr);
            int id;
            ss_id >> id;
            AlumnosXPruebas[e].push_back(id);
            PruebasXAlumnos[id].push_back(e);
        }
    }

    // Leer datos de salas
    int NS;
    in >> NS;

    cout << "Numero de Salas: " << NS << endl;
    vector<int> Salas(NS);

    for (int s = 0; s < NS; s++) {
        in >> Salas[s];
    }
    in.close();
    
    // Ejecutar con las tres estrategias de inicialización
    for (int n = 1; n <= 3; n++) {
        vector<pair<int,int>> MejorInstancia = Resolver(NE, AlumnosXPruebas, PruebasXAlumnos, NS, Salas, n, Idarchivo, iteraciones);
        
        // Guardar solución en archivo de salida
        string Salida = "";
        switch (n) {
            case 1: Salida = "salidasGreedy/i"; break;
            case 2: Salida = "salidasNaive/i"; break;
            case 3: Salida = "salidasAleatoria/i"; break;
        }
        Salida += i + ".out";
        ofstream out(Salida);

        for (size_t examen = 0; examen < MejorInstancia.size(); examen++) {
            int bloque = MejorInstancia[examen].first;
            int sala   = MejorInstancia[examen].second;
            out << examen << " " << bloque << " " << sala << "\n";
        }

        out.close();
    }
}

/**
 * Función principal - Coordina la ejecución de experimentos
 */
int main() {
    // Limpiar archivos de resultados anteriores
    ofstream("Greedy.txt", ios::trunc).close();
    ofstream("Naive.txt", ios::trunc).close();
    ofstream("Aleatorio.txt", ios::trunc).close();
    
    // Configurar semilla aleatoria
    srand(time(0));
    
    // Ejecutar experimentos con diferentes números de iteraciones
    for (int m = 1; m <= 3; m++) {
        int iteraciones;
        switch (m) {
            case 1: iteraciones = 60; break;
            case 2: iteraciones = 500; break;
            case 3: iteraciones = 1000; break;
        }
        
        // Procesar todas las instancias
        for (int i = 1; i <= 8; i++) {
            cout << "Procesando archivo: " << i << endl;
            string Nombre = "instancias/i";
            Nombre += to_string(i) + ".in";
            LeerArchivo(Nombre, to_string(i), i, iteraciones);
        }
    }
    
    return 0;
}