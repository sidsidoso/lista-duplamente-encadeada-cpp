# Lista Duplamente Encadeada em C++

[![CI](https://github.com/sidsidoso/lista-duplamente-encadeada-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/sidsidoso/lista-duplamente-encadeada-cpp/actions/workflows/ci.yml)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
[![Licenca: MIT](https://img.shields.io/badge/licen%C3%A7a-MIT-green.svg)](LICENSE)

Implementação header-only de uma **lista duplamente encadeada genérica** (`ListaDupla<T>`) em C++17, com nó sentinela, gerenciamento de memória RAII, regra dos 5 e iteradores bidirecionais compatíveis com `range-for` e com a biblioteca `<algorithm>`. Projeto da disciplina de Estrutura de Dados.

## Conceito

Cada nó guarda um valor e dois ponteiros: para o anterior (`ant`) e para o próximo (`prox`). Um **nó sentinela** (sem valor) fecha a lista em anel: o `prox` do sentinela é o primeiro elemento, o `ant` é o último, e `end()` aponta para o próprio sentinela.

```
         +--------------------------------------------------------+
         |                                                        |
         v                                                        |
   +-----------+     +-------+     +-------+     +-------+        |
   | sentinela |<--->|   3   |<--->|   1   |<--->|   4   |<-------+
   |  (end())  |     |begin()|     |       |     |       |
   +-----------+     +-------+     +-------+     +-------+
         ^                                           |
         +-------------------------------------------+
```

Como **nunca há ponteiros nulos** no anel, inserir ou remover em qualquer posição (inclusive nas pontas e na lista vazia) é sempre o mesmo código de 4 atribuições de ponteiros:

```
inserir X antes de B:            remover X:

  A <---> B                        A <---> X <---> B
                                   
  A <---> X <---> B                A <-----------> B   (delete X)
```

## Complexidade

| Operação                                   | Tempo   | Observação                                   |
|--------------------------------------------|---------|----------------------------------------------|
| `inserir_frente` / `inserir_tras`          | O(1)    | `push_front`/`push_back` também disponíveis  |
| `remover_frente` / `remover_tras`          | O(1)    | lançam `std::out_of_range` se vazia          |
| `inserir(it, v)` / `emplace(it, ...)`      | O(1)    | insere antes de `it`                         |
| `remover(it)`                              | O(1)    | retorna iterador para o seguinte             |
| `frente` / `tras`                          | O(1)    |                                              |
| `tamanho` / `vazia`                        | O(1)    | contador mantido a cada operação             |
| `buscar` / `contem`                        | O(n)    | busca linear                                 |
| `remover_valor`                            | O(n)    | remove todas as ocorrências                  |
| `inverter`                                 | O(n)    | troca ponteiros, não copia valores           |
| `limpar` / destrutor                       | O(n)    |                                              |
| cópia                                      | O(n)    | cópia profunda                               |
| movimento / `trocar`                       | O(1)    | `noexcept`, apenas religa o sentinela         |
| **Espaço**                                 | O(n)    | 2 ponteiros extras por elemento              |

## Decisões de implementação e otimizações

- **Nó sentinela embutido no objeto** (não alocado no heap): elimina todos os casos especiais (`if (head == nullptr)`), e a lista vazia não aloca nada. O sentinela é um `NoBase` sem campo `valor`, então `T` não precisa ser construtível por padrão.
- **RAII**: o destrutor libera todos os nós; não há `new`/`delete` expostos ao usuário.
- **Regra dos 5**:
  - construtor de cópia delega ao construtor padrão, de modo que, se a cópia de um elemento lançar exceção no meio, o destrutor ainda roda e nada vaza;
  - atribuição por cópia usa *copia-e-troca* (garantia forte de exceção);
  - construtor e atribuição de movimento são `noexcept` e O(1) (apenas religam o anel), o que permite que `std::vector<ListaDupla<T>>` mova em vez de copiar ao realocar.
- **Iterador único parametrizado por `bool Constante`**: `iterator` e `const_iterator` vêm do mesmo template, com conversão implícita apenas de `iterator` para `const_iterator`. Os *typedefs* de `iterator_traits` (`bidirectional_iterator_tag`) permitem usar `std::find`, `std::reverse`, `std::accumulate`, `std::reverse_iterator`, `std::back_inserter` etc.
- **`emplace`** constrói o valor diretamente no nó (*perfect forwarding*), evitando cópias; funciona com tipos somente-móveis como `std::unique_ptr`.
- **`inverter` em O(n) sem copiar valores**: troca `ant`/`prox` de cada nó (inclusive do sentinela). Iteradores existentes continuam válidos.
- **Inserção com garantia forte**: o nó é alocado antes de qualquer ponteiro ser alterado; se `new` ou o construtor de `T` lançar, a lista fica intacta.
- `noexcept` e `const` em tudo que não pode falhar ou não modifica a lista; `[[nodiscard]]` em consultas.

## Estrutura de pastas

```
lista-duplamente-encadeada-cpp/
├── include/
│   └── lista_dupla.hpp        # implementação (header-only)
├── src/
│   └── main.cpp               # demonstração
├── tests/
│   └── test_lista_dupla.cpp   # testes (assert + std::list como oráculo)
├── .github/workflows/ci.yml   # CI: testes com sanitizers + demo
├── Makefile
├── LICENSE
└── README.md
```

## Como compilar, executar e testar

### Linux / macOS (make)

```bash
make            # compila demo e testes em build/
make run        # executa a demonstração
make test       # executa os testes
make test SANITIZE=1   # testes com AddressSanitizer + UndefinedBehaviorSanitizer
make clean
```

### Windows (g++ do MinGW-w64 / WinLibs)

```bat
mkdir build
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude src/main.cpp -o build/demo.exe
g++ -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude tests/test_lista_dupla.cpp -o build/testes.exe
build\demo.exe
build\testes.exe
```

### Uso

```cpp
#include "lista_dupla.hpp"

ListaDupla<int> l{3, 1, 4};
l.inserir_tras(5);
l.inserir(l.buscar(4), 42);          // O(1) dado o iterador
for (int x : l) std::cout << x << ' ';
std::reverse(l.begin(), l.end());    // funciona com <algorithm>
```

## Exemplo de saída

`make run`:

```
inicial:           [3, 1, 4, 1, 5] (tamanho 5)
frente 9, tras 2:  [9, 3, 1, 4, 1, 5, 2] (tamanho 7)
42 antes do 4:     [9, 3, 1, 42, 4, 1, 5, 2] (tamanho 8)
remove o 4:        [9, 3, 1, 42, 1, 5, 2] (tamanho 7)
remove todos os 1: 2 removidos
resultado:         [9, 3, 42, 5, 2] (tamanho 5)
invertida:         [2, 5, 42, 3, 9] (tamanho 5)
soma (accumulate): 61
maximo:            42
std::reverse:      [9, 3, 42, 5, 2] (tamanho 5)
de tras pra frente: 2 5 42 3 9
original:          [ana, bruno, carla] (tamanho 3)
movida:            [ana, bruno, carla, diego] (tamanho 4)
copia apos move vazia? true
frente/tras:       bruno / carla
```

`make test`:

```
[ok] lista vazia
[ok] um elemento
[ok] push/pop nas pontas
[ok] inserir/remover por iterador
[ok] duplicados
[ok] inverter
[ok] copia e movimento
[ok] tipo somente movel
[ok] algoritmos da STL
[ok] aleatorio vs std::list
[ok] grande volume
11 testes passaram
```

Os testes cobrem lista vazia, um elemento, duplicados, tipos somente-móveis, cópia/movimento/auto-atribuição, compatibilidade com a STL, 200 mil elementos e 100 mil operações aleatórias comparadas com `std::list` como oráculo.

## Referências

- CORMEN, T. H. et al. *Algoritmos: Teoria e Prática*. 3. ed. Elsevier, 2012. Cap. 10.2 (listas ligadas com sentinela).
- SZWARCFITER, J. L.; MARKENZON, L. *Estruturas de Dados e Seus Algoritmos*. 3. ed. LTC, 2010.
- STROUSTRUP, B. *A Linguagem de Programação C++*. 4. ed. Bookman, 2013.
- cppreference: [std::list](https://en.cppreference.com/w/cpp/container/list), [requisitos de LegacyBidirectionalIterator](https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator), [regra de três/cinco/zero](https://en.cppreference.com/w/cpp/language/rule_of_three).

## Licença

Distribuído sob a licença MIT. Veja [LICENSE](LICENSE).
