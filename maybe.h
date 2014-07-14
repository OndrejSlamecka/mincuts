#ifndef MAYBE_H
#define MAYBE_H

/*
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
#include <vector>
#include <iterator>
#include <algorithm>

using namespace std;

/* Functor */
template <template <typename...> class F>
struct Functor {
    template <typename A, typename B>
    static function <F<B>(F<A>)> fmap(function <B(A)>);
};

template <template <typename...> class F, typename A, typename B>
static function <F<B>(F<A>)> fmap(function <B(A)> f) {
    return Functor<F>::fmap(f);
}

template <template <typename...> class F, typename A, typename B>
static F<B> fmap_(function <B(A)> f, F<A> v) {
    return Functor<F>::fmap(f)(v);
}

template <template <typename...> class F, typename A, typename B>
static F<B> operator %(function <B(A)> f, F<A> v) {
    return Functor<F>::fmap(f)(v);
}

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

/* Functor Maybe */
template <>
struct Functor<Maybe> {
    template <typename A, typename B>
    static function <Maybe<B>(Maybe<A>)> fmap(function <B(A)> f) {
        return [f](Maybe<A> m) -> Maybe<B> {
            if (m.isNothing()) {
                return Maybe<B>();
            } else {
                return Maybe<B>(f(m.fromJust()));
            }
        };
    }
};

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

/* Functor vector */
template <>
struct Functor<vector> {
    template <typename A, typename B>
    static function <vector<B>(vector<A>)> fmap(function <B(A)> f) {
        return [f](vector<A> v){
            vector<B> result;
            transform(v.begin(), v.end(), back_inserter(result), f);
            return result;
        };
    }
};

/* Monad vector */
template <>
struct Monad<vector> {
    template <typename A>
    static vector<A> return_(A v){
        return vector<A>{v};
    }

    template <typename A, typename B>
    static vector<B> bind(vector<A> m, function<vector<B>(A)> f){
        vector<B> v;
        for_each(m.begin(), m.end(), [f, &v](A a){
            vector<B> result = f(a);
            copy(result.begin(), result.end(), back_inserter(v));
        });
        return v;
    }
};

template <typename A, typename B, typename C>
static function<C(A)> compose(function<B(A)> f1, function<C(B)> f2) {
    return [f1,f2](A v) -> C {
        return f2(f1(v));
    };
}

#endif // MAYBE_H
