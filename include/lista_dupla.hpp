// lista_dupla.hpp - lista duplamente encadeada generica com no sentinela.
//
// O sentinela fecha a lista em um anel: sentinela.prox e o primeiro elemento,
// sentinela.ant e o ultimo e end() aponta para o proprio sentinela. Assim
// insercao e remocao nunca precisam tratar casos especiais de ponta.
#ifndef LISTA_DUPLA_HPP
#define LISTA_DUPLA_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

template <class T>
class ListaDupla {
    // Parte de encadeamento, sem valor: usada pelo sentinela.
    struct NoBase {
        NoBase* ant;
        NoBase* prox;
    };

    // No real: encadeamento + valor.
    struct No : NoBase {
        T valor;

        template <class... Args>
        explicit No(Args&&... args)
            : NoBase{nullptr, nullptr}, valor(std::forward<Args>(args)...) {}
    };

    template <bool Constante>
    class IteradorBase {
        friend class ListaDupla;
        friend class IteradorBase<!Constante>;

        NoBase* no_ = nullptr;

        explicit IteradorBase(NoBase* no) noexcept : no_(no) {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t<Constante, const T*, T*>;
        using reference = std::conditional_t<Constante, const T&, T&>;

        IteradorBase() noexcept = default;

        // Conversao implicita iterator -> const_iterator (nunca o contrario).
        template <bool C = Constante, class = std::enable_if_t<C>>
        IteradorBase(const IteradorBase<false>& outro) noexcept : no_(outro.no_) {}

        reference operator*() const noexcept { return static_cast<No*>(no_)->valor; }
        pointer operator->() const noexcept { return &static_cast<No*>(no_)->valor; }

        IteradorBase& operator++() noexcept {
            no_ = no_->prox;
            return *this;
        }
        IteradorBase operator++(int) noexcept {
            IteradorBase copia = *this;
            no_ = no_->prox;
            return copia;
        }
        IteradorBase& operator--() noexcept {
            no_ = no_->ant;
            return *this;
        }
        IteradorBase operator--(int) noexcept {
            IteradorBase copia = *this;
            no_ = no_->ant;
            return copia;
        }

        friend bool operator==(const IteradorBase& a, const IteradorBase& b) noexcept {
            return a.no_ == b.no_;
        }
        friend bool operator!=(const IteradorBase& a, const IteradorBase& b) noexcept {
            return a.no_ != b.no_;
        }
    };

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using iterator = IteradorBase<false>;
    using const_iterator = IteradorBase<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // ---------- construcao, copia, movimento, destruicao (regra dos 5) ----------

    ListaDupla() noexcept { reiniciar(); }

    ListaDupla(std::initializer_list<T> valores) : ListaDupla() {
        for (const T& v : valores) inserir_tras(v);
    }

    // Delegar ao construtor padrao garante que o destrutor rode se uma copia lancar.
    ListaDupla(const ListaDupla& outra) : ListaDupla() {
        for (const T& v : outra) inserir_tras(v);
    }

    ListaDupla(ListaDupla&& outra) noexcept : ListaDupla() { roubar(outra); }

    // Copia-e-troca: garantia forte de excecao.
    ListaDupla& operator=(const ListaDupla& outra) {
        if (this != &outra) {
            ListaDupla temp(outra);
            trocar(temp);
        }
        return *this;
    }

    ListaDupla& operator=(ListaDupla&& outra) noexcept {
        if (this != &outra) {
            limpar();
            roubar(outra);
        }
        return *this;
    }

    ListaDupla& operator=(std::initializer_list<T> valores) {
        ListaDupla temp(valores);
        trocar(temp);
        return *this;
    }

    ~ListaDupla() { limpar(); }

    // ---------- capacidade ----------

    [[nodiscard]] bool vazia() const noexcept { return tamanho_ == 0; }
    [[nodiscard]] size_type tamanho() const noexcept { return tamanho_; }

    // ---------- acesso ----------

    T& frente() {
        exigir_nao_vazia();
        return valor_de(sentinela_.prox);
    }
    const T& frente() const {
        exigir_nao_vazia();
        return valor_de(sentinela_.prox);
    }
    T& tras() {
        exigir_nao_vazia();
        return valor_de(sentinela_.ant);
    }
    const T& tras() const {
        exigir_nao_vazia();
        return valor_de(sentinela_.ant);
    }

    // ---------- iteradores ----------

    iterator begin() noexcept { return iterator(sentinela_.prox); }
    iterator end() noexcept { return iterator(&sentinela_); }
    const_iterator begin() const noexcept { return const_iterator(sentinela_.prox); }
    const_iterator end() const noexcept { return const_iterator(sentinela_ptr()); }
    const_iterator cbegin() const noexcept { return begin(); }
    const_iterator cend() const noexcept { return end(); }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

    // ---------- insercao (todas O(1)) ----------

    // Constroi o elemento no lugar, antes de `pos`. Retorna iterador para ele.
    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        No* novo = new No(std::forward<Args>(args)...);  // se lancar, nada mudou
        NoBase* direita = pos.no_;
        NoBase* esquerda = direita->ant;
        novo->ant = esquerda;
        novo->prox = direita;
        esquerda->prox = novo;
        direita->ant = novo;
        ++tamanho_;
        return iterator(novo);
    }

