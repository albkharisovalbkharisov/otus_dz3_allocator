#include <iostream>
#include <vector>
#include <map>
#include <bitset>


template <typename T, typename Alloc = std::allocator<T>>
class containerV
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
            std::cout << "delete node: " << o << std::endl;
            next = nullptr;
        }
        bool operator==(const node & rhs) const
        {
            std::cout << "operator==(node)" << std::endl;
            return this->next == rhs.next;
        }
    };
    node* head;
    std::size_t size_;
    using AllocOtherType = typename Alloc::template rebind<node>::other;
    AllocOtherType *b;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;

    containerV() : head(nullptr), size_(0), b(new AllocOtherType()){}
    ~containerV()
    {
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
        *p = b->allocate(1);
        b->construct(*p, std::forward<Args>(args)...);
        ++size_;
    }

    void itemDel(size_t pos, size_t n = 1)
    {
        node** p = &head;
        for ( ; (pos > 0) && (*p != nullptr); --pos, p = &(*p)->next);
        for ( ; (n > 0) && (*p != nullptr); --n)
        {
            std::cout << "del, n=" << n << " : " << reinterpret_cast<size_t>(*p) << std::endl;
            node *tmp = (*p)->next;
            b->destroy(*p);
            b->deallocate(*p, 1);
            *p = tmp;
            --size_;
        }
    }

    T& operator[](size_t pos)
    {
        std::cout << "operator" << std::endl;
        node** p = &head;
        for ( ; (pos > 0) && (*p != nullptr); --pos, p = &(*p)->next);
        if (*p == nullptr)
            throw "something to throw"; // ne umeyu pravil'no kidat', sorry
        return (*p)->o;
    }

    void print(void)
    {
        std::cout << "print" << std::endl;
        for (auto p = head; p != nullptr; p = p->next)
        {
            std::cout << p->o << std::endl;
        }
    }

#if 1
/////////////////////////////////////////////////
    struct MyIt : std::iterator<std::forward_iterator_tag, const T>
    {
//    private:
        node** p;
        MyIt(const MyIt & myit) = default;
        MyIt() = default;
//        MyIt(node** &&ptr) : p(ptr) {};
        MyIt(node** const &ptr) : p(ptr) {};
//        template <typename Z>
//        MyIt(Z && ptr) : p(ptr) {};

//        template <>
//        MyIt(Z && ptr) : p(ptr) {};
//        template <>
//        MyIt(Z && ptr) : p(ptr) <> {};

        MyIt & operator=(MyIt &&)      = default;
        MyIt & operator=(MyIt const &) = default;
//    public:

        bool operator==(const MyIt& rhs) const
        {
            std::cout << "op==()" << std::endl;
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
        std::cout << "begin()" << std::endl;
        return MyIt(&head);
    }

    MyIt end()
    {
        std::cout << "end()" << std::endl;
        node** p = &head;
        for ( ; *p != nullptr; p = &(*p)->next);
        return MyIt(p);
//        return MyIt(nullptr);
    }

/////////////////////////////////////////////////////
#endif

    using iterator = MyIt;
    using const_iterator = MyIt;
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
//        std::cout << __PRETTY_FUNCTION__ << "[n = " << n << ", N = " << N << "]" << std::endl;
        if (ptr == nullptr)
        {
//            std::cout << "\n\nALLOCATE\n" << std::endl;
            ptr = (T *) std::malloc(N * sizeof(T));
            while(n > 0)
                alloc[--n] = 1;
            return reinterpret_cast<T *>(ptr);
        }

        T *p = nullptr;
        size_t c = 0;   // n counter
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
                if (alloc[c])   //dbg_
                    throw "fignya occured";
                alloc[c] = 1;
            }
        }
        return reinterpret_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) {
        (void) n;   // dbg_
//        std::cout << __PRETTY_FUNCTION__ << "\nleft(" << alloc.count() << ")" << std::endl;
        int i = p - ptr;
//        std::cout << "num = " << i << std::endl;
//        std::cout << "p = " << p << ", ptr = " << ptr  << std::endl;

        for (std::size_t j = i; j < (i + n); ++j)
        {
            if (!alloc[j])
                throw "ERROR_BYAKA";
            alloc[j] = 0;
        }

        if (alloc.none())
        {
//            std::cout << "\n\nDEALLOCATE\n" << std::endl;
            std::free(ptr);
            ptr = nullptr;
        }
//        else
//            std::cout << "\n\nCAN'T DEALLOCATE, " << alloc.count() << " left\n" << std::endl;
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {
//        std::cout << __PRETTY_FUNCTION__ << std::endl;
//        new(p) T(std::forward<Args>(args)...);
        new(p) U(std::forward<Args>(args)...);
    }

    template<typename U>
    void destroy(U *p) {
//        std::cout << __PRETTY_FUNCTION__ << std::endl;
        p->~U();
    }
};


class test
{
public:
    long a;
    test(long a) : a(a)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    ~test()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

int main(int, char *[]) {
#if 0
    auto v = std::vector<int, logging_allocator<int, 5>>{};
    v.reserve(5);
    for (size_t i = 0; i < 5; ++i) {
        v.emplace_back(i);
        std::cout << std::endl;
    }

    for (auto i: v) {
        std::cout << i << std::endl;
    }


    auto m = std::map<int, int, std::less<int>, logging_allocator<std::pair<const int, int>, 10>>{};
    auto fact = [](size_t i) -> size_t
        {
            if (i == 0) return 1;
            while (i > 2) i *= i - 1;
            return i;
        };
    for (size_t i = 0; i < 10; ++i) {
        m[i] = 1;//fact(i);
        std::cout << std::endl;
    }
#endif

#if 0
    containerV<std::string, logging_allocator<std::string, 10>> cv;
//    containerV<std::string> cv;
    std::cout << "size " << cv.size() << std::endl;
    cv.itemAdd("s0");
    cv.itemAdd("s1");
    cv.itemAdd("s2");
    cv.itemAdd("s3");
    std::cout << "size " << cv.size() << std::endl;
//    cv.itemDel(2);
    std::cout << "size " << cv.size() << std::endl;
//    containerV<int, logging_allocator<int, 5>> cv;
//    cv.itemAdd(5);
    cv.print();
    std::cout << cv[0] << std::endl;
    std::cout << cv[1] << std::endl;
    std::cout << cv[2] << std::endl;
    std::cout << cv[3] << std::endl;

    std::cout << "=====================" << std::endl;

//    auto a = std::string("pesik");
//    cv[3] = a;
    cv[3] = "pes barbos";
    std::cout << cv[3] << std::endl;
#endif

    containerV<std::string> cv;
    cv.itemAdd("s0");
    cv.itemAdd("s1");
    cv.itemAdd("s2");

    for (auto a = cv.begin(); a != cv.end(); ++a)
    {
        std::cout << *a << std::endl;
    }
    for (auto a : cv)
        std::cout << a << std::endl;
    return 0;
}

