#pragma once

#include "logger.h"

#include <cmath>
#include <deque>
#include <optional>
#include <stdexcept>
#include <QRect>
#include <QPoint>
#include <QVector3D>

namespace util
{
    // basic type aliases and conversion functions
    namespace types
    {
        using uchar = unsigned char;
        using byte  = unsigned char;
        using uint  = unsigned int;

        template<typename T> static inline auto toChar(T x)   { return static_cast<char>(x);}
        template<typename T> static inline auto toUChar(T x)  { return static_cast<uchar>(x); }
        template<typename T> static inline auto toInt(T x)    { return static_cast<int>(x); }
        template<typename T> static inline auto toUInt(T x)   { return static_cast<unsigned int>(x); }
        template<typename T> static inline auto toFloat(T x)  { return static_cast<float>(x); }
        template<typename T> static inline auto toDouble(T x) { return static_cast<double>(x); }
    }

    
    // owner_ptr: Wraps a raw owning pointer that logs an error if you forget
    //            to delete it, and throws an std::runtime_error. If you define the
    //            NDEBUG preprocessor symbol, then no checks, logs or throws will be made.
    //            The class can only be made by the make_owner factory function, which can
    //            take a variadic number of template arguments like std::make_unique does,
    //            or by constructing it with a nullptr, meaning that it does not manage an object.
    template <typename T>
    class owner_ptr
    {
    private:

#ifndef NDEBUG

        void check(const char* msg)
        {
            if (p)
            {
                Logger::error(msg);
                throw std::runtime_error(msg);
            }
        }

#define OWNER_PTR_CHECK(msg) check(msg);

#else

#define OWNER_PTR_CHECK(msg)

#endif

        template <typename S, typename... Args>
        friend auto make_owner(Args&&... args) -> owner_ptr<S>;

        T* p;

        owner_ptr(T* p) : p{ p } { }

    public:

        owner_ptr(std::nullptr_t p) : p{ p } { }

        ~owner_ptr()
        {
            OWNER_PTR_CHECK("owner_ptr destroyed without reset!")
        }

        owner_ptr(owner_ptr&) = delete;
        owner_ptr(const owner_ptr&) = delete;
        owner_ptr& operator=(owner_ptr&) = delete;
        owner_ptr& operator=(const owner_ptr&) = delete;

        owner_ptr(owner_ptr&& other)
        {
            p = other.p;
            other.p = nullptr;
        }

        owner_ptr& operator=(owner_ptr&& other)
        {
            if (this != &other)
            {
                OWNER_PTR_CHECK("owner_ptr moved to without reset!")
                p = other.p;
                other.p = nullptr;
            }
            return *this;
        }

        void reset(T* newp = nullptr)
        {
            if (p)
                delete p;

            p = newp;
        }

        auto operator*()        -> T&       { return *p; }
        auto operator*() const  -> const T& { return *p; }
        auto operator->() const -> T*       { return p; }
        explicit operator bool() const      { return !!p; }
        auto get() const        -> T*       { return p; }
#undef OWNER_PTR_CHECK
    };

    template <typename T, typename... Args>
    auto make_owner(Args&&... args) -> owner_ptr<T>
    {
        return owner_ptr<T>(new T{ std::forward<Args>(args)... });
    }

    
    // history: Represents a collection of undoable and redoable actions.
    //          Must be instantiated with at least one element, and as such, can never be empty.
    //          Undo returns the previous action, or the current one if the size of the collection is 1.
    //          Redo returns the next action, or the current one, if the size of the collection is MaxSize - 1.
    //          Current returns the current action, identified by the current value of index.
    //          Size returns the current number of held actions.
    template <typename T, uint MaxSize>
    class history
    {
        private:
            std::deque<T> data{};
            unsigned int  index{};
            
        public:
            explicit history(const T& item)
            {
                data.push_back(item);
                index = 0u;
            }
            
            using iterator        = typename std::deque<T>::iterator;
            using const_iterator  = typename std::deque<T>::const_iterator;

            auto back() const    -> const T&       { return data.back(); }
            auto back()          -> T&             { return data.back(); }
            auto current() const -> T              { return data[index]; }
            auto size() const    -> unsigned int   { return data.size(); }

            auto begin()         -> iterator       { return data.begin(); }
            auto begin() const   -> const_iterator { return data.begin(); }
            auto cbegin() const  -> const_iterator { return data.begin(); }
            
            auto end()           -> iterator       { return data.end(); }
            auto end() const     -> const_iterator { return data.end(); }
            auto cend() const    -> const_iterator { return data.end(); }

            auto undo() -> T
            {
                if (index > 0)
                    --index;

                return data[index];
            }

            auto redo() -> T
            {
                if (index < data.size() - 1)
                    ++index;

                return data[index];
            }

            void append(const T& value)
            {
                if (index == MaxSize - 1)
                    data.pop_front();
                else if (data.size() > 0 && index < data.size() - 1)
                    data.resize(++index);
                else
                    ++index;

                data.push_back(value);
            }
    };

    // wrapper functions for the rounding operations, returning ints instead of longs
    template<typename T> static inline auto floor(T k) { return static_cast<int>(std::floor(k)); }
    template<typename T> static inline auto round(T k) { return static_cast<int>(std::round(k)); }
    template<typename T> static inline auto ceil(T k)  { return static_cast<int>(std::ceil(k)); }

    static inline auto roundPoint(QPointF p) -> QPoint { return { util::round(p.x()), util::round(p.y()) }; }

    // conversion functions for QPoint and QVector3D
    static inline auto toQPoint(const QVector3D& v) -> QPoint { return { types::toInt(v.x()), types::toInt(v.y()) }; }
    static inline auto toQVector3D(const QPoint& p) -> QVector3D { return { types::toFloat(p.x()), types::toFloat(p.y()), 0.0f }; }
    static inline auto toQVector3D(const QPointF& p) -> QVector3D { return { types::toFloat(p.x()), types::toFloat(p.y()), 0.0f }; }

    // stringifying functions for various Qt types
    static inline auto toQString(const QPoint& p) -> QString
    {
        return QString{ "QPoint( " } + QString::number(p.x()) + ", " + QString::number(p.y()) + ")";
    }

    static inline auto toQString(const QPointF& p) -> QString
    {
        return QString{ "QPoint( " } + QString::number(p.x()) + ", " + QString::number(p.y()) + ")";
    }

    static inline auto toQString(const QVector3D& v) -> QString
    {
        return QString{ "QVector3D( " } + QString::number(v.x()) + ", " + QString::number(v.y()) + ", " + QString::number(v.z())+ ")";
    }

    static inline auto toQString(const QSize& s) -> QString
    {
        return QString{ "QSize(" } + QString::number(s.width()) + "x" + QString::number(s.height()) + ")";
    }

    static inline auto toQString(const QRect& r) -> QString
    {
        return QString{ "QRect( topLeft = " } + toQString(r.topLeft()) + ", size = " + toQString(r.size()) + " )";
    }

    static inline auto clamp(int x, int low = 0, int high = 255) -> int
    {
        if (x < low)
            return low;
        else if (x > high)
            return high;
        else
            return x;
    }

    static inline auto clamp(const QPoint& p, const QRect& r) -> QPoint
    {
        const auto x = clamp(p.x(), 0, r.width()  - 1);
        const auto y = clamp(p.y(), 0, r.height() - 1);
    
        return QPoint{ x, y };
    }
}
