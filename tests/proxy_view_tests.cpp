// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "utils.h"
#include <gtest/gtest.h>
#include <proxy/proxy.h>

namespace proxy_view_tests_details {

struct TestFacade
    : pro::facade_builder                                              //
      ::add_convention<pro::operator_dispatch<"+=">, void(int)>        //
      ::add_convention<utils::spec::FreeToString, std::string() const> //
      ::support<pro::skills::as_view>                                  //
      ::build {};

template <class T>
concept SupportsIntPlusEqual = requires(T a, int b) { a += b; };
template <class T>
concept SupportsToString = requires(T a) {
  { ToString(a) } -> std::same_as<std::string>;
};

static_assert(!std::is_copy_constructible_v<pro::proxy<TestFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<TestFacade>>);
static_assert(
    SupportsIntPlusEqual<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(
    SupportsToString<decltype(*std::declval<pro::proxy<TestFacade>>())>);
static_assert(sizeof(pro::proxy<TestFacade>) == 3 * sizeof(void*));

static_assert(
    std::is_trivially_copy_constructible_v<pro::proxy_view<TestFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy_view<TestFacade>>);
static_assert(SupportsIntPlusEqual<
              decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(
    SupportsToString<decltype(*std::declval<pro::proxy_view<TestFacade>>())>);
static_assert(sizeof(pro::proxy_view<TestFacade>) == 3 * sizeof(void*));

} // namespace proxy_view_tests_details

namespace details = proxy_view_tests_details;

TEST(ProxyViewTests, TestViewOfNull) {
  pro::proxy<details::TestFacade> p1;
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_FALSE(p2.has_value());
}

TEST(ProxyViewTests, TestViewIndependentUse) {
  int a = 123;
  pro::proxy_view<details::TestFacade> p = &a;
  *p += 3;
  ASSERT_EQ(ToString(*p), "126");
  ASSERT_EQ(a, 126);
}

TEST(ProxyViewTests, TestViewOfOwning) {
  pro::proxy<details::TestFacade> p1 =
      pro::make_proxy<details::TestFacade>(123);
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p2 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  p1.reset();
  // p2 becomes dangling
}

TEST(ProxyViewTests, TestViewOfNonOwning) {
  int a = 123;
  pro::proxy<details::TestFacade> p1 = &a;
  pro::proxy_view<details::TestFacade> p2 = p1;
  ASSERT_TRUE(p1.has_value());
  ASSERT_TRUE(p2.has_value());
  *p2 += 3;
  ASSERT_EQ(ToString(*p1), "126");
  p1.reset();
  ASSERT_EQ(ToString(*p2), "126");
  ASSERT_EQ(a, 126);
}

TEST(ProxyViewTests, TestOverloadShadowing) {
  struct TestFacade
      : pro::facade_builder                                                //
        ::add_convention<pro::operator_dispatch<"()">, int(), int() const> //
        ::support<pro::skills::as_view>                                    //
        ::build {};
  struct TestImpl {
    int operator()() { return 0; }
    int operator()() const { return 1; }
  };
  pro::proxy<TestFacade> p1 = pro::make_proxy<TestFacade, TestImpl>();
  pro::proxy_view<TestFacade> p2 = p1;
  ASSERT_EQ((*p1)(), 0);
  ASSERT_EQ((std::as_const(*p1))(), 1);
  ASSERT_EQ((*p2)(), 0);
  ASSERT_EQ((std::as_const(*p2))(), 1);
}

TEST(ProxyViewTests, TestUpwardConversion_FromNull) {
  struct TestFacade1 : pro::facade_builder::build {};
  struct TestFacade2
      : pro::facade_builder             //
        ::add_facade<TestFacade1, true> // Supports upward conversion
        ::support<pro::skills::as_view> //
        ::build {};
  pro::proxy<TestFacade2> p1;
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<TestFacade1> p3 = p2;
  ASSERT_FALSE(p1.has_value());
  ASSERT_FALSE(p2.has_value());
  ASSERT_FALSE(p3.has_value());
}

TEST(ProxyViewTests, TestUpwardConversion_FromValue) {
  struct TestFacade1
      : pro::facade_builder                                              //
        ::add_convention<utils::spec::FreeToString, std::string() const> //
        ::build {};
  struct TestFacade2
      : pro::facade_builder                               //
        ::support_copy<pro::constraint_level::nontrivial> //
        ::add_facade<TestFacade1, true> // Supports upward conversion
        ::support<pro::skills::as_view> //
        ::build {};
  pro::proxy<TestFacade2> p1 = pro::make_proxy<TestFacade2>(123);
  pro::proxy_view<TestFacade2> p2 = p1;
  pro::proxy_view<TestFacade1> p3 = p2;
  ASSERT_EQ(ToString(*p1), "123");
  ASSERT_EQ(ToString(*p2), "123");
  ASSERT_EQ(ToString(*p3), "123");
}
