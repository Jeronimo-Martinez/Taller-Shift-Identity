#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <windows.h>

using namespace std;

const string SIMBOLO_VACIO = "▼";
const string FIN_SECUENCIA = "⊣";

struct Produccion {
    int id;
    string ladoIzquierdo;
    string ladoDerecho;
};

// Una celda de la tabla: tabla[topePila][simboloEntrada]
struct CeldaTransicion {
    vector<string> decisiones;   // "S", "id1", "id2", "id0"...
    string justificacion;        // fundamento (Primeros / Siguientes)
};

// Resultado del análisis de una celda con conflictos
struct Conflicto {
    string tope;
    string entrada;
    vector<string> decisiones;
    string justificacion;
};

// Registro de la transformación de la gramática
struct Transformacion {
    bool registrada = false;
    string simboloConflictivo;
    string nuevoNoTerminal;
    vector<Produccion> producciones;
};

struct Gramatica {
    string nombre;
    vector<Produccion> producciones;
    vector<string> simbolosEntrada;
    vector<string> simbolosPila;
    map<string, map<string, CeldaTransicion>> tabla;
    Transformacion transformacion;

    string simboloInicial() const { return producciones.front().ladoIzquierdo; }
};


//  UTILIDADES (entrada de datos y formato)

string recortar(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string leerLinea(const string& mensaje) {
    cout << mensaje;
    string s;
    if (!getline(cin, s)) return "0";   // si se cierra la entrada, "0" hace que los menús terminen
    return recortar(s);
}

int leerEntero(const string& mensaje) {
    while (true) {
        string s = leerLinea(mensaje);
        try {
            size_t pos;
            int v = stoi(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << " Entrada no válida: escriba un número entero.\n";
    }
}

// Rellena con espacios contando caracteres UTF-8 (para alinear ▼ y ⊣)
string pad(const string& s, size_t ancho) {
    size_t n = 0;
    for (unsigned char c : s) if ((c & 0xC0) != 0x80) n++;
    return s + string(n < ancho ? ancho - n : 0, ' ');
}

// Divide una hilera en símbolos: "<S>" es un solo símbolo de ser necesario
vector<string> tokenizar(const string& s) {
    vector<string> t;
    size_t i = 0;
    while (i < s.size()) {
        if (s[i] == '<') {
            size_t j = s.find('>', i);
            if (j != string::npos) { t.push_back(s.substr(i, j - i + 1)); i = j + 1; continue; }
        }
        t.push_back(string(1, s[i]));
        i++;
    }
    return t;
}

string unir(const vector<string>& v, const string& sep = "") {
    string r;
    for (size_t i = 0; i < v.size(); i++) r += v[i] + (i + 1 < v.size() ? sep : "");
    return r;
}

string textoPila(const vector<string>& pila) { return "[ " + unir(pila, " ") + " ]"; }

string textoDecisiones(const vector<string>& d) {
    return d.empty() ? "-" : unir(d, ",");
}

bool contiene(const vector<string>& v, const string& x) {
    return find(v.begin(), v.end(), x) != v.end();
}

bool esShift(const string& d) { return d == "S"; }
bool esId(const string& d)    { return d.rfind("id", 0) == 0; }

// Busca una celda sin crearla para no satutar la tabla
const CeldaTransicion* buscarCelda(const Gramatica& g, const string& tope, const string& ent) {
    auto f = g.tabla.find(tope);
    if (f == g.tabla.end()) return nullptr;
    auto h = f->second.find(ent);
    if (h == f->second.end()) return nullptr;
    return &h->second;
}

// Conflicto shift-identify
bool hayConflicto(const CeldaTransicion* c) {
    if (!c) return false;
    bool hayS = false, hayId = false;
    for (const auto& d : c->decisiones) {
        if (esShift(d)) hayS = true;
        if (esId(d)) hayId = true;
    }
    return hayS && hayId;
}
// Gramaticas de ejemplo
Gramatica crearGramatica1() {
    Gramatica g;
    g.nombre = "Gramática 1 (S -> bASB ...)";
    g.producciones = {
        {1, "S", "bASB"}, {2, "S", "b"},
        {3, "A", "dSca"}, {4, "A", "e"},
        {5, "B", "cAa"},  {6, "B", "c"}
    };
    g.simbolosEntrada = {"a", "b", "c", "d", "e", FIN_SECUENCIA};
    g.simbolosPila    = {SIMBOLO_VACIO, "S", "B", "C", "a", "b", "c", "d", "e"};
    return g;
}

Gramatica crearGramatica2() {
    Gramatica g;
    g.nombre = "Gramática 2 (<S> -> c<A>a ...)";
    g.producciones = {
        {1, "<S>", "c<A>a"},
        {2, "<A>", "c<B>"},
        {3, "<A>", "d<B>a"},
        {4, "<B>", "d<S>e<B>f"},
        {5, "<B>", "g"}
    };
    g.simbolosEntrada = {"a", "c", "d", "e", "f", "g", FIN_SECUENCIA};
    g.simbolosPila    = {SIMBOLO_VACIO, "<S>", "<A>", "<B>", "a", "c", "d", "e", "f", "g"};
    return g;
}

Gramatica crearGramatica3() {
    Gramatica g;
    g.nombre = "Gramática 3 (S -> bASB ...)";
    g.producciones = {
            {1, "S", "bASB"},
            {2, "S", "bA"},
            {3, "A", "dSca"},
            {4, "A", "e"},
            {5, "B", "cAa"},
            {6, "B", "c"}

    };
    g.simbolosEntrada = {"a", "c", "d", "e", FIN_SECUENCIA};
    g.simbolosPila    = {SIMBOLO_VACIO, "<S>", "<A>", "<B>", "a", "c", "d", "e"};
    return g;
}

//  PRODUCCIONES

void mostrarProducciones(const Gramatica& g) {
    cout << "\n=== PRODUCCIONES DE: " << g.nombre << " ===\n";
    for (const auto& p : g.producciones)
        cout << "  " << p.id << ". " << p.ladoIzquierdo << " -> " << p.ladoDerecho << "\n";
    cout << "Símbolo inicial: " << g.simboloInicial() << "\n";
    cout << "Símbolos de entrada: { " << unir(g.simbolosEntrada, " ") << " }\n";
    cout << "Símbolos de pila:    { " << unir(g.simbolosPila, " ") << " }\n";
}

// TABLA DE TRANSICIONES

void registrarDecision(Gramatica& g) {
    cout << "\n--- REGISTRAR DECISIÓN (Shift / Identificación) ---\n";
    string tope = leerLinea("Símbolo en el tope de la pila: ");
    string ent  = leerLinea("Símbolo de entrada pendiente: ");

    if (!contiene(g.simbolosPila, tope)) {
        cout << "-> Error: '" << tope << "' no es un símbolo válido de la pila.\n"; return;
    }
    if (!contiene(g.simbolosEntrada, ent)) {
        cout << "-> Error: '" << ent << "' no es un símbolo de entrada válido.\n"; return;
    }

    int t = leerEntero("Tipo de decisión (1: Shift 'S', 2: Identificación 'idN', 3: Aceptación 'id0'): ");
    string cod;
    if (t == 1) cod = "S";
    else if (t == 2) {
        int n = leerEntero("Número de la producción a identificar: ");
        if (n < 1 || n > (int)g.producciones.size()) {
            cout << "-> Error: la producción " << n << " no existe en esta gramática.\n"; return;
        }
        cod = "id" + to_string(n);
    } else if (t == 3) cod = "id0";
    else { cout << "-> Opción de decisión no válida.\n"; return; }

    string just = leerLinea("Justificación (ej. pertenece a Primeros/Siguientes): ");

    CeldaTransicion& c = g.tabla[tope][ent];
    if (contiene(c.decisiones, cod)) {
        cout << "-> Esa decisión ya estaba registrada en la celda; no se duplica.\n";
        return;
    }
    c.decisiones.push_back(cod);
    if (!c.justificacion.empty()) c.justificacion += " | ";
    c.justificacion += "[" + cod + "] " + just;

    if (hayConflicto(&c)) {
        cout << "\n¡ALERTA DE CONFLICTO SHIFT-IDENTIFY!\n"
             << "La celda (Tope: " << tope << ", Entrada: " << ent << ") contiene ahora [ "
             << textoDecisiones(c.decisiones) << " ]: el autómata no sabría si apilar o reducir.\n"
             << "Requiere revisión (ver 'Informe de conflictos').\n";
    } else {
        cout << "-> Decisión registrada con éxito.\n";
    }
}

void consultarTransicion(const Gramatica& g) {
    cout << "\n--- CONSULTA DE TRANSICIÓN ---\n";
    string tope = leerLinea("Símbolo en el tope de la pila: ");
    string ent  = leerLinea("Símbolo de entrada: ");
    const CeldaTransicion* c = buscarCelda(g, tope, ent);

    cout << "\n----------------------------------------------------\n";
    cout << "Tope: " << tope << " | Entrada: " << ent << "\n";
    if (!c || c->decisiones.empty()) {
        cout << "Acción registrada: -\n"
             << "Estado: AUSENCIA DE TRANSICIÓN DEFINIDA (la hilera sería rechazada).\n";
    } else {
        cout << "Acción registrada: " << textoDecisiones(c->decisiones) << "\n";
        if (hayConflicto(c))              cout << "Estado: CONFLICTO SHIFT-IDENTIFY detectado.\n";
        else if (c->decisiones.size() > 1) cout << "Estado: varias decisiones registradas (sin conflicto shift-identify).\n";
        else                              cout << "Estado: transición única y válida.\n";
        cout << "Justificación: " << c->justificacion << "\n";
    }
    cout << "----------------------------------------------------\n";
}

void mostrarMatriz(const Gramatica& g) {
    cout << "\n=============== MATRIZ DE TRANSICIONES: " << g.nombre << " ===============\n";
    cout << pad("Tope / Ent", 12);
    for (const auto& e : g.simbolosEntrada) cout << pad(e, 12);
    cout << "\n" << string(12 * (g.simbolosEntrada.size() + 1), '-') << "\n";
    for (const auto& p : g.simbolosPila) {
        cout << pad(p, 12);
        for (const auto& e : g.simbolosEntrada) {
            const CeldaTransicion* c = buscarCelda(g, p, e);
            string txt = c ? textoDecisiones(c->decisiones) : "-";
            if (hayConflicto(c)) txt += " (!)";
            cout << pad(txt, 12);
        }
        cout << "\n";
    }
    cout << string(12 * (g.simbolosEntrada.size() + 1), '=') << "\n";
    cout << "Leyenda: S = shift | idN = identificar producción N | id0 = aceptación | "
            "- = sin transición (rechazo) | (!) = conflicto\n";
}

void eliminarCelda(Gramatica& g) {
    string tope = leerLinea("Tope de la pila de la celda a limpiar: ");
    string ent  = leerLinea("Símbolo de entrada de la celda a limpiar: ");
    auto f = g.tabla.find(tope);
    if (f != g.tabla.end() && f->second.erase(ent)) cout << "-> Celda limpiada.\n";
    else cout << "-> Esa celda no tenía decisiones registradas.\n";
}


//  DETECCIÓN DE CONFLICTOS

vector<Conflicto> detectarConflictos(const Gramatica& g) {
    vector<Conflicto> lista;
    for (const auto& p : g.simbolosPila) {
        for (const auto& e : g.simbolosEntrada) {
            const CeldaTransicion* c = buscarCelda(g, p, e);
            if (!hayConflicto(c)) continue;
            Conflicto k;
            k.tope = p; k.entrada = e; k.decisiones = c->decisiones; k.justificacion = c->justificacion;
            lista.push_back(k);
        }
    }
    return lista;
}

void listarDecisiones(const Gramatica& g) {
    cout << "\n--- DECISIONES REGISTRADAS EN " << g.nombre << " ---\n";
    int n = 0;
    for (const auto& p : g.simbolosPila)
        for (const auto& e : g.simbolosEntrada) {
            const CeldaTransicion* c = buscarCelda(g, p, e);
            if (c && !c->decisiones.empty()) {
                n++;
                cout << "  (" << p << ", " << e << ") -> " << textoDecisiones(c->decisiones) << "\n";
            }
        }
    if (n == 0) cout << "  (No hay decisiones registradas todavía.)\n";
}

void informeConflictos(const Gramatica& g) {
    cout << "\n============== INFORME DE CONFLICTOS: " << g.nombre << " ==============\n";
    vector<Conflicto> lista = detectarConflictos(g);
    if (lista.empty()) {
        cout << "No se detectaron combinaciones que requieran revisión.\n"
                "(Esto solo cubre las decisiones registradas; revise que la tabla esté completa.)\n";
        return;
    }
    int i = 1;
    for (const auto& k : lista) {
        cout << "\nConflicto " << i++ << ": [ Tope: '" << k.tope << "' | Entrada: '" << k.entrada << "' ]\n";
        cout << "Tipo: SHIFT-IDENTIFY | Decisiones: [ " << textoDecisiones(k.decisiones) << " ]\n";
        cout << "Explicación:\n"
             << "  1. Ambigüedad: en la misma celda aplican a la vez desplazar (Shift) e identificar un manejador.\n"
             << "  2. No determinismo: el autómata no puede decidir si apila el siguiente símbolo o reduce.\n"
             << "  3. Impacto: Shift y reducción dejan la pila distinta; no se puede garantizar un análisis correcto.\n"
             << "  4. Símbolo conflictivo: '" << k.tope << "' es el candidato a reestructurar en la gramática.\n";
        cout << "Justificación registrada: " << k.justificacion << "\n";
        cout << string(78, '-') << "\n";
    }
    cout << "Total de combinaciones que requieren revisión: " << lista.size() << "\n";
}


//  SIMULADOR DE PILA (traza shift / reducción)

struct PasoTraza { string pila, entrada, accion; };

void simularPila(const Gramatica& g) {
    cout << "\n=== SIMULACIÓN MANUAL DEL RECONOCEDOR (" << g.nombre << ") ===\n";
    string cadena = leerLinea("Ingrese la hilera de entrada a analizar: ");
    vector<string> pila = {SIMBOLO_VACIO};
    vector<string> entrada = tokenizar(cadena);
    vector<PasoTraza> historial;
    historial.push_back({textoPila(pila), unir(entrada), "Inicio de la traza"});

    int opcion;
    do {
        int paso = (int)historial.size() - 1;
        string tope = pila.back();
        string sig = entrada.empty() ? FIN_SECUENCIA : entrada[0];

        cout << "\n-----------------------------------------------\n";
        cout << "Paso: " << paso << "\nPila actual: " << textoPila(pila)
             << "\nEntrada pendiente: " << (entrada.empty() ? FIN_SECUENCIA : unir(entrada)) << "\n";

        // Ayuda: lo que dice la tabla para (tope, siguiente símbolo)
        const CeldaTransicion* c = buscarCelda(g, tope, sig);
        cout << "Tabla para (" << tope << ", " << sig << "): ";
        if (!c || c->decisiones.empty()) cout << "sin transición definida\n";
        else if (hayConflicto(c)) cout << "CONFLICTO SHIFT-IDENTIFY [ " << textoDecisiones(c->decisiones) << " ]\n";
        else cout << textoDecisiones(c->decisiones) << "\n";

        if (pila.size() == 2 && pila[1] == g.simboloInicial() && entrada.empty())
            cout << ">>> La pila contiene solo el símbolo inicial y la entrada se agotó: HILERA ACEPTADA.\n";
        cout << "-----------------------------------------------\n";

        cout << "1. Mostrar historial\n2. Apilar símbolo (shift)\n3. Aplicar reducción (identificación)\n"
                "4. Nueva hilera\n0. Volver al menú principal\n";
        opcion = leerEntero("Seleccione una opción: ");

        switch (opcion) {
        case 1:
            cout << "\n=== HISTORIAL DE LA TRAZA ===\n";
            for (size_t i = 0; i < historial.size(); i++)
                cout << "Paso " << i << " | Pila: " << historial[i].pila
                     << " | Entrada: " << (historial[i].entrada.empty() ? FIN_SECUENCIA : historial[i].entrada)
                     << " | Acción: " << historial[i].accion << "\n";
            break;
        case 2: {
            if (entrada.empty()) { cout << "-> Error: no hay más símbolos en la entrada pendiente.\n"; break; }
            string s = entrada[0];
            entrada.erase(entrada.begin());
            pila.push_back(s);
            string desc = "Shift (" + s + ")";
            historial.push_back({textoPila(pila), unir(entrada), desc});
            cout << "-> " << desc << " realizado con éxito.\n";
            break;
        }
        case 3: {
            int id = leerEntero("Número de la producción cuyo manejador está en el tope: ");
            if (id < 1 || id > (int)g.producciones.size()) {
                cout << "-> Error: la producción " << id << " no existe.\n"; break;
            }
            const Produccion& p = g.producciones[id - 1];
            vector<string> manejador = tokenizar(p.ladoDerecho);
            size_t n = manejador.size();
            // No se puede desapilar el fondo ▼
            if (n > pila.size() - 1 ||
                !equal(manejador.begin(), manejador.end(), pila.end() - n)) {
                cout << "-> Error: el tope de la pila no coincide con el manejador '" << p.ladoDerecho
                     << "' de la producción " << id << ". Reducción no aplicada.\n";
                break;
            }
            pila.resize(pila.size() - n);
            pila.push_back(p.ladoIzquierdo);
            string desc = "Reduce (id" + to_string(id) + "): " + p.ladoIzquierdo + " -> " + p.ladoDerecho;
            historial.push_back({textoPila(pila), unir(entrada), desc});
            cout << "-> " << desc << " realizada con éxito.\n";
            break;
        }
        case 4:
            cadena = leerLinea("Nueva hilera de entrada: ");
            pila = {SIMBOLO_VACIO};
            entrada = tokenizar(cadena);
            historial.clear();
            historial.push_back({textoPila(pila), unir(entrada), "Inicio de la traza"});
            break;
        case 0:
            break;
        default:
            cout << "Opción inválida.\n";
        }
    } while (opcion != 0);
}
//   TRANSFORMACIÓN DE LA GRAMÁTICA

void registrarTransformacion(Gramatica& g) {
    Transformacion& t = g.transformacion;
    cout << "\n--- REGISTRAR TRANSFORMACIÓN DE LA GRAMÁTICA ---\n";
    t.simboloConflictivo = leerLinea("Símbolo identificado como CONFLICTIVO: ");
    t.nuevoNoTerminal    = leerLinea("NUEVO no terminal a introducir (ej. <NB>): ");
    int n = leerEntero("¿Cuántas producciones tendrá la gramática transformada?: ");
    if (n < 1) { cout << "-> Debe haber al menos una producción.\n"; return; }

    t.producciones.clear();
    for (int i = 1; i <= n; i++) {
        Produccion p;
        p.id = i;
        p.ladoIzquierdo = leerLinea("  Producción " + to_string(i) + " - Lado izquierdo: ");
        p.ladoDerecho   = leerLinea("  Producción " + to_string(i) + " - Lado derecho: ");
        t.producciones.push_back(p);
    }
    t.registrada = true;
    cout << "\n-> Transformación registrada exitosamente.\n";
}

void compararGramaticas(const Gramatica& g) {
    const Transformacion& t = g.transformacion;
    cout << "\n=============== COMPARACIÓN DE GRAMÁTICAS: " << g.nombre << " ===============\n";
    cout << pad("PRODUCCIONES ORIGINALES", 35) << " | PRODUCCIONES TRANSFORMADAS\n";
    cout << string(80, '=') << "\n";
    size_t filas = max(g.producciones.size(), t.producciones.size());
    for (size_t i = 0; i < filas; i++) {
        string o, tr;
        if (i < g.producciones.size()) {
            const auto& p = g.producciones[i];
            o = to_string(p.id) + ". " + p.ladoIzquierdo + " -> " + p.ladoDerecho;
        }
        if (!t.registrada) { if (i == 0) tr = "(Aún no registrada - use la opción de registrar transformación)"; }
        else if (i < t.producciones.size()) {
            const auto& p = t.producciones[i];
            tr = to_string(p.id) + ". " + p.ladoIzquierdo + " -> " + p.ladoDerecho;
        }
        cout << pad(o, 35) << " | " << tr << "\n";
    }
    cout << string(80, '=') << "\n";
    if (t.registrada) {
        cout << "\nDETALLES DE LA TRANSFORMACIÓN:\n"
             << "  • Símbolo conflictivo reemplazado: " << t.simboloConflictivo << "\n"
             << "  • Nuevo no terminal incorporado:   " << t.nuevoNoTerminal << "\n"
             << "  • Regla de enrutamiento sugerida:  " << t.nuevoNoTerminal << " -> " << t.simboloConflictivo << "\n";
    }
}

//  RESUMEN GENERAL

void resumenGeneral(const Gramatica& g) {
    int definidas = 0, vacias = 0, conflictos = 0;
    for (const auto& p : g.simbolosPila)
        for (const auto& e : g.simbolosEntrada) {
            const CeldaTransicion* c = buscarCelda(g, p, e);
            if (!c || c->decisiones.empty()) vacias++;
            else if (c->decisiones.size() == 1) definidas++;
        }
    conflictos = (int)detectarConflictos(g).size();
    int total = (int)(g.simbolosPila.size() * g.simbolosEntrada.size());
    // las celdas en conflicto cuentan como definidas con varias decisiones
    int multiples = total - definidas - vacias;

    cout << "\n============== RESUMEN GENERAL: " << g.nombre << " ==============\n";
    cout << "  - Producciones registradas:               " << g.producciones.size() << "\n";
    cout << "  - Celdas con transición única:            " << definidas << "\n";
    cout << "  - Celdas con varias decisiones:           " << multiples << "\n";
    cout << "  - Celdas que requieren revisión (conflicto): " << conflictos << "\n";
    cout << "  - Celdas sin definir (provocan rechazo):  " << vacias << " de " << total << " posibles\n";
    cout << "  - Transformación de la gramática:         "
         << (g.transformacion.registrada ? "registrada" : "pendiente") << "\n";
    if (conflictos > 0)
        cout << "-> Hay conflictos: revise el informe y considere transformar la gramática.\n";
}

//  SUBMENÚS Y MENÚ PRINCIPAL

void menuTabla(Gramatica& g) {
    int op;
    do {
        cout << "\n======== TABLA DE TRANSICIONES (" << g.nombre << ") ========\n"
             << "1. Ver alfabetos\n2. Registrar / actualizar una transición\n"
             << "3. Consultar una transición\n4. Ver matriz completa\n"
             << "5. Limpiar una celda\n0. Volver\n";
        op = leerEntero("Seleccione una opción: ");
        switch (op) {
        case 1:
            cout << "\nSímbolos de entrada: { " << unir(g.simbolosEntrada, " ") << " }\n"
                 << "Símbolos de pila:    { " << unir(g.simbolosPila, " ") << " }\n";
            break;
        case 2: registrarDecision(g); break;
        case 3: consultarTransicion(g); break;
        case 4: mostrarMatriz(g); break;
        case 5: eliminarCelda(g); break;
        case 0: break;
        default: cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

void menuConflictos(Gramatica& g) {
    int op;
    do {
        cout << "\n======== DECISIONES Y CONFLICTOS (" << g.nombre << ") ========\n"
             << "1. Registrar decisión shift / identificación\n"
             << "2. Listar decisiones registradas\n"
             << "3. Informe de conflictos (combinaciones a revisar)\n0. Volver\n";
        op = leerEntero("Seleccione una opción: ");
        switch (op) {
        case 1: registrarDecision(g); break;
        case 2: listarDecisiones(g); break;
        case 3: informeConflictos(g); break;
        case 0: break;
        default: cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

void menuTransformacion(Gramatica& g) {
    int op;
    do {
        cout << "\n======== TRANSFORMACIÓN (" << g.nombre << ") ========\n"
             << "1. Registrar transformación de la gramática\n"
             << "2. Comparar producciones originales vs. transformadas\n0. Volver\n";
        op = leerEntero("Seleccione una opción: ");
        switch (op) {
        case 1: registrarTransformacion(g); break;
        case 2: compararGramaticas(g); break;
        case 0: break;
        default: cout << "Opción inválida.\n";
        }
    } while (op != 0);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);   // para mostrar ▼ y ⊣ correctamente
    vector<Gramatica> gramaticas = {crearGramatica1(), crearGramatica2(), crearGramatica3(),};
    int activa = 0;

    int op;
    do {
        Gramatica& g = gramaticas[activa];
        cout << "\n=========================================================================\n"
             << "   RECONOCEDOR SHIFT-IDENTIFY - PROGRAMA FINAL\n"
             << "   Gramática activa: " << g.nombre << "\n"
             << "=========================================================================\n"
             << "1. Cambiar gramática activa\n"
             << "2. Mostrar producciones\n"
             << "3. Simulador de pila (shift / reducción)\n"
             << "4. Tabla de transiciones (registrar / consultar / matriz)\n"
             << "5. Decisiones shift e identificación y conflictos\n"
             << "6. Transformación de la gramática\n"
             << "7. Resumen general\n"
             << "0. Salir\n";
        op = leerEntero("Seleccione una opción: ");
        switch (op) {
        case 1:
            cout << "\n1. " << gramaticas[0].nombre << "\n2. " << gramaticas[1].nombre << "\n";
            {
                int s = leerEntero("Elija gramática (1 o 2): ");
                if (s == 1 || s == 2) { activa = s - 1; cout << "-> Gramática activa cambiada.\n"; }
                else cout << "-> Opción inválida.\n";
            }
            break;
        case 2: mostrarProducciones(g); break;
        case 3: simularPila(g); break;
        case 4: menuTabla(g); break;
        case 5: menuConflictos(g); break;
        case 6: menuTransformacion(g); break;
        case 7: resumenGeneral(g); break;
        case 0: cout << "Saliendo del programa...\n"; break;
        default: cout << "Opción inválida.\n";
        }
    } while (op != 0);

    return 0;
}