#pragma once

// The project's entry point to kaguya. Include this rather than kaguya's own
// headers from anything that registers a Lua binding.
//
// C++17 made noexcept part of a function's type. kaguya specialises
// util::FunctionSignature on the shapes it knows -- `Ret (T::*)(Args...)`,
// `... const`, `... const &` -- and none of them match a member function
// declared noexcept, so binding one leaves the primary template undefined:
//
//   implicit instantiation of undefined template
//   'kaguya::util::FunctionSignature<const lc::geo::Coordinate &
//    (Dimension::*)() const noexcept>'
//
// followed by a cascade from _call_apply and _type_name_apply, which are
// driven entirely by that trait. Under C++14 the same code compiled, because
// noexcept was not part of the type and the plain specialisation matched.
//
// The specialisations are supplied here rather than patched into kaguya,
// which is a submodule pinned at an upstream revision -- the same reasoning as
// the force-included <cstdint> in the root CMakeLists. Drop this once kaguya
// carries them upstream.
//
// noexcept is discarded rather than represented: kaguya reads the trait only
// for the return type and the argument list, and a Lua binding has no use for
// the guarantee.

#include <kaguya/include/kaguya/state.hpp>

namespace kaguya {
namespace util {

template <typename T, typename Ret, typename... Args>
struct FunctionSignature<Ret (T::*)(Args...) noexcept> {
    typedef FunctionSignatureType<Ret, T&, Args...> type;
};

template <typename T, typename Ret, typename... Args>
struct FunctionSignature<Ret (T::*)(Args...) const noexcept> {
    typedef FunctionSignatureType<Ret, const T&, Args...> type;
};

#if defined(_MSC_VER) && _MSC_VER >= 1900 || defined(__cpp_ref_qualifiers)
template <typename T, typename Ret, typename... Args>
struct FunctionSignature<Ret (T::*)(Args...) const& noexcept> {
    typedef FunctionSignatureType<Ret, const T&, Args...> type;
};

template <typename T, typename Ret, typename... Args>
struct FunctionSignature<Ret (T::*)(Args...) const&& noexcept> {
    typedef FunctionSignatureType<Ret, const T&, Args...> type;
};
#endif

template <class Ret, class... Args>
struct FunctionSignature<Ret (*)(Args...) noexcept> {
    typedef FunctionSignatureType<Ret, Args...> type;
};

template <class Ret, class... Args>
struct FunctionSignature<Ret(Args...) noexcept> {
    typedef FunctionSignatureType<Ret, Args...> type;
};

}  // namespace util
}  // namespace kaguya
