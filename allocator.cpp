#include <iostream>
#include <vector>
#include <map>
#include <bitset>
#include <typeinfo>


template <typename T, typename Alloc = std::allocator<T>>
class containerL
{
private:
    class node
    {
    public:
        node* next;
        T o;
        template<typename ...Args>
        node(Args &&...args) : next(nullptr), o(std::forward<Args>(args)...){}
        ~node()
        {
            next = nullptr;
        }
        bool operator==(const node & rhs) const
        {
            return this->next == rhs.next;
        }
    };
    using AllocOtherType = typename Alloc::template rebind<node>::other;
public:
    node* head;
    std::size_t size_;
    AllocOtherType a;
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    // move constructor
#if 0
//    template<typename Y>
//    containerL(containerL<T, Y>&& c) : a()
    containerL(containerL&& c) : a()
    {
        std::cout << "MOVE CTOR" << std::endl;
        head = static_cast<node *>(static_cast<void *>(c.head));
        std::cout << "  head=" << reinterpret_cast<void*>(head) << std::endl;
        std::cout << "c.head=" << reinterpret_cast<void*>(c.head) << std::endl;
        size_ = c.size_;
        c.head = nullptr;
        c.size_ = 0;
    }
#else
    template<typename Y>
    containerL(containerL<T, Y>&& c) : head(nullptr), size_(0), a()
    {
        std::cout << "MOVE CTOR" << std::endl;
        node** p = &head;
        for (size_t i = 0; i < c.size(); ++i, p = &(*p)->next)
        {
            if (*p != nullptr)
                throw "smth";
            *p = a.allocate(1);
            a.construct(*p, std::move(c[i]));
            ++size_;
        }
    }


#endif // 0

    containerL() : head(nullptr), size_(0), a()
    {
        std::cout << "CTOR" << std::endl;
    }

#if 1
    // not used yet
    // copy constructor as it is
    containerL(const containerL &c) : head(nullptr), size_(0), a()
    {
        std::cout << "COPY CTOR" << std::endl;
        node** p = &head;
        for (size_t i = 0; i < c.size(); ++i, p = &(*p)->next)
        {
            if (*p != nullptr)
                throw "smth";
            *p = a.allocate(1);
            a.construct(*p, c[i]);
            ++size_;
        }
    }
#endif  // 0
    // seems to be a copy constructor, but takes containerL with another allocator type
    template<typename Y>
    containerL(const containerL<T, Y>& c) : head(nullptr), size_(0), a()
    {
        std::cout << "COPY CTOR<>" << std::endl;
        node** p = &head;
        for (size_t i = 0; i < c.size(); ++i, p = &(*p)->next)
        {
            if (*p != nullptr)
                throw "smth";       // dbg_
            *p = a.allocate(1);
            a.construct(*p, c[i]);
            ++size_;
        }
    }
    ~containerL()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "DTOR: " << static_cast<void*>(head) << std::endl;
        itemDel(0, size_);
    }

    size_t size(void) const
    {
        return size_;
    }

    template <typename ...Args>
    void itemAdd(Args &&...args)
    {
        node** p = &head;
        for ( ; *p != nullptr; p = &(*p)->next);
        *p = a.allocate(1);
        a.construct(*p, std::forward<Args>(args)...);
        ++size_;
    }

    void itemDel(size_t pos, size_t n = 1)
    {
        node** p = &head;
        for ( ; (pos > 0) && (*p != nullptr); --pos, p = &(*p)->next);
        for ( ; (n > 0) && (*p != nullptr); --n)
        {
            node *tmp = (*p)->next;
            a.destroy(*p);
            a.deallocate(*p, 1);
            *p = tmp;
            --size_;
        }
    }

    const T& operator[](size_t pos) const
    {
        std::cout << "const op []" << std::endl;
        node * const * p = &head;
        for ( ; (pos > 0) && (*p != nullptr); --pos, p = &(*p)->next);
        if (*p == nullptr)
            throw "something to throw"; // ne umeyu pravil'no kidat', sorry
        return (*p)->o;
    }

    T& operator[](size_t pos)
    {
        std::cout << "op []" << std::endl;
        node** p = &head;
        for ( ; (pos > 0) && (*p != nullptr); --pos, p = &(*p)->next);
        if (*p == nullptr)
            throw "something to throw"; // ne umeyu pravil'no kidat', sorry
        return (*p)->o;
    }

    void print(void)
    {
        for (auto p = head; p != nullptr; p = p->next)
        {
            std::cout << p->o << std::endl;
        }
    }

    struct MyIt : std::iterator<std::forward_iterator_tag, const T>
    {
        node** p;
        MyIt(const MyIt & myit) = default;
        MyIt() = default;
        MyIt(node** const &ptr) : p(ptr) {};

        MyIt & operator=(MyIt &&)      = default;
        MyIt & operator=(MyIt const &) = default;

        bool operator==(const MyIt& rhs) const
        {
            return this->p == rhs.p;
        }

        bool operator!=(const MyIt& rhs) const
        {
            return !(*this == rhs);
        }

        MyIt& operator++ ()
        {
            if (*p != nullptr)
                p = &(*p)->next;
            return *this;
        }
        MyIt operator++(int) { MyIt temp = *this; ++*this; return temp; }

        const T& operator* () const
        {
            return (*p)->o;
        }
        const T * operator->() const
        {
            return &(*p)->o;
        }
    };

    MyIt begin()
    {
        return MyIt(&head);
    }

    MyIt end()
    {
        node** p = &head;
        for ( ; *p != nullptr; p = &(*p)->next);
        return MyIt(p);
    }

    using iterator = MyIt;
};


