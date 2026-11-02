#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <algorithm>
#include <windows.h>

using namespace std;

const string SIMBOLO_VACIO = "▼";
const string FIN_SECUENCIA = "⊣";

// Representación de una celda en la Tabla de Transiciones
struct CeldaTransicion {
    vector<string> decisiones; // Puede contener "S" (Shift), "id1", "id2", "id0"...
    string justificacion;      // Fundamento teórico basado en Primeros/Siguientes
};

// Estructura de la Gramática
struct Produccion {
    int id;
    string ladoIzquierdo;
    string ladoDerecho;
};

// Función auxiliar para formatear la salida de decisiones de una celda
string obtenerTextoDecisiones(const vector<string>& decs) {
    if (decs.empty()) return "-"; // si no hay transicion definida muestra -
    string res = "";
    for (size_t i = 0; i < decs.size(); i++) {
        res += decs[i] + (i + 1 < decs.size() ? "," : "");
    }
    return res;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    vector<string> simbolosEntrada = { "a", "b","c" ,"d","e",FIN_SECUENCIA};
    vector<string> simbolosPila = { SIMBOLO_VACIO,"S","A","B","C","a", "b","c" ,"d","e"};

    vector<Produccion> gramatica = {
        {1, "S", "bASB"},
        {2, "S", "b"},
        {3, "A", "dSca"},
        {4, "A", "e"},
        {5, "B", "cAa"},
        {6, "B", "c"},
    };

    // Estructura para la Tabla de Transiciones: tabla[topePila][simboloEntrada]
    map<string, map<string, CeldaTransicion>> tablaTransiciones;

    int opcion;
    do {
        cout << "\n=========================================================================\n";
        cout << "    CONSTRUCCIÓN Y VERIFICACIÓN DE LA TABLA \n";
        cout << "=========================================================================\n";
        cout << "1. Ver Alfabetos (Símbolos de Pila y de Entrada)\n";
        cout << "2. Registrar / Actualizar una Transición en la Tabla\n";
        cout << "3. Consultar una Transición Específica (Tope Pila + Entrada)\n";
        cout << "4. Ver Matriz Completa de la Tabla de Transiciones\n";
        cout << "5. Ver Resumen de Transiciones y Decisiones a Revisar (Conflictos/Vacías)\n";
        cout << "6. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {
        case 1: {
            cout << "\n---  ALFABETOS DEL RECONOCEDOR ---\n";
            cout << "Símbolos de Entrada (Terminales + Fin de Secuencia): { ";
            for (const auto& s : simbolosEntrada) cout << s << " ";
            cout << "}\n";

            cout << "Símbolos de Pila (No Terminales + Terminales + Pila Vacía): { ";
            for (const auto& s : simbolosPila) cout << s << " ";
            cout << "}\n";
            cout << "Nota: El símbolo '⊣' representa el fin de entrada y NO se coloca en la pila.\n";
            break;
        }

        case 2: { // Registrar transiciones (Shift, Identificación)
            string tope, entrada, tipoAccion, idRegla, just;

            cout << "\n--- REGISTRAR / ACTUALIZAR TRANSICIÓN ---\n";
            cout << "Ingrese Símbolo en Tope de Pila: ";
            cin >> tope;
            cout << "Ingrese Símbolo de Entrada Pendiente: ";
            cin >> entrada;

            // Validar si los símbolos pertenecen a los alfabetos
            if (find(simbolosPila.begin(), simbolosPila.end(), tope) == simbolosPila.end()) {
                cout << " Error: El tope '" << tope << "' no es un símbolo válido de la pila.\n";
                break;
            }
            if (find(simbolosEntrada.begin(), simbolosEntrada.end(), entrada) == simbolosEntrada.end()) {
                cout << " Error: La entrada '" << entrada << "' no es un símbolo de entrada válido.\n";
                break;
            }

            cout << "Tipo de Transición (1: Shift 'S', 2: Identificación 'id', 3: Aceptación 'id0'): ";
            int t;
            cin >> t;

            string codAccion = "";
            if (t == 1) {
                codAccion = "S";
            } else if (t == 2) {
                cout << "Ingrese el número de la producción a identificar (ej. 1, 2, 3, 4): ";
                cin >> idRegla;
                codAccion = "id" + idRegla;
            } else if (t == 3) {
                codAccion = "id0";
            } else {
                cout << "Opción de acción no válida.\n";
                break;
            }

            cout << "Ingrese la justificación teórica (ej. Pertenece a Primeros/Siguientes): ";
            cin.ignore();
            getline(cin, just);

            // Agregar la decisión a la celda
            auto& celda = tablaTransiciones[tope][entrada];
            if (find(celda.decisiones.begin(), celda.decisiones.end(), codAccion) == celda.decisiones.end()) {
                celda.decisiones.push_back(codAccion);
                if (!celda.justificacion.empty()) celda.justificacion += " | ";
                celda.justificacion += "[" + codAccion + "] " + just;
            }

            if (celda.decisiones.size() > 1) {
                cout << "\nALERTA DE CONFLICTO SHIFT-IDENTIFY! \n";
                cout << "La combinación (Tope: " << tope << ", Entrada: " << entrada
                     << ") ahora contiene " << celda.decisiones.size() << " decisiones: [ "
                     << obtenerTextoDecisiones(celda.decisiones) << " ]. Requiere revisión.\n";
            } else {
                cout << "-> Transición registrada con éxito.\n";
            }
            break;
        }

        case 3: { // REQUERIMIENTO 4: Consultar una transición
            string tope, entrada;
            cout << "\n--- CONSULTA DE TRANSICIÓN ---\n";
            cout << "Ingrese Símbolo en Tope de Pila: ";
            cin >> tope;
            cout << "Ingrese Símbolo de Entrada: ";
            cin >> entrada;

            auto celda = tablaTransiciones[tope][entrada];
            cout << "\n----------------------------------------------------\n";
            cout << "Tope Pila: " << tope << " | Entrada: " << entrada << endl;
            cout << "Acción Registrada: " << obtenerTextoDecisiones(celda.decisiones) << endl;

            // REQUERIMIENTO 3: Distinguir ausencia de transición definida
            if (celda.decisiones.empty()) {
                cout << "Estado: AUSENCIA DE TRANSICIÓN DEFINIDA (Provoca Rechazo ).\n";
            } else if (celda.decisiones.size() > 1) {
                cout << "Estado: CONFLICTO SHIFT-IDENTIFY DETECTADO.\n";
                cout << "Justificación: " << celda.justificacion << endl;
            } else {
                cout << "Estado: Transición Única y Válida.\n";
                cout << "Justificación: " << celda.justificacion << endl;
            }
            cout << "----------------------------------------------------\n";
            break;
        }

        case 4: { // IMPRESIÓN MATRICIAL COMPLETA
            cout << "\n================================ MATRIZ DE TRANSICIONES ================================\n";
            cout << left << setw(12) << "Tope / Ent";
            for (const auto& e : simbolosEntrada) {
                cout << setw(12) << e;
            }
            cout << "\n" << string(75, '-') << endl;

            for (const auto& p : simbolosPila) {
                cout << left << setw(12) << p;
                for (const auto& e : simbolosEntrada) {
                    string textoCelda = obtenerTextoDecisiones(tablaTransiciones[p][e].decisiones);
                    if (tablaTransiciones[p][e].decisiones.size() > 1) {
                        textoCelda += " ⚠️"; // Marcar conflicto visualmente
                    }
                    cout << setw(12) << textoCelda;
                }
                cout << endl;
            }
            cout << string(75, '=') << endl;
            break;
        }

        case 5: { // REQUERIMIENTO 6: Resumen de decisiones que requieren revisión
            cout << "\n============================== RESUMEN DE REVISIÓN Y CONFLICTOS ==============================\n";
            int conflictos = 0;
            int vacias = 0;
            int definidas = 0;

            cout << "\n1. CELDAS CON CONFLICTO SHIFT-IDENTIFY (Múltiples decisiones en la misma celda):\n";
            for (const auto& p : simbolosPila) {
                for (const auto& e : simbolosEntrada) {
                    auto celda = tablaTransiciones[p][e];
                    if (celda.decisiones.size() > 1) {
                        conflictos++;
                        cout << "  • Tope: '" << p << "' | Entrada: '" << e
                             << "' ➔ Decisiones: [" << obtenerTextoDecisiones(celda.decisiones)
                             << "] | Justificación: " << celda.justificacion << endl;
                    } else if (celda.decisiones.size() == 1) {
                        definidas++;
                    } else {
                        vacias++;
                    }
                }
            }
            if (conflictos == 0) cout << "  (No se han detectado conflictos shift-identify en las celdas registradas).\n";

            cout << "\n2. ESTADÍSTICAS DE LA TABLA:\n";
            cout << "  • Transiciones Únicas Definidas: " << definidas << endl;
            cout << "  • Celdas con Conflicto a Revisar: " << conflictos << endl;
            cout << "  • Celdas Sin Definir (Rechazo):   " << vacias << " de "
                 << (simbolosPila.size() * simbolosEntrada.size()) << " posibles.\n";
            cout << "===============================================================================================\n";
            break;
        }

        case 6:
            cout << "Saliendo del programa...\n";
            break;
        default:
            cout << "Opción inválida.\n";
        }
    } while (opcion != 6);

    return 0;
}
