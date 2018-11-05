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
    };
    node* head;
    std::size_t size_;
    using AllocOtherType = typename Alloc::template rebind<node>::other;
    AllocOtherType *b;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
//    using iterator = MyIt;
//    using const_iterator = MyIt;

    containerV() : head(nullptr), size_(0), b(new AllocOtherType()){}
    ~containerV()
    {
        node* &p = head;
        while( p != nullptr )
        {
            std::cout << "node to delete: " << reinterpret_cast<unsigned long>(p) << std::endl;
            node *tmp = p->next;
            b->destroy(p);
            b->deallocate(p, 1);
            p = tmp;
        }
    }

    size_t size(void) const
    {
        return size_;
    }

    template <typename ...Args>
    void itemAdd(Args &&...args)
    {
        if (head == nullptr)
        {
            head = b->allocate(1);
            b->construct(head, std::forward<Args>(args)...);
        }
        else
        {
            node *p = head;
            for ( ; p->next != nullptr; p = p->next);
            p->next = b->allocate(1);
            b->construct(p->next, std::forward<Args>(args)...); // don't know how to get rid of code repetition
        }
        ++size_;
    }

    void itemDel(size_t)
    {

    }

    void print(void)
    {
        std::cout << "print" << std::endl;
        for (auto p = head; p != nullptr; p = p->next)
        {
//            std::cout << reinterpret_cast<unsigned long long>(p) << " : ";
            std::cout << p->o << std::endl;
        }
    }

#if 0
/////////////////////////////////////////////////
    struct MyIt : std::iterator<std::forward_iterator_tag, const T>
    {
        MyIt(const MyIt & myit) = default;
        MyIt(const MyItR & myitr) : itL(myitr.itL), itV(myitr.itV) { }
        MyIt() = default;

        MyIt & operator=(MyIt &&)      = default;
        MyIt & operator=(MyIt const &) = default;

        bool operator==(const MyIt& rhs) const
        {
            return (this->itL == rhs.itL) && (this->itV == rhs.itV);
        }

        bool operator!=(const MyIt& rhs) const
        {
            return !(*this == rhs);
        }

        MyIt& operator++ ()
        {
            ++itV;
            if (itV == itL->end())
            {
                ++itL;
                itV = itL->begin();
            }
            return *this;
        }
        MyIt operator++(int) { MyIt temp = *this; ++*this; return temp; }

        const T& operator* () const
        {
            return *itV;
        }
        const T * operator->() const
        {
            return &(*itV);
        }

        typename VectT::const_iterator itV;
        typename ListT::const_iterator itL;
    };

    const MyIt begin() const
    {
        MyIt myit;
        myit.itL = data_.begin();
        myit.itV = myit.itL->begin();
        return myit;
    }

    const MyIt end() const
    {
        MyIt myit;
        myit.itL = data_.end();
        myit.itV = myit.itL->begin();
        return myit;
    }

/////////////////////////////////////////////////////
#endif

};


template<typename T, int N>
struct logging_allocator {
    using value_type = T;
//
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template<typename U>
    struct rebind {
        using other = logging_allocator<U, N>;
    };
//

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
        std::cout << __PRETTY_FUNCTION__ << std::endl;
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
#endif

#if 0
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

#if 1
//    containerV<std::string, logging_allocator<std::string, 10>> cv;
    containerV<std::string> cv;
    cv.itemAdd("banka sopel'");
    cv.itemAdd(" another banka of sopel'");
    cv.itemAdd(" another banka of sopel' 2");
    cv.itemAdd("banka sopel' 2");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
    cv.itemAdd("banka sopel' 3");
//    containerV<int, logging_allocator<int, 5>> cv;
//    cv.itemAdd(5);
    cv.print();
#else
    {
        containerV<test> ct;
        ct.itemAdd(1);
        ct.itemAdd(2);
        ct.itemAdd(3);
        ct.itemAdd(4);
    }
    std::cout << "===================" << std::endl;
//    cv.print();

#endif
    return 0;
}

