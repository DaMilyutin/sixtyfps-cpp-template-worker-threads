#pragma once
#include <utility>

template<typename T, typename G, typename S>
struct Property
{
    Property(G&& in_g, S&& in_s)
    : get(std::forward<G>(in_g))
    , set(std::forward<S>(in_s))
    {}

    class Blocking
    {
    public:
        Blocking(Property& p): weak(p) {}

        auto get() const 
        { 
            auto& p = weak;
            return sixtyfps::blocking_invoke_from_event_loop([&p](){ return p.get(); });
        }

        void set(T const& t) 
        {
            auto& p = weak; 
            sixtyfps::blocking_invoke_from_event_loop([&p, t](){ p.set(t); return 0; }); 
        }

        template<typename F>
        void update(F f) { 
            auto& p = weak;
            sixtyfps::blocking_invoke_from_event_loop([&p, &f]() { p.update(std::forward<F>(f)); return 0; });
        }

        void operator=(T const& t) { set(t); }
        operator T() const { return get(); }

        template<typename X>
        void operator+=(X const& x) { update([x](auto const& t) { return t + x; }); } 

        template<typename X>
        void operator-=(X const& x) { update([x](auto const& t) { return t - x; }); } 

        template<typename X>
        void operator*=(X const& x) { update([x](auto const& t) { return t * x; }); }

        template<typename X>
        void operator/=(X const& x) { update([x](auto const& t) { return t / x; }); }

    private:
        Property& weak;
    };

    class Safe
    {
    public:
        Safe(Property& p): weak(p) {}

        auto get() const 
        { 
            auto& p = weak;
            return sixtyfps::invoke_from_event_loop([&p](){ return p.get();});
        }

        void set(T const& t) 
        {
            auto& p = weak; 
            sixtyfps::invoke_from_event_loop([&p, t](){ p.set(t);}); 
        }

        template<typename F>
        void update(F f) {
            auto& p = weak;
            sixtyfps::invoke_from_event_loop([&p, &f]() { p.update(std::forward<F>(f)); });
        }

        void operator=(T const& t) { set(t); }
        operator T() const { return get(); }

        template<typename X>
        void operator+=(X const& x) { update([x](auto const& t) { return t + x; }); } 

        template<typename X>
        void operator-=(X const& x) { update([x](auto const& t) { return t - x; }); } 

        template<typename X>
        void operator*=(X const& x) { update([x](auto const& t) { return t * x; }); }

        template<typename X>
        void operator/=(X const& x) { update([x](auto const& t) { return t / x; }); }

    private:
        Property& weak;
    };


    template<typename F>
    void update(F f) { set(f(get())); } 

    template<typename X>
    void operator+=(X const& x) { update([x](auto const& t) { return t + x; }); } 

    template<typename X>
    void operator-=(X const& x) { update([x](auto const& t) { return t - x; }); } 

    template<typename X>
    void operator*=(X const& x) { update([x](auto const& t) { return t * x; }); }

    template<typename X>
    void operator/=(X const& x) { update([x](auto const& t) { return t / x; }); } 

    Blocking blocking() { return Blocking(*this); }
    Safe     safe()     { return Safe(*this); }
    
    G get;
    S set;
};

template<typename T, typename G, typename S>
inline Property<T, G, S> make_property(G&& g, S&& s) { return Property<T, G, S>(std::forward<G>(g), std::forward<S>(s)); }

