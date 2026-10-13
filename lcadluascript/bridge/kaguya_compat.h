#pragma once

// kaguya, plus the specialisations C++17 needs it to have.
//
// The lcadluascript bridge headers all come through here. Fourteen other
// translation units under lcUI, unittest and luacmdinterface still include
// kaguya directly; that is fine while none of them binds a noexcept member
// function, and this is the header to route them through when one does.
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

// Guarded, because below C++17 `Ret (T::*)(Args...) noexcept` names the same
// type as `Ret (T::*)(Args...)` -- so these would not add specialisations,
// they would redefine kaguya's own, and the header would not compile at all.
// The guard makes the file say what it is for rather than depending on the
// project's standard staying where it is.
#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)

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

#endif  // C++17 or newer
