/**
 * @file   mw/coroutine.hpp
 * @date   09.06.2017
 * @author Klemens D. Morgenstern
 *
 * Published under [Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0.html)
  <pre>
    /  /|  (  )   |  |  /
   /| / |   \/    | /| /
  / |/  |   /\    |/ |/
 /  /   |  (  \   /  |
               )
 </pre>
 */
#ifndef MW_COROUTINE_HPP_
#define MW_COROUTINE_HPP_

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <algorithm>

namespace mw
{



namespace detail
{
namespace coroutine
{

struct impl
{
    std::uint32_t _stack_ptr;

    std::uint32_t _stack_begin;
    std::uint32_t _stack_end;
};

template<typename T>
constexpr std::size_t size_of() {return sizeof(T);}

template<>
constexpr std::size_t size_of<void>() {return 0u;}



template<typename T>
using is_valid_type_t
    = std::integral_constant<bool,
         (size_of<T>() <= sizeof(std::uint32_t)) ||
         ((std::is_integral<T>::value || std::is_enum<T>::value || std::is_pointer<T>::value || std::is_reference<T>::value) && (sizeof(T) <= 8))
         >;

extern "C"
{

std::uint32_t __mw_make_context_0(impl * const, void * target, void * executor);
std::uint32_t __mw_make_context_1(impl * const, void * target, void * executor, std::uint32_t  value);
std::uint32_t __mw_make_context_2(impl * const, void * target, void * executor, std::uint64_t *value);

std::uint32_t __mw_switch_context_0(impl * const);
std::uint32_t __mw_switch_context_1(std::uint32_t, impl * const);
std::uint32_t __mw_switch_context_2(std::uint64_t, impl * const);

}


template<typename Return, typename PushType, bool large = (size_of<PushType>() > 4)>
struct make_context_t
{
    static Return invoke(impl * const ptr, void* target, void* executor, PushType value)
    {
        using func_t = Return(impl * const, void *, void *, PushType);
        auto make_context = reinterpret_cast<func_t*>(&__mw_make_context_1);
        return static_cast<Return>(make_context(ptr, target, executor, static_cast<PushType>(value)));
    }
};

template<typename Return, typename PushType>
struct make_context_t<Return, PushType, true>
{
    static Return invoke(impl * const ptr, void* target, void* executor, PushType value)
    {
        using func_t = Return(impl * const, void*, void*, PushType*);
        auto make_context = reinterpret_cast<func_t*>(&__mw_make_context_2);
        return static_cast<Return>(make_context(ptr, target, executor, &value));
    }
};

template<typename Return>
struct make_context_t<Return, void, false>
{
    static Return invoke(impl * const ptr, void * target, void * executor)
    {
        using func_t = Return(impl * const, void*, void*);
        auto make_context = reinterpret_cast<func_t*>(&__mw_make_context_0);
        return static_cast<Return>(make_context(ptr, target, executor));
    }
};

template<>
struct make_context_t<void, void, false>
{
    static void invoke(impl * const ptr, void * target, void * executor)
    {
        using func_t = void(impl * const, void*, void*);
        auto make_context = reinterpret_cast<func_t*>(&__mw_make_context_0);
        make_context(ptr, target, executor);
    }
};

template<typename Return, typename PushType>
inline Return make_context(impl * const this_, void* target, void * exec, PushType value)
{
    return static_cast<Return>(
        make_context_t<Return, PushType>::invoke(this_, target, exec, static_cast<PushType>(value))
            );
}

template<typename Return>
inline Return make_context(impl * const this_, void* target, void * exec)
{
    return static_cast<Return>(
        make_context_t<Return, void>::invoke(this_, target, exec)
            );
}

template<>
inline void make_context<void>(impl * const this_, void* target, void * exec)
{
    make_context_t<void, void>::invoke(this_, target, exec);
}



template<typename Return, typename PushType, bool large = (size_of<PushType>() > 4)>
struct switch_context_t
{
    static Return invoke(PushType value, impl * const ptr)
    {
        using func_t = Return(PushType, impl * const);
        auto switch_context = reinterpret_cast<func_t*>(&__mw_switch_context_1);
        return static_cast<Return>(switch_context(static_cast<PushType>(value), ptr));
    }
};

template<typename Return, typename PushType>
struct switch_context_t<Return, PushType, true>
{
    static Return invoke(PushType value, impl * const ptr)
    {
        using func_t = Return(PushType, impl * const);
        auto switch_context = reinterpret_cast<func_t*>(&__mw_switch_context_2);
        return static_cast<Return>(switch_context(static_cast<PushType>(value), ptr));
    }
};

template<typename Return>
struct switch_context_t<Return, void, false>
{
    static Return invoke(impl * const ptr)
    {
        using func_t = Return(impl * const);
        auto switch_context = reinterpret_cast<func_t*>(&__mw_switch_context_0);
        return static_cast<Return>(switch_context(ptr));
    }
};

template<typename Return, typename PushType>
inline Return switch_context(PushType value, impl * const this_)
{
    return static_cast<Return>(
        switch_context_t<Return, PushType>::invoke(static_cast<PushType>(value), this_)
            );
}

template<typename Return>
inline Return switch_context(impl * const this_)
{
    return static_cast<Return>(
        switch_context_t<Return, void>::invoke(this_)
            );
}

}
}

template<typename T = void()>
class coroutine;



template<typename T = void()>
class yield_t;

template<typename Return, typename PushType>
class yield_t<Return(PushType)>
{
    coroutine<Return(PushType)> *_cr;
    yield_t(coroutine<Return(PushType)> * const ptr) : _cr(ptr) {}
public:
    yield_t(const yield_t & yt) = delete;
    yield_t operator=(const yield_t & yt) = delete;
    inline PushType operator()(Return rt);


