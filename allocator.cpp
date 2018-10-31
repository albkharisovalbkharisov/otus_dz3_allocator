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
    using value_type = T;
    container(){}
    ~container(){}
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

