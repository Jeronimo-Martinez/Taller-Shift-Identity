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

void apilar(vector<string>& pila, const string& simbolo) {
    pila.push_back(simbolo);
    cout << " Simbolo '" << simbolo << "' apilado exitosamente.\n";
}

void desapilar(vector<string>& pila) {
    if (pila.size() <= 1 && pila.back() == SIMBOLO_VACIO) {
        cout << "-> Error: No se puede retirar el simbolo de pila vacia ("
             << SIMBOLO_VACIO << ").\n";
        return;
    }

    string tope = pila.back();
    pila.pop_back();
    cout << " Simbolo '" << tope << "' retirado del tope.\n";
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // para mostrar el caracter "▼" en la consola correctamente
    vector<string> pila = {SIMBOLO_VACIO};
    int opcion;
    string simbolo;

    do {
        cout << "\n=== SIMULACION DE PILA (RECONOCEDOR) ===";
        cout << "\n1. Mostrar pila";
        cout << "\n2. Apilar simbolo";
        cout << "\n3. Retirar simbolo del tope";
        cout << "\n4. Salir";
        cout << "\nSeleccione una opcion: ";
        cin >> opcion;

        switch (opcion) {
            case 1:
                mostrarPila(pila);
                break;
            case 2:
                cout << "Ingrese el simbolo a apilar: ";
                cin >> simbolo;
                apilar(pila, simbolo);
                break;
            case 3:
                desapilar(pila);
                break;
            case 4:
                cout << "Saliendo del programa...\n";
                break;
            default:
                cout << "Opcion invalida.\n";
        }
    } while (opcion != 4);

    return 0;
}