    inline std::uint32_t stack_ptr () const;
    inline std::size_t stack_size() const;
    inline std::size_t stack_used() const;
    inline std::size_t stack_left() const;

    template<typename T>
    friend class coroutine;
};

template<typename Return>
class yield_t<Return()>
{
    coroutine<Return()> *_cr;
    yield_t(coroutine<Return()> * const ptr) : _cr(ptr) {}
public:
    yield_t(const yield_t & yt) = delete;
    yield_t operator=(const yield_t & yt) = delete;
    inline void operator()(Return rt);

    inline std::uint32_t stack_ptr () const;
    inline std::size_t stack_size() const;
    inline std::size_t stack_used() const;
    inline std::size_t stack_left() const;

    template<typename T>
    friend class coroutine;
};

template<typename PushType>
class yield_t<void(PushType)>
{
    coroutine<void(PushType)> *_cr;
    yield_t(coroutine<void(PushType)> * const ptr) : _cr(ptr) {}
public:
    yield_t(const yield_t & yt) = delete;
    yield_t operator=(const yield_t & yt) = delete;
    inline PushType operator()();

    inline std::uint32_t stack_ptr () const;
    inline std::size_t stack_size() const;
    inline std::size_t stack_used() const;
    inline std::size_t stack_left() const;

    template<typename T>
    friend class coroutine;
};

template<>
class yield_t<void()>
{
    coroutine<void()> *_cr;
    yield_t(coroutine<void()> * const ptr) : _cr(ptr) {}
public:
    yield_t(const yield_t & yt) = delete;
    yield_t operator=(const yield_t & yt) = delete;
    inline void operator()();

    inline std::uint32_t stack_ptr () const;
    inline std::size_t stack_size() const;
    inline std::size_t stack_used() const;
    inline std::size_t stack_left() const;

    template<typename T>
    friend class coroutine;
};

template<typename Return, typename PushType>
class coroutine<Return(PushType)> : ::mw::detail::coroutine::impl
{
    static_assert(mw::detail::coroutine::is_valid_type_t<Return>::value,   "The return type must either be a 64-bit integral type or a 32-bit compound type");

    bool _started = false;
    bool _exited  = false;

    template<typename T>
    friend struct yield_t;

public:

    typedef Return return_type;
    typedef PushType push_type;
    typedef yield_t<Return()> yield_type;

