#include <iostream>
#include <vector>
#include <string>
#include <windows.h>

using namespace std;

const string SIMBOLO_VACIO = "▼";

void mostrarPila(const vector<string>& pila) {
    cout << "\nEstado de la pila: [ ";
    for (const string& simbolo : pila) {
        cout << simbolo << " ";
    }
    cout << "]\n";
}
string stringPila(const vector<string>& pila) {
    string resultado = "[ ";
    for (const string& elem : pila) {
        resultado += elem + " ";
    }
    resultado += "]";
    return resultado;
}

void apilar(vector<string>& pila, const string& simbolo) {
    pila.push_back(simbolo);
    cout << " Simbolo '" << simbolo << "' apilado exitosamente.\n";
}

void desapilar(vector<string>& pila) {
    if (pila.empty() || (pila.size() == 1 && pila.back() == SIMBOLO_VACIO)) {
        cout << "-> Error: No se puede retirar el simbolo de pila vacia (" << SIMBOLO_VACIO << ").\n";
        return;
    }

    string tope = pila.back();
    pila.pop_back();
    cout << " Simbolo '" << tope << "' retirado del tope.\n";
}

void mostrarHistorial(const vector<vector<string>>& historial) {
    cout << "\n=== HISTORIAL DE LA TRAZA ===\n";
    for (int i = 0; i < historial.size(); i++) {
        cout << "Paso " << i << " | "
             << "Pila: " << historial[i][0] << " | "
             << "Entrada: " << historial[i][1] << " | "
             << "Acción: " << historial[i][2] << endl;
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // para mostrar el caracter "▼" en la consola correctamente
    vector<string> pila = { SIMBOLO_VACIO };
    string entrada;
    vector<vector<string>> historial; //historial de la traza  paso:( estado pila ,  estado entrada , operacion )
    cout << "\n=== SIMULACION MANUAL DEL RECONOCEDOR) ===\n";
    cout << "Ingrese la hilera de entrada a analizar ej: (((b)a(a))(b))";
    cin >> entrada;

    historial.push_back({stringPila(pila),entrada,"Inicio de la Traza"});

    int opcion;
    int paso = 0;

    do {

        cout << "\n ----------------------------------------------- \n";
        cout << "Paso: " << paso << endl;
        cout << "Pila Actual: " << historial[paso][0] << endl;
        cout << "Entrada pendiente: " << historial[paso][1] << endl;
        cout << "\n ----------------------------------------------- \n\n";

        cout << "\n1. Mostrar Historial";
        cout << "\n2. Apilar simbolo (shift)";
        cout << "\n3. Aplicar Reduccion";
        cout << "\n4. Salir";
        cout << "\nSeleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
        case 1:
            mostrarHistorial(historial);
            break;
        case 2: {
            if (entrada.empty()) {
                cout << "-> Error: No hay más símbolos en la entrada pendiente.\n";
            } else {
                // Toma el primer carácter de la entrada
                string simbolo(1, entrada[0]);
                entrada.erase(0, 1); // eliminar el símbolo de la entrada

                pila.push_back(simbolo); // Apilar
                paso++;

                string desc = "Shift (" + simbolo + ")";
                historial.push_back({stringPila(pila), entrada, desc});
                cout << "-> " << desc << " realizado con éxito.\n";
            }
            break;
        }
        case 3: {
            int numDesapilar;
            string noTerminal;
            string reglaAplicada;

            cout << "¿Cuántos elementos del tope forman el manejador a desapilar?: ";
            cin >> numDesapilar;

            // Validar que no se intente borrar el símbolo de pila vacía (▼)
            if (numDesapilar >= pila.size()) {
                cout << "-> Error: No se pueden desapilar " << numDesapilar
                     << " elementos. Supera los elementos disponibles sobre " << SIMBOLO_VACIO << ".\n";
                break;
            }

            cout << "Ingrese el no terminal al que se reduce (ej. <S> o <A>): ";
            cin >> noTerminal;
            cout << "Ingrese la regla o producción aplicada (ej. <S> -> (b)): ";
            cin.ignore();
            getline(cin, reglaAplicada);

            // Desapilar los elementos del manejador
            for (int i = 0; i < numDesapilar; i++) {
                pila.pop_back();
            }

            // Apilar el no terminal sustituto
            pila.push_back(noTerminal);
            paso++;

            string desc = "Reduce: " + reglaAplicada;
            historial.push_back({stringPila(pila), entrada, desc});
            cout << "-> " << desc << " realizada con éxito.\n";
            break;
        }
        case 4:
            cout << "Saliendo del programa...\n";
            break;
        default:
            cout << "Opcion invalida.\n";
        }
    } while (opcion != 4);

    return 0;
}