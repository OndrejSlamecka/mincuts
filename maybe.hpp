#ifndef MAYBE_H
#define MAYBE_H

/*
 * I REMOVED SOME FUNCIONALITY TO INCLUDE LESS LIBRARIES
 * -----------------------------------------------------
 *
 * Original: https://gist.github.com/brotchie/3134600
 *
 *
 * Minimal C++ implementation of Functor, Monad and Maybe.
 *
 * Requires c++0x variadic templates and lambda expressions:
 *
 *      g++ -std=c++0x main.cpp -o main
 *
 * fmap, monadic bind and return implementations for std::vector
 * and Maybe.
 *
 * Copyright 2012 James Brotchie <brotchie@gmail.com>
 *  https://github.com/brotchie/
 *  @brotchie
 *
 */

#include <iostream>
#include <algorithm>

using namespace std;

/* Monad */
template <template <typename...> class F>
struct Monad {
    template <typename A>
    static F<A> return_(A);

    template <typename A, typename B>
    static F<B> bind(F<A>, function<F<B>(A)>);
};

template <template <typename...> class F, typename A, typename B>
static F<B> bind(F<A> m, function<F<B>(A)> f) {
    return Monad<F>::bind(m, f);
}

template <template <typename...> class F, typename A>
static F<A> return_(A a) {
    return Monad<F>::return_(a);
}

template <template <typename...> class F, typename A, typename B>
static F<B> operator >=(F<A> m, function<F<B>(A)> f) {
    return Monad<F>::bind(m, f);
}

template <template <typename...> class F, typename A, typename B>
static F<B> operator >>(F<A> a, F<B> b) {
    function<F<B>(A)> f = [b](A){ return b; };
    return a >= f;
}

/* Maybe */
template <typename T>
class Maybe {
public:
    Maybe() : _empty(true){};
    explicit Maybe(T value) : _empty(false), _value(value){};

    T fromJust() const {
        if (isJust()) {
            return _value;
        } else {
            throw "Cannot get value from Nothing";
        }
    }

    bool isJust() const { return !_empty; }
    bool isNothing() const { return _empty; }

    static bool isJust(Maybe &m) { return m.isJust(); }
    static bool isNothing(Maybe &m) { return m.isNothing(); }
private:
    bool _empty;
    T _value;
};

template <typename T>
ostream& operator<<(ostream& s, const Maybe<T> m)
{
    if (m.isJust()) {
        return s << "Just " << m.fromJust();
    } else {
        return s << "Nothing";
    }
}


/* Monad Maybe */
template <>
struct Monad<Maybe> {
    template <typename A>
    static Maybe<A> return_(A v){
        return Maybe<A>(v);
    }

    template <typename A, typename B>
    static Maybe<B> bind(Maybe<A> m, function<Maybe<B>(A)> f){
        if (m.isNothing()){
            return Maybe<B>();
        } else {
            return f(m.fromJust());
        }
    }
};


template <typename A, typename B, typename C>
static function<C(A)> compose(function<B(A)> f1, function<C(B)> f2) {
    return [f1,f2](A v) -> C {
        return f2(f1(v));
    };
}

#endif // MAYBE_H
