// Testes da ListaDupla. Usa assert e std::list como oraculo.
#undef NDEBUG
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "lista_dupla.hpp"

namespace {

int testes_passados = 0;

template <class F>
void executar(const char* nome, F teste) {
    teste();
    ++testes_passados;
    std::cout << "[ok] " << nome << '\n';
}

// Compara conteudo nos dois sentidos e o tamanho.
template <class T>
bool igual(const ListaDupla<T>& lista, const std::list<T>& oraculo) {
    return lista.tamanho() == oraculo.size() &&
           std::equal(lista.begin(), lista.end(), oraculo.begin(), oraculo.end()) &&
           std::equal(lista.rbegin(), lista.rend(), oraculo.rbegin(), oraculo.rend());
}

void teste_vazia() {
    ListaDupla<int> l;
    assert(l.vazia());
    assert(l.tamanho() == 0);
    assert(l.begin() == l.end());
    assert(l.rbegin() == l.rend());
    assert(!l.contem(1));
    assert(l.buscar(1) == l.end());
    l.inverter();
    assert(l.vazia());
    l.limpar();
    assert(l.vazia());

    bool lancou = false;
    try {
        l.remover_frente();
    } catch (const std::out_of_range&) {
        lancou = true;
    }
    assert(lancou);
    lancou = false;
    try {
        (void)l.tras();
    } catch (const std::out_of_range&) {
        lancou = true;
    }
    assert(lancou);
}

void teste_um_elemento() {
    ListaDupla<int> l;
    l.inserir_tras(7);
    assert(l.tamanho() == 1);
    assert(l.frente() == 7 && l.tras() == 7);
    assert(std::next(l.begin()) == l.end());
    assert(std::prev(l.end()) == l.begin());
    l.inverter();
    assert(l.frente() == 7);
    l.remover_tras();
    assert(l.vazia());
    l.inserir_frente(8);
    l.remover_frente();
    assert(l.vazia() && l.begin() == l.end());
}

void teste_push_pop() {
    ListaDupla<int> l;
    for (int i = 0; i < 5; ++i) l.inserir_tras(i);
    for (int i = 1; i <= 3; ++i) l.inserir_frente(-i);
    assert(igual(l, std::list<int>{-3, -2, -1, 0, 1, 2, 3, 4}));
    l.remover_frente();
    l.remover_tras();
    assert(igual(l, std::list<int>{-2, -1, 0, 1, 2, 3}));
    l.frente() = 100;
    l.tras() = 200;
    assert(igual(l, std::list<int>{100, -1, 0, 1, 2, 200}));
}

void teste_inserir_remover_iterador() {
    ListaDupla<int> l{1, 2, 4, 5};
    auto it = l.buscar(4);
    auto novo = l.inserir(it, 3);
    assert(*novo == 3);
    assert(*it == 4);  // iteradores existentes continuam validos
    assert(igual(l, std::list<int>{1, 2, 3, 4, 5}));

    auto seguinte = l.remover(novo);
    assert(*seguinte == 4);
    assert(igual(l, std::list<int>{1, 2, 4, 5}));

    l.inserir(l.end(), 6);
    l.inserir(l.begin(), 0);
    assert(igual(l, std::list<int>{0, 1, 2, 4, 5, 6}));

    auto fim = l.remover(std::next(l.begin()), std::prev(l.end()));
    assert(*fim == 6);
    assert(igual(l, std::list<int>{0, 6}));
}

void teste_duplicados() {
    ListaDupla<int> l{1, 2, 1, 3, 1, 1};
    assert(l.buscar(1) == l.begin());
    assert(l.remover_valor(1) == 4);
    assert(igual(l, std::list<int>{2, 3}));
    assert(l.remover_valor(42) == 0);
    ListaDupla<int> iguais{5, 5, 5};
    assert(iguais.remover_valor(5) == 3);
    assert(iguais.vazia());
}

void teste_inverter() {
    ListaDupla<int> l{1, 2, 3, 4, 5};
    auto it3 = l.buscar(3);
    l.inverter();
    assert(igual(l, std::list<int>{5, 4, 3, 2, 1}));
    assert(*it3 == 3);  // os nos nao se movem
    l.inverter();
    assert(igual(l, std::list<int>{1, 2, 3, 4, 5}));
    ListaDupla<int> dois{1, 2};
    dois.inverter();
    assert(igual(dois, std::list<int>{2, 1}));
}

void teste_copia_movimento() {
    ListaDupla<std::string> a{"x", "y", "z"};
    ListaDupla<std::string> b = a;  // copia
    b.inserir_tras("w");
    assert(a.tamanho() == 3 && b.tamanho() == 4);

    ListaDupla<std::string> c = std::move(b);  // movimento
    assert(b.vazia() && b.begin() == b.end());
    assert(c.tamanho() == 4 && c.tras() == "w");
    b.inserir_tras("reuso");  // objeto movido continua utilizavel
    assert(b.frente() == "reuso");

    a = c;  // atribuicao por copia
    assert(a == c);
    const auto& mesmo = a;
    a = mesmo;  // auto-atribuicao
    assert(a.tamanho() == 4);
    c = std::move(a);  // atribuicao por movimento
    assert(a.vazia() && c.tamanho() == 4);

    ListaDupla<std::string> vazia;
    c = vazia;  // copia de lista vazia
    assert(c.vazia());
    c = {"p", "q"};
    assert(c.tamanho() == 2 && c.frente() == "p");

    ListaDupla<std::string> d{"1"};
    swap(c, d);
    assert(c.tamanho() == 1 && d.tamanho() == 2);
    assert(d.tras() == "q" && c.frente() == "1");

    static_assert(std::is_nothrow_move_constructible_v<ListaDupla<std::string>>);
    static_assert(std::is_nothrow_move_assignable_v<ListaDupla<std::string>>);
}

void teste_tipo_somente_movel() {
    ListaDupla<std::unique_ptr<int>> l;
    l.inserir_tras(std::make_unique<int>(1));
    l.emplace_frente(new int(0));
    l.emplace_tras(std::make_unique<int>(2));
    int esperado = 0;
    for (const auto& p : l) assert(*p == esperado++);
    ListaDupla<std::unique_ptr<int>> m = std::move(l);
    assert(m.tamanho() == 3 && l.vazia());
}

void teste_algoritmos_stl() {
    ListaDupla<int> l{5, 3, 8, 1, 9, 2};
    static_assert(std::is_same_v<std::iterator_traits<ListaDupla<int>::iterator>::iterator_category,
                                 std::bidirectional_iterator_tag>);
    assert(*std::max_element(l.begin(), l.end()) == 9);
    assert(std::count_if(l.cbegin(), l.cend(), [](int x) { return x % 2 == 0; }) == 2);
    std::reverse(l.begin(), l.end());
    assert(igual(l, std::list<int>{2, 9, 1, 8, 3, 5}));

    // conversao iterator -> const_iterator e comparacao mista
    ListaDupla<int>::const_iterator ci = l.begin();
    assert(ci == l.cbegin());

    std::vector<int> v;
    std::copy(l.begin(), l.end(), std::back_inserter(v));
    assert((v == std::vector<int>{2, 9, 1, 8, 3, 5}));

    ListaDupla<int> destino;
    std::copy(v.rbegin(), v.rend(), std::back_inserter(destino));
    assert(igual(destino, std::list<int>{5, 3, 8, 1, 9, 2}));

    int soma = 0;
    for (int x : l) soma += x;
    assert(soma == 28);

    const ListaDupla<int>& cl = l;
    assert(cl.contem(8) && !cl.contem(7));
    assert(*cl.buscar(8) == 8);
}

// Operacoes aleatorias comparadas com std::list.
void teste_aleatorio_oraculo() {
    std::mt19937 gerador(12345);
    for (int rodada = 0; rodada < 20; ++rodada) {
        ListaDupla<int> l;
        std::list<int> oraculo;
        for (int passo = 0; passo < 5000; ++passo) {
            int op = static_cast<int>(gerador() % 9);
            int valor = static_cast<int>(gerador() % 100);
            switch (op) {
                case 0: l.inserir_tras(valor); oraculo.push_back(valor); break;
                case 1: l.inserir_frente(valor); oraculo.push_front(valor); break;
                case 2:
                    if (!oraculo.empty()) { l.remover_tras(); oraculo.pop_back(); }
                    break;
                case 3:
                    if (!oraculo.empty()) { l.remover_frente(); oraculo.pop_front(); }
                    break;
                case 4: {  // insere antes da primeira ocorrencia (ou no fim)
                    auto it = l.buscar(valor);
                    auto ito = std::find(oraculo.begin(), oraculo.end(), valor);
                    l.inserir(it, valor + 1000);
                    oraculo.insert(ito, valor + 1000);
                    break;
                }
                case 5: {  // remove a primeira ocorrencia
                    auto it = l.buscar(valor);
                    auto ito = std::find(oraculo.begin(), oraculo.end(), valor);
                    assert((it == l.end()) == (ito == oraculo.end()));
                    if (it != l.end()) { l.remover(it); oraculo.erase(ito); }
                    break;
                }
                case 6:
                    if (passo % 50 == 0) { l.inverter(); oraculo.reverse(); }
                    break;
                case 7:
                    assert(l.contem(valor) ==
                           (std::find(oraculo.begin(), oraculo.end(), valor) != oraculo.end()));
                    break;
                default:
                    if (passo % 100 == 0) {
                        l.remover_valor(valor);
                        oraculo.remove(valor);
                    }
                    break;
            }
            assert(l.tamanho() == oraculo.size());
        }
        assert(igual(l, oraculo));
        ListaDupla<int> copia(l);
        assert(igual(copia, oraculo));
    }
}

void teste_grande_volume() {
    constexpr int n = 200000;
    ListaDupla<int> l;
    for (int i = 0; i < n; ++i) l.inserir_tras(i);
    assert(l.tamanho() == static_cast<std::size_t>(n));
    l.inverter();
    assert(l.frente() == n - 1 && l.tras() == 0);
    // remove os pares percorrendo uma vez
    for (auto it = l.cbegin(); it != l.cend();) it = (*it % 2 == 0) ? l.remover(it) : std::next(it);
    assert(l.tamanho() == static_cast<std::size_t>(n / 2));
    assert(std::all_of(l.begin(), l.end(), [](int x) { return x % 2 == 1; }));
}

}  // namespace

int main() {
    executar("lista vazia", teste_vazia);
    executar("um elemento", teste_um_elemento);
    executar("push/pop nas pontas", teste_push_pop);
    executar("inserir/remover por iterador", teste_inserir_remover_iterador);
    executar("duplicados", teste_duplicados);
    executar("inverter", teste_inverter);
    executar("copia e movimento", teste_copia_movimento);
    executar("tipo somente movel", teste_tipo_somente_movel);
    executar("algoritmos da STL", teste_algoritmos_stl);
    executar("aleatorio vs std::list", teste_aleatorio_oraculo);
    executar("grande volume", teste_grande_volume);
    std::cout << testes_passados << " testes passaram\n";
    return 0;
}
