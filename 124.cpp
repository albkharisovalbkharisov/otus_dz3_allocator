
#include <vector>
#include <list>
#include <iterator>

template<class T>
class VectorList
{
private:
    using VectT = std::vector<T>;
    using ListT = std::list<VectT>;

public:
    struct MyIt;
    struct MyItR;
    typedef MyIt iterator;
    typedef MyIt const_iterator;
    typedef MyItR reverse_iterator;
    typedef MyItR const_reverse_iterator;
//    typedef std::const_reverse_iterator<VectorList::const_iterator> MyItR;
//    typedef MyItR const_reverse_iterator;

    using value_type = T;

    VectorList() = default;
    VectorList(VectorList const &) = default;
    VectorList(VectorList &&) = default;
    VectorList & operator=(VectorList &&)     = default;
    VectorList & operator=(VectorList const &) = default;

    template<class It>
    void append(It p, It q);

    bool empty() const { return size() == 0; }

    size_t size() const
    {
        int i = 0;
        for (auto It = data_.begin(); It != data_.end() ; ++It)
            i += It->size();
        return i;
    }

    struct MyIt : std::iterator<std::bidirectional_iterator_tag, const T>
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

        MyIt& operator-- ()
        {
            if (itV == itL->begin())
            {
                --itL;
                itV = itL->end();
            }
            --itV;

            return *this;
        }
        MyIt operator--(int) { MyIt temp = *this; --*this; return temp; }

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

////////////////////////////////////////////////////////////////////////////////////////
    struct MyItR : std::iterator<std::bidirectional_iterator_tag, const T>
    {
        MyItR(const MyItR & myitr) = default;
        MyItR(const MyIt & myit) : itL(myit.itL), itV(myit.itV)
        {
        }
        MyIt base() const
        {
            MyIt m;
            m.itL = itL.base();
            m.itV = itV.base();
            return m;
        }
        MyItR() = default;
        MyItR & operator=(MyItR &&)      = default;
        MyItR & operator=(MyItR const &) = default;

        bool operator==(const MyItR& rhs) const
        {
            return (this->itL == rhs.itL) && (this->itV == rhs.itV);
        }

        bool operator!=(const MyItR& rhs) const
        {
            return !(*this == rhs);
        }

        MyItR& operator++ ()        // prefix ++
        {
            ++itV;
            if (itV == itL->rend())
            {
                ++itL;
                itV = itL->rbegin();
            }
            return *this;
        }
        MyItR operator++(int) { MyItR temp = *this; ++*this; return temp; }

        MyItR& operator--()
        {
            if (itV == itL->rbegin())
            {
                --itL;
                itV = itL->rend();
            }
            --itV;

            return *this;
        }
        MyItR operator--(int) { MyItR temp = *this; --*this; return temp; }


        const T& operator* () const
        {
            return *itV;
        }
        const T * operator->() const
        {
            return &(*itV);
        }

        typename VectT::const_reverse_iterator itV;
        typename ListT::const_reverse_iterator itL;
    };

    const MyItR rbegin() const
    {
        MyItR myitr;
        myitr.itL = data_.rbegin();
        myitr.itV = myitr.itL->rbegin();
        return myitr;
    }

    const MyItR rend() const
    {
        MyItR myitr;
        myitr.itL = data_.rend();
        myitr.itV = myitr.itL->rbegin();
        return myitr;
    }

private:
    ListT data_;
};