    iterator inserir(const_iterator pos, const T& valor) { return emplace(pos, valor); }
    iterator inserir(const_iterator pos, T&& valor) { return emplace(pos, std::move(valor)); }

    void inserir_frente(const T& valor) { emplace(begin(), valor); }
    void inserir_frente(T&& valor) { emplace(begin(), std::move(valor)); }
    void inserir_tras(const T& valor) { emplace(end(), valor); }
    void inserir_tras(T&& valor) { emplace(end(), std::move(valor)); }

    template <class... Args>
    T& emplace_frente(Args&&... args) {
        return *emplace(begin(), std::forward<Args>(args)...);
    }
    template <class... Args>
    T& emplace_tras(Args&&... args) {
        return *emplace(end(), std::forward<Args>(args)...);
    }

    // Nomes da STL, para compatibilidade com std::back_inserter/front_inserter.
    void push_back(const T& valor) { inserir_tras(valor); }
    void push_back(T&& valor) { inserir_tras(std::move(valor)); }
    void push_front(const T& valor) { inserir_frente(valor); }
    void push_front(T&& valor) { inserir_frente(std::move(valor)); }

    // ---------- remocao ----------

    // Remove o elemento em `pos` (que nao pode ser end()). Retorna o seguinte. O(1).
    iterator remover(const_iterator pos) noexcept {
        NoBase* alvo = pos.no_;
        NoBase* seguinte = alvo->prox;
        alvo->ant->prox = seguinte;
        seguinte->ant = alvo->ant;
        delete static_cast<No*>(alvo);
        --tamanho_;
        return iterator(seguinte);
    }

    // Remove o intervalo [primeiro, ultimo). Retorna `ultimo`.
    iterator remover(const_iterator primeiro, const_iterator ultimo) noexcept {
        while (primeiro != ultimo) primeiro = remover(primeiro);
        return iterator(ultimo.no_);
    }

    void remover_frente() {
        exigir_nao_vazia();
        remover(begin());
    }
    void remover_tras() {
        exigir_nao_vazia();
        remover(const_iterator(sentinela_.ant));
    }

    // Remove todas as ocorrencias de `valor`. Retorna quantas foram removidas.
    size_type remover_valor(const T& valor) {
        size_type removidos = 0;
        for (auto it = cbegin(); it != cend();) {
            if (*it == valor) {
                it = remover(it);
                ++removidos;
            } else {
                ++it;
            }
        }
        return removidos;
    }

    void limpar() noexcept {
        NoBase* atual = sentinela_.prox;
        while (atual != &sentinela_) {
            NoBase* seguinte = atual->prox;
            delete static_cast<No*>(atual);
            atual = seguinte;
        }
        reiniciar();
    }

    // ---------- busca ----------

    iterator buscar(const T& valor) { return std::find(begin(), end(), valor); }
    const_iterator buscar(const T& valor) const { return std::find(begin(), end(), valor); }
    [[nodiscard]] bool contem(const T& valor) const { return buscar(valor) != end(); }

    // ---------- operacoes ----------

    // Inverte a ordem em O(n) trocando os ponteiros de cada no (inclusive o
    // sentinela). Nenhum valor e copiado e os iteradores continuam validos.
    void inverter() noexcept {
        NoBase* atual = &sentinela_;
        do {
            std::swap(atual->ant, atual->prox);
            atual = atual->ant;  // era o prox antes da troca
        } while (atual != &sentinela_);
    }

    void trocar(ListaDupla& outra) noexcept {
        if (this == &outra) return;
        ListaDupla temp;
        temp.roubar(outra);
        outra.roubar(*this);
        roubar(temp);
    }

    friend void swap(ListaDupla& a, ListaDupla& b) noexcept { a.trocar(b); }

    friend bool operator==(const ListaDupla& a, const ListaDupla& b) {
        return a.tamanho_ == b.tamanho_ && std::equal(a.begin(), a.end(), b.begin());
    }
    friend bool operator!=(const ListaDupla& a, const ListaDupla& b) { return !(a == b); }

private:
    NoBase sentinela_;
    size_type tamanho_ = 0;

    void reiniciar() noexcept {
        sentinela_.ant = &sentinela_;
        sentinela_.prox = &sentinela_;
        tamanho_ = 0;
    }

    // Pre-condicao: *this vazia. Toma os nos de `outra` e a deixa vazia.
    void roubar(ListaDupla& outra) noexcept {
        if (outra.vazia()) return;
        sentinela_.prox = outra.sentinela_.prox;
        sentinela_.ant = outra.sentinela_.ant;
        sentinela_.prox->ant = &sentinela_;
        sentinela_.ant->prox = &sentinela_;
        tamanho_ = outra.tamanho_;
        outra.reiniciar();
    }

    NoBase* sentinela_ptr() const noexcept { return const_cast<NoBase*>(&sentinela_); }

    static T& valor_de(NoBase* no) noexcept { return static_cast<No*>(no)->valor; }

    void exigir_nao_vazia() const {
        if (tamanho_ == 0) throw std::out_of_range("ListaDupla: lista vazia");
    }
};

#endif  // LISTA_DUPLA_HPP