template<typename T, int N>
struct logging_allocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind {
        using other = logging_allocator<U, N>;
    };

    T *ptr;
    std::bitset<N> alloc;     // 1 - allocated, 0 - free
    T *allocate(std::size_t n) {
        if (ptr == nullptr)
        {
            ptr = (T *) std::malloc(N * sizeof(T));
            while(n > 0)
                alloc[--n] = 1;
            return reinterpret_cast<T *>(ptr);
        }

        T *p = nullptr;
        size_t c = 0;   // counter
        size_t i = 0;
        for ( ; i < N; ++i)
        {
            if (!alloc[i])
            {
                if (p == nullptr)
                    p = &ptr[i];
                if (++c == n)
                    break;
            }
            else
            {
                p = nullptr;
                c = 0;
            }
        }
        if ((p == nullptr) || (c != n))
            throw std::bad_alloc();
        else
        {
            for (c = i + 1 - n; c < (i + 1); ++c)
            {
                if (alloc[c])
                    throw "fignya occured";
                alloc[c] = 1;
            }
        }
        return reinterpret_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) {
        int i = p - ptr;

        for (std::size_t j = i; j < (i + n); ++j)
        {
            if (!alloc[j])
                throw "ERROR";
            alloc[j] = 0;
        }

        if (alloc.none())
        {
            std::free(ptr);
            ptr = nullptr;
        }
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {
        new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U *p) {
        p->~U();
    }
};

int fact(int i)
{
    return i > 1 ? i*fact(i-1) : 1;
}

int main(int, char *[]) {
////    auto cc = containerL<int>();
//    containerL<int> cc{};
//    std::cout << typeid(cc).name() << std::endl;
////    int i = 5;
////    cc.itemAdd(i);
////    cc.itemAdd(i);
////    cc.itemAdd(1);
////    cc.itemAdd(1);
////    cc.itemAdd(1);
//    std::cout << cc[0] << std::endl;
////    cc.itemDel(0);
////    containerL<int, logging_allocator<int, 10>> c1(std::move(cc));

#if 1
//    // 1
//    auto m = std::map<int, int>();
//    for (int i = 0; i < 10; ++i){
//        m[i] = fact(i);
//    }
//
//    // 2
//    auto ma = std::map<int, int, std::less<int>, logging_allocator<std::pair<int, int>, 10>>();
//    for (int i = 0; i < 10; ++i){
//        ma.insert(std::pair<int, int>(i, fact(i)));
//    }
//    for (auto it = ma.begin(); it != ma.end(); ++it){
//        std::cout << it->first << " " << it->second << std::endl;
//    }

    // 3
    containerL<int> c{};
    for (int i = 0; i < 10; ++i){
        c.itemAdd(i);
    }

    // 4
    std::cout << "========================" << std::endl;
    containerL<int, logging_allocator<int, 10>> ca{c};
    std::cout << "========================" << std::endl;
    for (auto a : ca){
        std::cout << a << std::endl;
    }

    std::cout << "========================" << std::endl;
    // 5
    containerL<int> c1(std::move(ca));
    std::cout << "========================" << std::endl;
#endif
}

