#include <iostream>
#include <vector>
#include <map>
#include <bitset>


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
        std::cout << __PRETTY_FUNCTION__ << "[n = " << n << ", N = " << N << "]" << std::endl;
        if (ptr == nullptr)
        {
            std::cout << "\n\nALLOCATE\n" << std::endl;
            ptr = (T *) std::malloc(N * sizeof(T));
            while(n > 0)
                alloc[--n] = 1;
            return reinterpret_cast<T *>(ptr);
        }

        T *p = nullptr;
        size_t nn = n;
        for (int i = 0; (i < N); ++i)
        {
            if (!alloc[i])
            {
                if (p == nullptr)
                    p = &ptr[i];
                if (--n <= 0)
                {
                    for (; nn > 0; )
                    {
                        if (alloc[i + nn - 1])   //dbg_
                            throw "fignya occured";
                        alloc[i + --nn] = 1;
                    }
                    break;
                }
            }
            else
            {
                p = nullptr;
                n = nn;
            }
        }
        if (!p || (n != 0))
            throw std::bad_alloc();
        return reinterpret_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) {
        (void) n;   // dbg_
        std::cout << __PRETTY_FUNCTION__ << "\nleft(" << alloc.count() << ")" << std::endl;
        int i = p - ptr;
        std::cout << "num = " << i << std::endl;
        std::cout << "p = " << p << ", ptr = " << ptr  << std::endl;

        for (std::size_t j = i; j < (i + n); ++j)
        {
            if (!alloc[j])
                throw "ERROR_BYAKA";
            alloc[j] = 0;
        }

        if (alloc.none())
        {
            std::cout << "\n\nDEALLOCATE\n" << std::endl;
            std::free(ptr);
            ptr = nullptr;
        }
        else
            std::cout << "\n\nCAN'T DEALLOCATE, " << alloc.count() << " left\n" << std::endl;
    }

    template<typename U, typename ...Args>
    void construct(U *p, Args &&...args) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
//        new(p) T(std::forward<Args>(args)...);
        new(p) U(std::forward<Args>(args)...);
    }

    void destroy(T *p) {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        p->~T();
    }
};

template <typename T, typename Alloc = std::allocator<T>>
class container
{
private:
    T* ptr;
    std::size_t size;
public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = MyIt;
    using const_iterator = MyIt;

    container() : ptr(nullptr), size(0) {}
    ~container(){}
    void addItem(const T& item)
    {
        ptr = Alloc::allocate();
    }

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








};



int main(int, char *[]) {
/*
    auto v = std::vector<int, logging_allocator<int>>{};
    // v.reserve(5);
    for (size_t i = 0; i < 1; ++i) {
        v.emplace_back(i);
        std::cout << std::endl;
    }

    for (auto i: v) {
        std::cout << i << std::endl;
    }
*/

    auto m = std::map<int, int, std::less<int>, logging_allocator<std::pair<const int, int>, 10>>{};
    auto fact = [](size_t i) -> size_t
        {
            if (i == 0) return 1;
            while (i > 2) i *= i - 1;
            return i;
        };
    for (size_t i = 0; i < 10; ++i) {
        m[i] = fact(i);
        std::cout << std::endl;
    }

    return 0;
}

