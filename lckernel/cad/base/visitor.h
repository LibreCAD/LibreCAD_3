//
// Created by R. van Twisk on 3/28/15.
//

#pragma once

#include <memory>


namespace lc {
    namespace geo {
        class Base;
        class Vector;
        class Coordinate;
        class Area;
        class Circle;
        class Vector;
        class Ellipse;
        class Spline;
        class Arc;
    }

    class EntityDispatch;
    class Visitable;

    namespace entity {
        class Arc;
        typedef std::shared_ptr<Arc> Arc_SPtr;
        typedef std::shared_ptr<const Arc> Arc_CSPtr;

        class Point;
        typedef std::shared_ptr<Point> Point_SPtr;
        typedef std::shared_ptr<const Point> Point_CSPtr;

        class Line;
        typedef std::shared_ptr<Line> Line_SPtr;
        typedef std::shared_ptr<const Line> Line_CSPtr;

        class Text;
        typedef std::shared_ptr<Text> Text_SPtr;
        typedef std::shared_ptr<const Text> Text_CSPtr;

        class Spline;
        typedef std::shared_ptr<Spline> Spline_SPtr;
        typedef std::shared_ptr<const Spline> Spline_CSPtr;

        class Ellipse;
        typedef std::shared_ptr<Ellipse> Ellipse_SPtr;
        typedef std::shared_ptr<const Ellipse> Ellipse_CSPtr;

        class Circle;
        typedef std::shared_ptr<Circle> Circle_SPtr;
        typedef std::shared_ptr<const Circle> Circle_CSPtr;

        class CADEntity;
        typedef std::shared_ptr<CADEntity> CADEntity_SPtr;
        typedef std::shared_ptr<const CADEntity> CADEntity_CSPtr;

        class DimAligned;
        typedef std::shared_ptr<DimAligned> DimAligned_SPtr;
        typedef std::shared_ptr<const DimAligned> DimAligned_CSPtr;

        class DimAngular;
        typedef std::shared_ptr<DimAngular> DimAngular_SPtr;
        typedef std::shared_ptr<const DimAngular> DimAngular_CSPtr;

        class DimDiametric;
        typedef std::shared_ptr<DimDiametric> DimDiametric_SPtr;
        typedef std::shared_ptr<const DimDiametric> DimDiametric_CSPtr;

        class DimLinear;
        typedef std::shared_ptr<DimLinear> DimLinear_SPtr;
        typedef std::shared_ptr<const DimLinear> DimLinear_CSPtr;

        class DimRadial;
        typedef std::shared_ptr<DimRadial> DimRadial_SPtr;
        typedef std::shared_ptr<const DimRadial> DimRadial_CSPtr;

        class LWPolyline;
        typedef std::shared_ptr<LWPolyline> LWPolyline_SPtr;
        typedef std::shared_ptr<const LWPolyline> LWPolyline_CSPtr;

    }
}




// sequence of size_t // not in C++11
template <std::size_t ...> struct index_sequence {};

// Create index_sequence<0, >
template <std::size_t N, std::size_t ...Is>
struct make_index_sequence : make_index_sequence <N - 1, N - 1, Is... > {};

template <std::size_t ... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...> {};

// Generic IVisitor
// Do: using MyIVisitor = IVisitorTs<Child1, Child2, ...>
template <typename ... Ts> class IVisitorTs;

template <typename T, typename ... Ts>
class IVisitorTs<T, Ts...> : public IVisitorTs<Ts...>
{
public:
    using tuple_type = std::tuple<T, Ts...>;
    using IVisitorTs<Ts...>::visit;

    virtual void visit(const T& t) = 0;
};

template <typename T> class IVisitorTs<T>
{
public:
    using tuple_type = std::tuple<T>;

    virtual void visit(const T& t) = 0;
};

namespace detail {

// retrieve the index of T in Ts...
    template <typename T, typename ... Ts> struct get_index;

    template <typename T, typename ... Ts>
    struct get_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

    template <typename T, typename Tail,  typename ... Ts>
    struct get_index<T, Tail, Ts...> :
            std::integral_constant < std::size_t, 1 + get_index<T, Ts...>::value > {};

// retrieve the index of T in Tuple<Ts...>
    template <typename T, typename Tuple> struct get_index_in_tuple;

    template <typename T, template <typename...> class C, typename ... Ts>
    struct get_index_in_tuple<T, C<Ts...>> : get_index<T, Ts...> {};

// get element of a multiarray
    template <std::size_t I>
    struct multi_array_getter
    {
        template <typename T, std::size_t N>
        static constexpr auto get(const T& a, const std::array<std::size_t, N>& index)
        -> decltype(multi_array_getter<I - 1>::get(a[index[N - I]], index))
        {
            return multi_array_getter<I - 1>::get(a[index[N - I]], index);
        }
    };

    template <>
    struct multi_array_getter<0>
    {
        template <typename T, std::size_t N>
        static constexpr auto get(const T& a, const std::array<std::size_t, N>& index)
        -> decltype(a)
        {
            return a;
        }
    };

// Provide an implementation of visitor
// by forwarding to C implementation (which may be non virtual)
    template <typename IVisitor, typename C, typename...Ts> struct IVisitorImpl;