    template<typename StackContainer>
    coroutine(StackContainer & sc) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size()) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc.data()),
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size())
            }
            )
    {}

    template<typename T, std::size_t Size>
    coroutine(T(&sc)[Size]) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc + Size) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc),
                reinterpret_cast<std::uintptr_t>(sc + Size)
            }
            )
    {}

    coroutine(const coroutine & cr) = delete;
    coroutine(coroutine && cr) = default;

    coroutine& operator=(const coroutine & cr) = delete;
    coroutine& operator=(coroutine && cr) = default;


    template<typename StackContainer>
    coroutine fork(StackContainer & sc)
    {
        coroutine cr(sc);
        cr._stack_ptr = cr._stack_end - (_stack_end - _stack_ptr);
        std::copy(_stack_ptr, _stack_end, cr._stack_ptr);
        return cr;
    }

    PushType yield_(Return ret)
    {
        return static_cast<PushType>(mw::detail::coroutine::switch_context<PushType, Return>(static_cast<Return>(ret), this));
    }

    Return reenter(PushType pt)
    {
        return mw::detail::coroutine::switch_context<Return, PushType>(static_cast<PushType>(pt), this);
    }

    template<typename Function>
    Return spawn(Function && func)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p)
        {
            this_->_started = true;
            Function func = std::forward<Function>(*func_p);
            Return val = static_cast<Return>(func({this_}));

            this_->_exited = true;
            return mw::detail::coroutine::switch_context<PushType, Return>(static_cast<Return>(val), this_);
        };
        return static_cast<Return>(mw::detail::coroutine::make_context<Return>(this, &func, reinterpret_cast<void*>(executor)));
    }

    template<typename Function>
    Return spawn(Function && func, PushType pt)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p, PushType pt)
        {
            this_->_started = true;
            Function func = std::forward<Function>(*func_p);
            Return val = static_cast<Return>(func({this_}, static_cast<PushType>(pt)));

            this_->_exited = true;
            return mw::detail::coroutine::switch_context(static_cast<Return>(val), this_);
        };
        return static_cast<Return>(mw::detail::coroutine::make_context<Return, PushType>(
                this, &func,
                reinterpret_cast<void*>(executor),
                static_cast<PushType>(pt)));
    }

    Return spawn(return_type(&func)(yield_type)) {return static_cast<Return>(spawn(&func));}
    Return spawn(return_type(*func)(yield_type))
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type))
        {
            this_->_started = true;
            Return val = static_cast<Return>(func(yield_type{this_}));
            this_->_exited = true;

            return mw::detail::coroutine::switch_context(static_cast<Return>(val), this_);
        };
        return mw::detail::coroutine::make_context<Return>(this, func, reinterpret_cast<void*>(executor));
    }

    Return spawn(return_type(&func)(yield_type), Return rt) {return static_cast<Return>(spawn(&func), static_cast<Return>(rt));}
    Return spawn(return_type(*func)(yield_type), Return rt)
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type), Return rt)
        {
            this_->_started = true;
            Return val = static_cast<Return>(func({this_}, static_cast<Return>(rt)));
            this_->_exited = true;

            return mw::detail::coroutine::switch_context(static_cast<Return>(val), this_);
        };
        return mw::detail::coroutine::make_context<Return>(this, func, reinterpret_cast<void*>(executor), static_cast<Return>(rt));
    }

    Return operator()(PushType pt){return reenter(static_cast<PushType>(pt));}

    bool started() const {return _started;}
    bool  exited() const {return _exited;}

    std::uint32_t stack_ptr () const { return _stack_ptr; }
    std::size_t   stack_size() const { return _stack_end - _stack_begin; }
    std::size_t   stack_used() const { return _stack_end - _stack_ptr - sizeof(std::uint32_t); }
    std::size_t   stack_left() const { return _stack_begin >= _stack_ptr ? 0ul : (_stack_ptr - _stack_begin); }
};


template<typename PushType>
class coroutine<void(PushType)> : ::mw::detail::coroutine::impl
{
    bool _started = false;
    bool _exited  = false;

    template<typename T>
    friend struct yield_t;

public:

    typedef void return_type;
    typedef void push_type;
    typedef yield_t<void()> yield_type;

