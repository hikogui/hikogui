
#include "string_tag.hpp"

namespace TTauri {

template<string_tag Tag, typename Type>
struct tagged_tuple_element {
    T value;
};

template<string_tag... Tags, typename... Types>
struct tagged_tuple_impl: tagged_tuple_element<Tags,Types>... {
};

template <size_t I, typename Head, typename... Tail>
struct type_at_index
{
    using type = typename type_at_index<I-1, Tail...>::type;
};

template <typename Head, typename... Tail>
struct type_at_index<0, Head, Tail...>
{
    using type = Head;
};

template <size_t I, typename... Types>
using type_at_index_t = typename type_at_index<I, Types...>::type;

template <size_t I, string_tag Head, string_tag... Tail>
struct tag_at_index
{
    static constexpr string_tag value = tag_at_index<I-1, Tail...>::value;
};

template <string_tag Head, string_tag... Tail>
struct tag_at_index<0, Head, Tail...>
{
    static constexpr string_tag value = Head;
};

template <size_t I, string_tag... Tags>
constexpr string_tag tag_at_index_v = tag_at_index<I, Tags...>::value;

}
