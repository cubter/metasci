/* 
 * Copyright (c) 2022 - present, GitHub: @cubter
 * 
 * See COPYING.txt in the project root for license information.
 */

// This is an optional part, in which I just wanted to learn to use SFINAE.
// 
// It'd be probably more efficient to define which functions to use 
// based on preprocessor directives, because the whole point of `try_emplace` 
// is not to construct the value if the same key already exists -- and it's 
// missing here, as the object is passed using `move` semantics.
#include <iostream>
#include <unordered_map>

namespace metasci
{
namespace cond 
{
template<typename>
struct sfinae_true : std::true_type{ };

template<typename T, typename... Val>
static auto check(int)
    -> sfinae_true<decltype(std::declval<T>().try_emplace(std::declval<Val...>()))>;
template<typename, typename... Val>
static auto check(long) -> std::false_type;

template<typename T, typename... Val>
struct can_try_emplace : decltype(check<T, Val...>(0)) 
{ };

// If unordered set is in use, or if it's unordered map and the files were 
// compiled with C++ < 17, `emplace` will be used, otherwise -- `try_emplace`.
template<typename Container, typename Iterator, typename... Val>
struct Emplacer
{ 
    template<bool B, typename Rettype>
    using enable_if_t = typename std::enable_if<B, Rettype>::type;

    // Returns a pair of iterator and emplacement status (1 if emplacement 
    // occurred, 0 if such key already existed)
    template<typename C = Container>
    auto emplace_to(C &c, Val &&...vs) 
        -> enable_if_t<can_try_emplace<C, Val...>::value, std::pair<Iterator, bool>>  
    {
        auto emp_res = c.try_emplace(std::forward<Val...>(vs...));
        return std::make_pair(emp_res.first, emp_res.second);
    }
    
    template<typename C = Container>
    auto emplace_to(C &c, Val &&...vs) 
         -> enable_if_t<!can_try_emplace<C, Val...>::value, std::pair<Iterator, bool>> 
    {
        auto emp_res = c.emplace(std::forward<Val...>(vs...));
        return std::make_pair(emp_res.first, emp_res.second);
    } 

};
}
}