    template<typename StackContainer>
    coroutine(StackContainer & sc) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size()) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc.data()),
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size())
            }
            )
    {}

    template<typename T, std::size_t Size>
    coroutine(T(&sc)[Size]) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc + Size) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc),
                reinterpret_cast<std::uintptr_t>(sc + Size)
            }
            )
    {}

    coroutine(const coroutine & cr) = delete;
    coroutine(coroutine && cr) = default;

    coroutine& operator=(const coroutine & cr) = delete;
    coroutine& operator=(coroutine && cr) = default;


    template<typename StackContainer>
    coroutine fork(StackContainer & sc)
    {
        coroutine cr(sc);
        cr._stack_ptr = cr._stack_end - (_stack_end - _stack_ptr);
        std::copy(_stack_ptr, _stack_end, cr._stack_ptr);
        return cr;
    }

    PushType yield_()
    {
        return static_cast<PushType>(mw::detail::coroutine::switch_context<PushType>(this));
    }

    void reenter(PushType pt)
    {
        mw::detail::coroutine::switch_context<void>( static_cast<PushType>(pt), this);
    }

    template<typename Function>
    void spawn(Function && func)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p)
        {
            this_->_started = true;
            Function func = static_cast<Function>(*func_p);
            func({this_});
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void>(
                this,
                reinterpret_cast<void*>(&func),
                reinterpret_cast<void*>(executor));
    }


    template<typename Function>
    void spawn(Function && func, PushType pt)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p, PushType pt)
        {
            this_->_started = true;
            Function func = static_cast<Function>(*func_p);
            func({this_}, static_cast<PushType>(pt));
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void, PushType>(
                this,
                reinterpret_cast<void*>(&func),
                reinterpret_cast<void*>(executor),
                static_cast<PushType>(pt));
    }

    void spawn(return_type(&func)(yield_type)) {spawn(&func);}
    void spawn(return_type(*func)(yield_type))
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type))
        {
            this_->_started = true;
            func({this_});
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void>(this, reinterpret_cast<void*>(func), reinterpret_cast<void*>(executor));
    }

    void spawn(return_type(&func)(yield_type, PushType), PushType pt) {spawn(&func, static_cast<PushType>(pt));}
    void spawn(return_type(*func)(yield_type, PushType), PushType pt)
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type), PushType pt)
        {
            this_->_started = true;
            func({this_}, static_cast<PushType>(pt));
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void, PushType>(
                this,
                reinterpret_cast<void*>(func),
                reinterpret_cast<void*>(executor),
                static_cast<PushType>(pt));
    }

    void operator()(PushType pt){reenter(static_cast<PushType>(pt));}

    bool started() const {return _started;}
    bool  exited() const {return _exited;}

    std::uint32_t stack_ptr () const { return _stack_ptr; }
    std::size_t   stack_size() const { return _stack_end - _stack_begin; }
    std::size_t   stack_used() const { return _stack_end - _stack_ptr - sizeof(std::uint32_t); }
    std::size_t   stack_left() const { return _stack_begin >= _stack_ptr ? 0ul : (_stack_ptr - _stack_begin); }
};


template<typename Return>
class coroutine<Return()> : ::mw::detail::coroutine::impl
{
    static_assert(mw::detail::coroutine::is_valid_type_t<Return>::value,   "The return type must either be a 64-bit integral type or a 32-bit compound type");

    bool _started = false;
    bool _exited  = false;

    template<typename T>
    friend struct yield_t;

public:

    typedef Return return_type;
    typedef void push_type;
    typedef yield_t<Return()> yield_type;

