//
//  Bismillah ar-Rahmaan ar-Raheem
//
//  TestX 1.0.2
//  Single header, header only helper for creating test data
//
//  Copyright (c) 2017-present Muflihun Labs
//
//  This library is released under the MIT Licence.
//
//  https://github.com/muflihun/testx
//  https://muflihun.github.io
//  http://muflihun.com
//
#ifndef MUFLIHUN_TEST_X_H
#define MUFLIHUN_TEST_X_H

#include <tuple>
#include <type_traits>
#include <vector>

namespace muflihun {
namespace testx {

template <typename... Args>
class TestCaseWrapper
{
public:    
    template <std::size_t Index>
    using TestCaseItemType = typename std::remove_reference<
        decltype(std::get<Index>(std::tuple<Args...>()))
            >::type;

    TestCaseWrapper(Args... args) : m_data(args...) {
    }
    
    template <std::size_t Index>
    auto get() const -> TestCaseItemType<Index> { 
        return std::get<Index>(m_data); 
    }
    
    const std::tuple<Args...>& data() const {
        return m_data;
    }
private:
    std::tuple<Args...> m_data;
};

template <typename... T>
TestCaseWrapper<T...> TestCase(T... t) { return TestCaseWrapper<T...>(t...); }

template <typename... T>
using TestData = const std::vector<TestCaseWrapper<T...>>;

} // namespace testx
} // namespace muflihun

#endif // MUFLIHUN_TEST_X_H

