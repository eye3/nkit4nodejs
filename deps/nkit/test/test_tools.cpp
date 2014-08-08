#include "nkit/test.h"

namespace nkit_test
{
  using namespace nkit;

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(tools_bool_cast)
  {
    NKIT_TEST_ASSERT(bool_cast("Yes"));
    NKIT_TEST_ASSERT(bool_cast("yes"));
    NKIT_TEST_ASSERT(bool_cast("True"));
    NKIT_TEST_ASSERT(bool_cast("true"));
    NKIT_TEST_ASSERT(!bool_cast("tru"));
    NKIT_TEST_ASSERT(!bool_cast("Tru"));
    NKIT_TEST_ASSERT(!bool_cast("false"));
    NKIT_TEST_ASSERT(!bool_cast("False"));
    NKIT_TEST_ASSERT(!bool_cast("no"));
    NKIT_TEST_ASSERT(!bool_cast("No"));
    NKIT_TEST_ASSERT(!bool_cast("N"));
    NKIT_TEST_ASSERT(!bool_cast("n"));
    NKIT_TEST_ASSERT(!bool_cast("fals"));
    NKIT_TEST_ASSERT(!bool_cast("Fals"));
    NKIT_TEST_ASSERT(!bool_cast("0"));
    NKIT_TEST_ASSERT(bool_cast("1"));
    NKIT_TEST_ASSERT(bool_cast("2"));
    NKIT_TEST_ASSERT(bool_cast("3"));
    NKIT_TEST_ASSERT(bool_cast("4"));
    NKIT_TEST_ASSERT(bool_cast("5"));
    NKIT_TEST_ASSERT(bool_cast("6"));
    NKIT_TEST_ASSERT(bool_cast("7"));
    NKIT_TEST_ASSERT(bool_cast("8"));
    NKIT_TEST_ASSERT(bool_cast("9"));
    NKIT_TEST_ASSERT(bool_cast("1a"));
    NKIT_TEST_ASSERT(bool_cast("10"));
    NKIT_TEST_ASSERT(!bool_cast("a2"));
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(tools_simple_split_key_value)
  {
    std::string k("key"), v("value");

    std::string d(","), str(k + d + v), _k, _v;
    simple_split(str, d, &_k, &_v);
    NKIT_TEST_ASSERT_WITH_TEXT(k == _k, _k);
    NKIT_TEST_ASSERT_WITH_TEXT(v == _v, _v);

    str = k + " " + d + v;
    simple_split(str, d, &_k, &_v);
    NKIT_TEST_ASSERT_WITH_TEXT(k == _k, _k);
    NKIT_TEST_ASSERT_WITH_TEXT(v == _v, _v);

    d = ",,";
    str = k + d + " " + v;
    simple_split(str, d, &_k, &_v);
    NKIT_TEST_ASSERT_WITH_TEXT(k == _k, _k);
    NKIT_TEST_ASSERT_WITH_TEXT(v == _v, _v);

    str = k + " " + d + " " + v;
    simple_split(str, d, &_k, &_v);
    NKIT_TEST_ASSERT_WITH_TEXT(k == _k, _k);
    NKIT_TEST_ASSERT_WITH_TEXT(v == _v, _v);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(tools_simple_split)
  {
    std::string s0("qwe"), s1("asdasd"), s2("zxczxc"), s3("3333"), s4,
        s5("55555555");

    std::string d(","),
        str(s0 + d + " " + s1 + " " + d + " " + s2 + d + s3 + d);
    StringVector v;
    simple_split(str, d, &v);
    NKIT_TEST_ASSERT(v.size() == 5);
    NKIT_TEST_ASSERT_WITH_TEXT(v[0] == s0, v[0]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[1] == s1, v[1]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[2] == s2, v[2]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[3] == s3, v[3]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[4] == s4, v[4]);

    str += s5;
    simple_split(str, d, &v);
    NKIT_TEST_ASSERT(v.size() == 5);
    NKIT_TEST_ASSERT_WITH_TEXT(v[0] == s0, v[0]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[1] == s1, v[1]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[2] == s2, v[2]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[3] == s3, v[3]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[4] == s5, v[4]);

    str = s0;
    simple_split(str, d, &v);
    NKIT_TEST_ASSERT(v.size() == 1);
    NKIT_TEST_ASSERT_WITH_TEXT(v[0] == s0, v[0]);

    d = ",,";
    str = s0 + d + d;
    simple_split(str, d, &v);
    NKIT_TEST_ASSERT(v.size() == 3);
    NKIT_TEST_ASSERT_WITH_TEXT(v[0] == s0, v[0]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[1].empty(), v[1]);
    NKIT_TEST_ASSERT_WITH_TEXT(v[2].empty(), v[2]);
  }

} // namespace nkit_test