    template<typename StackContainer>
    coroutine(StackContainer & sc) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size()) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc.data()),
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size())
            }
            )
    {}

    template<typename T, std::size_t Size>
    coroutine(T(&sc)[Size]) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc + Size) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc),
                reinterpret_cast<std::uintptr_t>(sc + Size)
            }
            )
    {}

    coroutine(const coroutine & cr) = delete;
    coroutine(coroutine && cr) = default;

    coroutine& operator=(const coroutine & cr) = delete;
    coroutine& operator=(coroutine && cr) = default;


    template<typename StackContainer>
    coroutine fork(StackContainer & sc)
    {
        coroutine cr(sc);
        cr._stack_ptr = cr._stack_end - (_stack_end - _stack_ptr);
        std::copy(_stack_ptr, _stack_end, cr._stack_ptr);
        return cr;
    }

    void yield_(Return ret)
    {
        mw::detail::coroutine::switch_context<Return>(static_cast<Return>(ret), this);
    }

    Return reenter()
    {
        return mw::detail::coroutine::switch_context<Return>(this);
    }

    template<typename Function>
    Return spawn(Function && func)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p)
        {
            this_->_started = true;
            Function func = std::forward<Function>(*func_p);
            Return val = static_cast<Return>(func({this_}));

            this_->_exited = true;
            return mw::detail::coroutine::switch_context<Return>(static_cast<Return>(val), this_);
        };
        return mw::detail::coroutine::make_context<Return>(this, &func, reinterpret_cast<void*>(executor));
    }

    Return spawn(return_type(&func)(yield_type)) {return spawn(&func);}
    Return spawn(return_type(*func)(yield_type))
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type))
        {
            this_->_started = true;
            Return val = static_cast<Return>(func(yield_type{this_}));
            this_->_exited = true;

            return mw::detail::coroutine::switch_context(static_cast<Return>(val), this_);
        };
        return mw::detail::coroutine::make_context<Return>(this, func, reinterpret_cast<void*>(executor));
    }

    Return operator()(){return reenter();}

    bool started() const {return _started;}
    bool  exited() const {return _exited;}

    std::uint32_t stack_ptr () const { return _stack_ptr; }
    std::size_t   stack_size() const { return _stack_end - _stack_begin; }
    std::size_t   stack_used() const { return _stack_end - _stack_ptr - sizeof(std::uint32_t); }
    std::size_t   stack_left() const { return _stack_begin >= _stack_ptr ? 0ul : (_stack_ptr - _stack_begin); }
};


template<>
class coroutine<void()> : ::mw::detail::coroutine::impl
{
    bool _started = false;
    bool _exited  = false;

    template<typename T>
    friend struct yield_t;

public:

    typedef void return_type;
    typedef void push_type;
    typedef yield_t<void()> yield_type;

    template<typename StackContainer>
    coroutine(StackContainer & sc) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size()) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc.data()),
                reinterpret_cast<std::uintptr_t>(sc.data() + sc.size())
            }
            )
    {}

    template<typename T, std::size_t Size>
    coroutine(T(&sc)[Size]) : ::mw::detail::coroutine::impl(
            {
                reinterpret_cast<std::uintptr_t>(sc + Size) - sizeof(std::uint32_t),
                reinterpret_cast<std::uintptr_t>(sc),
                reinterpret_cast<std::uintptr_t>(sc + Size)
            }
            )
    {}


    coroutine(const coroutine & cr) = delete;
    coroutine(coroutine && cr) = default;

    coroutine& operator=(const coroutine & cr) = delete;
    coroutine& operator=(coroutine && cr) = default;


    template<typename StackContainer>
    coroutine fork(StackContainer & sc)
    {
        coroutine cr(sc);
        cr._stack_ptr = cr._stack_end - (_stack_end - _stack_ptr);
        std::copy(_stack_ptr, _stack_end, cr._stack_ptr);
        return cr;
    }

    void yield_()
    {
        mw::detail::coroutine::switch_context<void>(this);
    }

    void reenter()
    {
        mw::detail::coroutine::switch_context<void>(this);
    }

    template<typename Function>
    void spawn(Function && func)
    {
        auto executor = +[](coroutine * const this_, typename std::remove_reference<Function>::type *func_p)
        {
            this_->_started = true;
            Function func = static_cast<Function>(*func_p);
            func({this_});
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void>(this, reinterpret_cast<void*>(&func), reinterpret_cast<void*>(executor));
    }

    void spawn(return_type(&func)(yield_type)) {return spawn(&func);}
    void spawn(return_type(*func)(yield_type))
    {
        auto executor = +[](coroutine * const this_, return_type(*func)(yield_type))
        {
            this_->_started = true;
            func({this_});
            this_->_exited = true;

            mw::detail::coroutine::switch_context<void>(this_);
        };
        mw::detail::coroutine::make_context<void>(this, reinterpret_cast<void*>(func), reinterpret_cast<void*>(executor));
    }

    void operator()(){reenter();}

    bool started() const {return _started;}
    bool  exited() const {return _exited;}

    std::uint32_t stack_ptr () const { return _stack_ptr; }
    std::size_t   stack_size() const { return _stack_end - _stack_begin; }
    std::size_t   stack_used() const { return _stack_end - _stack_ptr - sizeof(std::uint32_t); }
    std::size_t   stack_left() const { return _stack_begin >= _stack_ptr ? 0ul : (_stack_ptr - _stack_begin); }
};


