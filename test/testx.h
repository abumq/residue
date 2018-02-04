//
//  Bismillah ar-Rahmaan ar-Raheem
//
//  TestX 1.0.4
//  Single C++11 header for creating typesafe test data
//
//  Copyright (c) 2017-present Muflihun Labs
//
//  This library is released under the MIT Licence.
//
//  https://github.com/muflihun/testx/
//  https://muflihun.github.io/
//  https://muflihun.com/
//
#ifndef MUFLIHUN_TEST_X_H
#define MUFLIHUN_TEST_X_H

#include <tuple>
#include <type_traits>
#include <vector>

namespace muflihun {
    namespace testx {
        
        /**
         * @brief Test case with variadic templates. 
         *
         * The test case item can be accessed using get<Index> function
         */
        template <typename... Args>
        class TestCase
        {
        public:
            template <std::size_t Index>
            using TestCaseItemType = typename std::remove_reference<
            decltype(std::get<Index>(std::tuple<Args...>()))
            >::type;
            
            TestCase(Args... args) : m_data(args...) {
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
        
        /**
         * @brief A test data wrapper
         */
        template <typename T>
        using TestDataW = std::vector<T>;
        
        /**
         * @brief A simple test case container that can be iterated using for (auto& testcase : data) ...
         */
        template <typename... T>
        using TestData = TestDataW<TestCase<T...>>;
        
        /**
         * @brief A TestCase wrapper to create test case with implicit type
         */
        template <typename... T>
        TestCase<T...> TestCaseW(T... t) { return TestCase<T...>(t...); }
        
    } // namespace testx
} // namespace muflihun

#endif // MUFLIHUN_TEST_X_H