    template <typename IVisitor, typename C, typename T, typename...Ts>
    struct IVisitorImpl<IVisitor, C, T, Ts...> : IVisitorImpl<IVisitor, C, Ts...>
    {
        virtual void visit(const T& t) override { C::visit(t); }
    };

    template <typename IVisitor, typename C, typename T>
    struct IVisitorImpl<IVisitor, C, T> : IVisitor, C
    {
        virtual void visit(const T& t) override { C::visit(t); }
    };

// helper to expand child type to IVisitorImpl
    template <typename IVisitor, typename C>
    struct IVisitorImplType;

    template <typename ... Ts, typename C>
    struct IVisitorImplType<IVisitorTs<Ts...>, C>
    {
        using type = IVisitorImpl<IVisitorTs<Ts...>, C, Ts...>;
    };

// Create an multi array of pointer of function
// (with all combinaisons of overload).
    template <typename Ret, typename F, typename Arg>
    class GetAllOverload
    {
    private:
        template <typename...Ts>
        struct Functor
        {
            // function which will be in array.
            static Ret call(F&f, const Arg& arg)
            {
                return call_helper(f, arg, make_index_sequence<sizeof...(Ts)>());
            }
        private:
            // The final dispatched function
            template <std::size_t ... Is>
            static Ret call_helper(F&f, const Arg& arg, index_sequence<Is...>)
            {
                using RetTuple = std::tuple<const Ts&...>;
                // static cast is suffisant if arg is the abstract type
                // when given arg is concrete type, reinterpret_cast is required.
                // TODO: build a smaller table with only possible value to avoid that
//                return f(static_cast<typename std::tuple_element<Is, RetTuple>::type>(std::get<Is>(arg))...);
                return f(reinterpret_cast<typename std::tuple_element<Is, RetTuple>::type>(std::get<Is>(arg))...);
            }
        };

        // helper class to create the multi array of function pointer
        template <std::size_t N, typename Tuple, typename...Ts>
        struct Builder;

        template <typename...Ts, typename...Ts2>
        struct Builder<1, std::tuple<Ts...>, Ts2...>
        {
            using RetType = std::array<Ret (*)(F&, const Arg&), sizeof...(Ts)>;

            static constexpr RetType build()
            {
                return RetType{ &Functor<Ts2..., Ts>::call... };
            }
        };

        template <std::size_t N, typename ...Ts, typename...Ts2>
        struct Builder<N, std::tuple<Ts...>, Ts2...>
        {
            template <typename T>
            using RecType = Builder<N - 1, std::tuple<Ts...>, Ts2..., T>;
            using T0 = typename std::tuple_element<0, std::tuple<Ts...>>::type;
            using RetType = std::array<decltype(RecType<T0>::build()), sizeof...(Ts)>;

            static constexpr RetType build() {
                return RetType{ RecType<Ts>::build()... };
            }
        };

    public:
        template <std::size_t N, typename VisitorTuple>
        static constexpr auto get()
        -> decltype(Builder<N, VisitorTuple>::build())
        {
            return Builder<N, VisitorTuple>::build();
        }
    };

    template <typename Ret, typename IVisitor, typename F, std::size_t N>
    class dispatcher
    {
    private:
        std::array<std::size_t, N> index;

        struct visitorCallImpl
        {
            template <typename T>
            void visit(const T&) const
            {
                *index = get_index_in_tuple<T, IVisitor>::value;
            }

            void setIndexPtr(std::size_t& index) { this->index = &index; }
        private:
            std::size_t* index = nullptr;
        };

        template <std::size_t I, typename Tuple>
        void set_index(const Tuple&t)
        {
            using VisitorType = typename IVisitorImplType<IVisitor, visitorCallImpl>::type;
            VisitorType visitor;
            visitor.setIndexPtr(index[I]);

            std::get<I>(t).accept(visitor);
        }
    public:
        template <typename Tuple, std::size_t ... Is>
        Ret operator () (F&& f, const Tuple&t, index_sequence<Is...>)
        {
            const int dummy[] = {(set_index<Is>(t), 0)...};
            static_cast<void>(dummy); // silent the warning unused varaible
            constexpr auto a = GetAllOverload<Ret, F&&, Tuple>::
            template get<sizeof...(Is), typename IVisitor::tuple_type>();
            auto func = multi_array_getter<N>::get(a, index);
            return (*func)(f, t);
        }
    };

} // namespace detail

template <typename Ret, typename Visitor, typename F, typename ... Ts>
Ret visitorDispatcher(F&& f, Ts&...args)
{
    constexpr std::size_t size = sizeof...(Ts);
    detail::dispatcher<Ret, Visitor, F&&, size> d;
    return d(std::forward<F>(f), std::tie(args...), make_index_sequence<size>());
}

namespace lc {



    using GeoEntityVisitor = IVisitorTs<
            lc::Visitable,
            geo::Vector, geo::Circle, geo::Arc, geo::Area, geo::Ellipse,
            entity::CADEntity, entity::Point, entity::Line, entity::Arc, entity::Circle, entity::Ellipse,
            entity::LWPolyline
    >;

    class Visitable {
    public:
        virtual ~Visitable() = default;
        virtual void accept(GeoEntityVisitor &) const = 0;
    };
}
