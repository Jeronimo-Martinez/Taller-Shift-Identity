#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>

using namespace std;
const string SIMBOLO_VACIO = "▼";

// Representación de una producción gramatical
struct Produccion {
    int id;
    string ladoIzquierdo; // ej: "<S>"
    string ladoDerecho;   // ej: "(<A><S>)"
};

// Estructura para registrar los conjuntos de Primeros y Siguientes
struct ConjuntosSimbolo {
    string simbolo;
    vector<string> primeros;
    vector<string> siguientes;
};

// Estructura para registrar cada decisión de transición y detectar conflictos
struct RegistroTransicion {
    string topePila;       // Símbolo A en el tope
    string simboloEntrada; // Símbolo x en la entrada
    vector<string> decisiones; // Guarda "SHIFT", "IDENTIFICACION", o ambas si hay conflicto
    string justificacion;
};

string listarConjuto(const vector<string>& lista) {
    string res = "{ ";
    for (size_t i = 0; i < lista.size(); i++) {
        res += lista[i] + (i + 1 < lista.size() ? ", " : " ");
    }
    res += "}";
    return res;
}

// Función para ingresar los elementos de un conjunto
vector<string> registrarConjunto(const string& nombreConjunto, const string& noTerminal) {
    vector<string> conjunto;
    int opcion;

    do {
        cout << "\n  [ " << nombreConjunto << "(" << noTerminal << ") = " << listarConjuto(conjunto) << " ]\n";
        cout << "  1. Agregar un elemento \n";
        cout << "  2. Agregar símbolo de fin de secuencia (⊣)\n";
        cout << "  3. Finalizar este conjunto\n";
        cout << "  Seleccione una opción: ";
        cin >> opcion;

        if (opcion == 1) {
            string elem;
            cout << "     Ingrese el elemento (ej. (, a, b, etc.): ";
            cin >> elem;
            conjunto.push_back(elem);
            cout << " Elemento '" << elem << "' agregado con éxito.\n";
        }
        else if (opcion == 2) {
            conjunto.push_back("⊣");
            cout << " Símbolo '⊣' agregado con éxito.\n";
        }
    } while (opcion != 3);
    return conjunto;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    vector<string> pila = { SIMBOLO_VACIO };
    vector<Produccion> gramatica = {
        {1, "S", "bASB"},
        {2, "S", "b"},
        {3, "A", "dSca"},
        {4, "A", "e"},
        {5, "B", "cAa"},
        {6, "B", "c"},
    };

    // 2. Registro de conjuntos Primeros y Siguientes ingresados por el equipo
    vector<ConjuntosSimbolo> tablaConjuntos;
    vector< RegistroTransicion> tablaTransiciones;

    int opcion;
    do {
        cout << "\n===================================================\n";
        cout << "  ANALIZADOR DE TRANSICIONES Y CONFLICTOS \n";
        cout << "===================================================\n";
        cout << "1. Ver Producciones de la Gramática\n";
        cout << "2. Registrar/Editar Conjuntos (Primeros y Siguientes)\n";
        cout << "3. Ver Conjuntos Registrados\n";
        cout << "4. Registrar / Analizar una Transición (Tope Pila + Entrada)\n";
        cout << "5. Ver Tabla de Transiciones y Alertas de Conflicto\n";
        cout << "6. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
            case 1: {
                cout << "\n--- PRODUCCIONES REGISTRADAS ---\n";
                for (const Produccion& p : gramatica) {
                    cout << p.id << ". " << p.ladoIzquierdo << " -> " << p.ladoDerecho << endl;
                }
                break;
            }

            case 2: {
                string noTerminal;
                cout << "\n--- REGISTRO DE CONJUNTOS PARA UN SÍMBOLO NO TERMINAL ---\n";
                cout << "Ingrese el símbolo no terminal (ej. S o A): ";
                cin >> noTerminal;

                cout << "\n>> Registar elementos de PRIMEROS(" << noTerminal << "):";
                vector<string> primeros = registrarConjunto("PRIMEROS", noTerminal);

                // Cargar Siguientes elemento por elemento
                cout << "\n>> Cargar elementos de SIGUIENTES(" << noTerminal << "):";
                vector<string> siguientes = registrarConjunto("SIGUIENTES", noTerminal);

                ConjuntosSimbolo nuevoConjunto;
                nuevoConjunto.simbolo = noTerminal;
                nuevoConjunto.primeros = primeros;
                nuevoConjunto.siguientes = siguientes;

                // Actualizar si ya existía o agregar nuevo
                bool actualizado = false;
                for (ConjuntosSimbolo& c : tablaConjuntos) {
                    if (c.simbolo == noTerminal) {
                        c = nuevoConjunto;
                        actualizado = true;
                        cout << "\n Conjuntos para " << noTerminal << " actualizados con éxito\n";
                        break;
                    }
                }

                if (!actualizado) {
                    tablaConjuntos.push_back(nuevoConjunto);
                    cout << "\n Conjuntos para " << noTerminal << " registrados con éxito\n";
                }
            }
            break;

            case 3: {
                cout << "\n--- CONJUNTOS PRIMEROS Y SIGUIENTES REGISTRADOS ---\n";
                if (tablaConjuntos.empty()) {
                    cout << "No hay conjuntos registrados aún.\n";
                } else {
                    for (const ConjuntosSimbolo& c : tablaConjuntos) {
                        cout << "Símbolo: " << c.simbolo << endl;
                        cout << "  Primeros:   " << listarConjuto(c.primeros) << endl;
                        cout << "  Siguientes: " << listarConjuto(c.siguientes) << endl;
                    }
                }
            }
            break;

            case 4: {
                string tope, entrada, decision, justificacion;

                cout << "\n--- REGISTRAR NUEVA TRANSICIÓN ---\n";
                cout << "Ingrese el símbolo en el tope de la pila (ej. <S>, <A>, (, b, ▼): ";
                cin >> tope;
                cout << "Ingrese el símbolo de entrada pendiente (ej. (, a, b, ), ⊣): ";
                cin >> entrada;
                cout << "Ingrese la decisión propuesta (SHIFT / IDENTIFICACIÓN): ";
                cin >> decision;

                for (auto &c : decision) c = toupper(c); // pasar a mayusculas

                cout << "Ingrese la justificación de la decisión: ";
                cin.ignore();
                getline(cin, justificacion);

                // Buscar si la transición ya fue registrada previamente para detectar conflictos
                bool encontrado = false;
                for (RegistroTransicion& trans : tablaTransiciones) {
                    if (trans.topePila == tope && trans.simboloEntrada == entrada) {
                        encontrado = true;
                        if (find(trans.decisiones.begin(), trans.decisiones.end(), decision) == trans.decisiones.end()) {
                            trans.decisiones.push_back(decision);
                            trans.justificacion += " | Nueva decisión agregada: " + justificacion;
                        }

                        // ALERTA DE CONFLICTO (Punto 6)
                        if (trans.decisiones.size() > 1) {
                            cout << "\n ALERTA DE CONFLICTO!\n";
                            cout << "La combinación (Tope: '" << tope << "', Entrada: '" << entrada
                                 << "') tiene múltiples decisiones registradas: [ ";
                            for (const string& d : trans.decisiones) cout << d << " ";
                            cout << "].\nInvestigar posible conflicto .\n";
                        } else {
                            cout << " Decisión registrada exitosamente.\n";
                        }
                        break;
                    }
                }

                if (!encontrado) {
                    RegistroTransicion nuevaTrans;
                    nuevaTrans.topePila = tope;
                    nuevaTrans.simboloEntrada = entrada;
                    nuevaTrans.decisiones.push_back(decision);
                    nuevaTrans.justificacion = justificacion;
                    tablaTransiciones.push_back(nuevaTrans);
                    cout << " Transición registrada exitosamente.\n";
                }
            }
            break;

            case 5: {
                cout << "\n================================ TABLA DE TRANSICIONES REGISTRADAS ================================\n";
                if (tablaTransiciones.empty()) {
                    cout << "No hay transiciones registradas aún.\n";
                } else {
                    for (size_t i = 0; i < tablaTransiciones.size(); i++) {
                        const RegistroTransicion& t = tablaTransiciones[i];
                        cout << "Fila " << i + 1 << " | Tope: " << t.topePila
                             << " | Entrada: " << t.simboloEntrada
                             << " | Decisión(es): [ ";
                        for (const string& d : t.decisiones) cout << d << " ";
                        cout << "]";

                        if (t.decisiones.size() > 1) {
                            cout << "CONFLICTO DETECTADO!";
                        }
                        cout << "\n   Justificación: " << t.justificacion << endl;
                        cout << string(85, '-') << endl;
                    }
                }
            }
            break;
            case 6:
                cout << "Saliendo del programa...\n";
                break;

            default:
                cout << "Opción inválida.\n";
        }
    } while (opcion != 6);

    return 0;
}