template<typename Return, typename PushType>
PushType yield_t<Return(PushType)>::operator()(Return rt)
{
    return static_cast<Return>(_cr->yield_(static_cast<Return>(rt)));
}

template<typename Return>
void yield_t<Return()>::operator()(Return rt) {_cr->yield_(static_cast<Return>(rt));}


template<typename PushType>
PushType yield_t<void(PushType)>::operator()() {return static_cast<PushType>(_cr->yield_());}

void yield_t<void()>::operator()() {_cr->yield_();}




template<typename Return, typename PushType>
std::uint32_t yield_t<Return(PushType)>::stack_ptr () const
{
    return _cr->stack_ptr();
}

template<typename Return, typename PushType>
std::size_t   yield_t<Return(PushType)>::stack_size() const
{
    return _cr->stack_size();
}

template<typename Return, typename PushType>
std::size_t   yield_t<Return(PushType)>::stack_used() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr)); //get the stack_ptr
    return _cr->_stack_end - stack_ptr + sizeof(std::uint32_t);
}

template<typename Return, typename PushType>
std::size_t   yield_t<Return(PushType)>::stack_left() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr) : : "sp"); //get the stack_ptr
    return _cr->_stack_begin >= stack_ptr ? 0ul : (stack_ptr - _cr->_stack_begin);
}

template<typename Return>
std::uint32_t yield_t<Return()>::stack_ptr () const
{
    return _cr->stack_ptr();
}

template<typename Return>
std::size_t   yield_t<Return()>::stack_size() const
{
    return _cr->stack_size();
}

template<typename Return>
std::size_t   yield_t<Return()>::stack_used() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr)); //get the stack_ptr
    return _cr->_stack_end - stack_ptr + sizeof(std::uint32_t);
}

template<typename Return>
std::size_t   yield_t<Return()>::stack_left() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr) : : "sp"); //get the stack_ptr
    return _cr->_stack_begin >= stack_ptr ? 0ul : (stack_ptr - _cr->_stack_begin);
}

template<typename PushType>
std::uint32_t yield_t<void(PushType)>::stack_ptr () const
{
    return _cr->stack_ptr();
}

template<typename PushType>
std::size_t   yield_t<void(PushType)>::stack_size() const
{
    return _cr->stack_size();
}

template<typename PushType>
std::size_t   yield_t<void(PushType)>::stack_used() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr)); //get the stack_ptr
    return _cr->_stack_end - stack_ptr + sizeof(std::uint32_t);
}

template<typename PushType>
std::size_t   yield_t<void(PushType)>::stack_left() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr) : : "sp"); //get the stack_ptr
    return _cr->_stack_begin >= stack_ptr ? 0ul : (stack_ptr - _cr->_stack_begin);
}


std::uint32_t yield_t<void()>::stack_ptr () const
{
    return _cr->stack_ptr();
}

std::size_t   yield_t<void()>::stack_size() const
{
    return _cr->stack_size();
}

std::size_t   yield_t<void()>::stack_used() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr)); //get the stack_ptr
    return _cr->_stack_end - stack_ptr + sizeof(std::uint32_t);
}

std::size_t   yield_t<void()>::stack_left() const
{
    std::uint32_t stack_ptr;
    __asm__("mov %0, sp" : "=r"(stack_ptr) : : "sp"); //get the stack_ptr
    return _cr->_stack_begin >= stack_ptr ? 0ul : (stack_ptr - _cr->_stack_begin);
}



}



#endif /* MW_COROUTINE_HPP_ */
