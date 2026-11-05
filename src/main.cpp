// Demonstracao da ListaDupla.
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>

#include "lista_dupla.hpp"

template <class T>
void imprimir(const std::string& rotulo, const ListaDupla<T>& lista) {
    std::cout << rotulo << " [";
    bool primeiro = true;
    for (const T& v : lista) {
        std::cout << (primeiro ? "" : ", ") << v;
        primeiro = false;
    }
    std::cout << "] (tamanho " << lista.tamanho() << ")\n";
}

int main() {
    ListaDupla<int> lista{3, 1, 4, 1, 5};
    imprimir("inicial:          ", lista);

    lista.inserir_frente(9);
    lista.inserir_tras(2);
    imprimir("frente 9, tras 2: ", lista);

    // Insercao O(1) no meio usando um iterador obtido por busca.
    auto it = lista.buscar(4);
    lista.inserir(it, 42);
    imprimir("42 antes do 4:    ", lista);

    lista.remover(lista.buscar(4));
    imprimir("remove o 4:       ", lista);

    std::cout << "remove todos os 1: " << lista.remover_valor(1) << " removidos\n";
    imprimir("resultado:        ", lista);

    lista.inverter();
    imprimir("invertida:        ", lista);

    // Iterador bidirecional: funciona com <algorithm> e <numeric>.
    std::cout << "soma (accumulate): " << std::accumulate(lista.begin(), lista.end(), 0) << '\n';
    std::cout << "maximo:            " << *std::max_element(lista.begin(), lista.end()) << '\n';
    std::reverse(lista.begin(), lista.end());
    imprimir("std::reverse:     ", lista);

    std::cout << "de tras pra frente:";
    for (auto r = lista.rbegin(); r != lista.rend(); ++r) std::cout << ' ' << *r;
    std::cout << '\n';

    // Copia profunda e movimento.
    ListaDupla<std::string> nomes{"ana", "bruno", "carla"};
    ListaDupla<std::string> copia = nomes;
    copia.inserir_tras("diego");
    ListaDupla<std::string> movida = std::move(copia);
    imprimir("original:         ", nomes);
    imprimir("movida:           ", movida);
    std::cout << "copia apos move vazia? " << std::boolalpha << copia.vazia() << '\n';

    movida.remover_frente();
    movida.remover_tras();
    std::cout << "frente/tras:       " << movida.frente() << " / " << movida.tras() << '\n';
    return 0;
}
