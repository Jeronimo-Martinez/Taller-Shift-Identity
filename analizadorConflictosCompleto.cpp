#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>

using namespace std;

// Estructura para representar las producciones de la gramática
struct Produccion {
    int id;
    string ladoIzquierdo;
    string ladoDerecho;
};

// Estructura para analizar una combinación (Tope Pila, Entrada) y detectar conflictos
struct AnalisisCombinacion {
    string topePila;
    string simboloEntrada;
    vector<string> decisiones;
    bool tieneShift = false;
    bool tieneIdentificacion = false;
    bool esConflicto = false;
};

// Estructura para registrar la transformación de la gramática
struct RegistroTransformacion {
    string simboloConflictivo;
    string nuevoNoTerminal;
    vector<Produccion> originales;
    vector<Produccion> transformadas;
};

// funcion para formatear salida
string rellenarEspacios(string texto, size_t ancho) {
    while (texto.length() < ancho) {
        texto += " ";
    }
    return texto;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    // Producciones originales del Módulo / Proyecto
    vector<Produccion> produccionesOriginales = {
        {1, "<S>", "c<A>a"},
        {2, "<A>", "c<B>"},
        {3, "<A>", "d<B>a"},
        {4, "<B>", "d<S>e<B>f"},
        {5, "<B>", "g"}
    };

    vector<AnalisisCombinacion> listaCombinaciones;
    RegistroTransformacion registroTrans;
    registroTrans.originales = produccionesOriginales;
    bool transformacionRegistrada = false;

    int opcion;
    do {
        cout << "\n=========================================================================\n";
        cout << "    ANALIZADOR Y RESOLCION DE CONFLICTOS SHIFT-IDENTIFY\n";
        cout << "=========================================================================\n";
        cout << "1. Registrar Combinación y Decisiones (Tope Pila + Entrada)\n";
        cout << "2. Ver Análisis de Conflictos Detectados y Explicación\n";
        cout << "3. Registrar Transformación de la Gramática (Símbolo Conflictivo y Reglas)\n";
        cout << "4. Comparar Producciones Originales vs. Transformadas\n";
        cout << "5. Salir\n";
        cout << "Seleccione una opción: ";
        cin >> opcion;

        switch (opcion) {

        case 1: {
            string tope, entrada;
            cout << "\n--- REGISTRAR COMBINACIÓN Y DECISIONES ---\n";
            cout << "Ingrese el símbolo en el tope de la pila (ej. <B>): ";
            cin >> tope;
            cout << "Ingrese el símbolo de entrada pendiente (ej. a): ";
            cin >> entrada;

            int pos = -1;
            for (size_t i = 0; i < listaCombinaciones.size(); i++) {
                if (listaCombinaciones[i].topePila == tope && listaCombinaciones[i].simboloEntrada == entrada) {
                    pos = i;
                    break;
                }
            }

            if (pos == -1) {
                AnalisisCombinacion nuevaComb;
                nuevaComb.topePila = tope;
                nuevaComb.simboloEntrada = entrada;
                listaCombinaciones.push_back(nuevaComb);
                pos = listaCombinaciones.size() - 1;
            }

            int opcDec;
            do {
                cout << "\nPara la combinación (Tope: '" << tope << "', Entrada: '" << entrada << "'):\n";
                cout << "  1. Agregar decisión SHIFT\n";
                cout << "  2. Agregar decisión IDENTIFICACIÓN (ej. id2)\n";
                cout << "  3. Finalizar registro de decisiones para esta combinación\n";
                cout << "  Seleccione: ";
                cin >> opcDec;

                if (opcDec == 1) {
                    string d = "SHIFT";
                    if (find(listaCombinaciones[pos].decisiones.begin(), listaCombinaciones[pos].decisiones.end(), d) == listaCombinaciones[pos].decisiones.end()) {
                        listaCombinaciones[pos].decisiones.push_back(d);
                        listaCombinaciones[pos].tieneShift = true;
                    }
                } else if (opcDec == 2) {
                    string codId;
                    cout << "  Ingrese el código de identificación (ej. id2, id3): ";
                    cin >> codId;
                    for (auto &c : codId) c = tolower(c);
                    string d = "IDENTIFICACIÓN (" + codId + ")";
                    if (find(listaCombinaciones[pos].decisiones.begin(), listaCombinaciones[pos].decisiones.end(), d) == listaCombinaciones[pos].decisiones.end()) {
                        listaCombinaciones[pos].decisiones.push_back(d);
                        listaCombinaciones[pos].tieneIdentificacion = true;
                    }
                }
            } while (opcDec != 3);

            if (listaCombinaciones[pos].tieneShift && listaCombinaciones[pos].tieneIdentificacion) {
                listaCombinaciones[pos].esConflicto = true;
                cout << "\n ALERTA! Se ha registrado simultáneamente SHIFT e IDENTIFICACIÓN.\n";
            } else {
                cout << "\n Combinación registrada sin conflicto de ambigüedad.\n";
            }
            break;
        }

        case 2: {
            cout << "\n============================== REVISIÓN DE CONFLICTOS ==============================\n";
            if (listaCombinaciones.empty()) {
                cout << "No hay combinaciones registradas aún.\n";
            } else {
                int contadorConflictos = 0;
                for (size_t i = 0; i < listaCombinaciones.size(); i++) {
                    const auto& c = listaCombinaciones[i];
                    cout << "\nCombinación " << i + 1 << ": [ Tope: '" << c.topePila << "' | Entrada: '" << c.simboloEntrada << "' ]\n";
                    cout << "Decisiones registradas: [ ";
                    for (const auto& d : c.decisiones) cout << d << " ";
                    cout << "]\n";

                    if (c.esConflicto) {
                        contadorConflictos++;
                        cout << "Estado:CONFLICTO SHIFT-IDENTIFY DETECTADO\n";
                        cout << "EXPLICACIÓN Y JUSTIFICACIÓN DE REVISIÓN:\n";
                        cout << "  1. Ambigüedad en la Celda: Los principios de desplazamiento (Shift) y reducción\n"
                             << "     (Identificación) aplican al mismo tiempo para esta combinación.\n";
                        cout << "  2. No Determinismo: El autómata no puede decidir de forma única si debe apilar\n"
                             << "     el símbolo de entrada o desapilar el manejador del tope de la pila.\n";
                        cout << "  3. Impacto en el Reconocimiento: Ejecutar 'Shift' altera la pila de forma distinta\n"
                             << "     que ejecutar 'Reduce', imposibilitando garantizar un análisis sintáctico correcto.\n";
                        cout << "  4. Símbolo del Conflicto: El símbolo en el tope de la pila ('" << c.topePila
                             << "') es el símbolo conflictivo que requiere reestructuración gramatical.\n";
                    } else {
                        cout << "Estado: Transición sin conflicto shift-identify.\n";
                    }
                    cout << string(80, '-') << endl;
                }
                cout << "Total de combinaciones en conflicto que requieren revisión: " << contadorConflictos << endl;
            }
            break;
        }

        case 3: {
            cout << "\n--- REGISTRAR TRANSFORMACIÓN DE LA GRAMÁTICA ---\n";
            cout << "Ingrese el símbolo identificado como CONFLICTIVO (ej. <B>): ";
            cin >> registroTrans.simboloConflictivo;
            cout << "Ingrese el NUEVO símbolo NO TERMINAL a introducir (ej. <NB>): ";
            cin >> registroTrans.nuevoNoTerminal;

            cout << "\nA continuación, ingrese las producciones TRANSFORMADAS una a una:\n";

            int numReglas;
            cout << "¿Cuántas producciones tendrá la gramática transformada?: ";
            cin >> numReglas;

            registroTrans.transformadas.clear();
            for (int i = 1; i <= numReglas; i++) {
                Produccion p;
                p.id = i;
                cout << "  Producción " << i << " - Lado Izquierdo (ej. <S>, <A>, " << registroTrans.nuevoNoTerminal << "): ";
                cin >> p.ladoIzquierdo;
                cout << "  Producción " << i << " - Lado Derecho (ej. d" << registroTrans.nuevoNoTerminal << "a): ";
                cin >> p.ladoDerecho;
                registroTrans.transformadas.push_back(p);
            }

            transformacionRegistrada = true;
            cout << "\n-> Transformación de la gramática registrada exitosamente!\n";
            break;
        }

        case 4: {
            cout << "\n================================ COMPARACIÓN DE GRAMÁTICAS ================================\n";
            cout << rellenarEspacios("PRODUCCIONES ORIGINALES", 35) << " | " << "PRODUCCIONES TRANSFORMADAS" << endl;
            cout << string(80, '=') << endl;

            size_t maxFilas = max(registroTrans.originales.size(), registroTrans.transformadas.size());

            for (size_t i = 0; i < maxFilas; i++) {
                string origStr = "";
                if (i < registroTrans.originales.size()) {
                    const auto& p = registroTrans.originales[i];
                    origStr = to_string(p.id) + ". " + p.ladoIzquierdo + " -> " + p.ladoDerecho;
                }

                string transStr = "";
                if (transformacionRegistrada && i < registroTrans.transformadas.size()) {
                    const auto& p = registroTrans.transformadas[i];
                    transStr = to_string(p.id) + ". " + p.ladoIzquierdo + " -> " + p.ladoDerecho;
                } else if (!transformacionRegistrada) {
                    transStr = "(Aún no registrada - Use opción 3)";
                }

                cout << rellenarEspacios(origStr, 35) << " | " << transStr << endl;
            }
            cout << string(80, '=') << endl;

            if (transformacionRegistrada) {
                cout << "\nDETALLES DE LA TRANSFORMACIÓN:\n";
                cout << "  • Símbolo conflictivo reemplazado: " << registroTrans.simboloConflictivo << endl;
                cout << "  • Nuevo no terminal incorporado:   " << registroTrans.nuevoNoTerminal << endl;
                cout << "  • Regla de enrutamiento añadida:  " << registroTrans.nuevoNoTerminal
                     << " -> " << registroTrans.simboloConflictivo << endl;
            }
            break;
        }

        case 5:
            cout << "Saliendo del programa...\n";
            break;

        default:
            cout << "Opción inválida.\n";
        }

    } while (opcion != 5);

    return 0;
}