#include "nkit/test.h"

namespace nkit_test
{
  using namespace nkit;

  //---------------------------------------------------------------------------
  void _copy_file(const std::string & content)
  {
    std::string error;
    std::string text;
    static const std::string SRC_FILE("./data/src.txt");
    static const std::string DST_FILE("./data/dst.txt");
    NKIT_TEST_ASSERT_WITH_TEXT(
        string_to_text_file(SRC_FILE, content, &error), error);
    NKIT_TEST_ASSERT(path_is_file(SRC_FILE));
    NKIT_TEST_ASSERT_WITH_TEXT(
        copy_file(SRC_FILE, DST_FILE, &error), error);
    NKIT_TEST_ASSERT(path_is_file(DST_FILE));
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(DST_FILE, &text, &error), error);
    NKIT_TEST_EQ(text, content);
    NKIT_TEST_ASSERT_WITH_TEXT(
        delete_file(DST_FILE, &error), error);
    NKIT_TEST_ASSERT(!path_is_file(DST_FILE));
    NKIT_TEST_ASSERT_WITH_TEXT(
        delete_file(SRC_FILE, &error), error);
  }

  NKIT_TEST_CASE(tools_copy_file)
  {
    std::string tmp("0132465798"), etalon;
    _copy_file(tmp);

    tmp = std::string(BUFSIZ-1, 1);
    tmp[BUFSIZ-34] = 2;
    _copy_file(tmp);

    tmp = std::string(BUFSIZ, 1);
    tmp[BUFSIZ-34] = 2;
    _copy_file(tmp);

    tmp = std::string(BUFSIZ+1, 1);
    tmp[BUFSIZ-34] = 2;
    _copy_file(tmp);

    tmp = std::string(2*BUFSIZ, 1);
    tmp[BUFSIZ-34] = 2;
    _copy_file(tmp);

    tmp = std::string(2*BUFSIZ+100000, 1);
    tmp[BUFSIZ-34] = 2;
    _copy_file(tmp);
  }

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

  NKIT_TEST_CASE(tools_ltrim)
  {
    std::string str, etalon("etal \n \t \r on");

    str = " " + etalon;
    NKIT_TEST_ASSERT(ltrim(str, " ") == etalon);

    str = "\n" + etalon;
    NKIT_TEST_ASSERT(ltrim(str, "\n") == etalon);

    str = "   \n" + etalon;
    NKIT_TEST_ASSERT(ltrim(str, " \n") == etalon);

    str = "   \n\t\r" + etalon;
    NKIT_TEST_ASSERT(ltrim(str, " \n\r\t") == etalon);

    str = "   \n\r" + etalon;
    NKIT_TEST_ASSERT(ltrim(str, " \n") != etalon);
  }

  NKIT_TEST_CASE(tools_rtrim)
  {
    std::string str, etalon("etal \n \t \r on");

    str = etalon + " ";
    NKIT_TEST_ASSERT_WITH_TEXT(rtrim(str, " ") == etalon, rtrim(str, " "));

    str = etalon + "\n";
    NKIT_TEST_ASSERT(rtrim(str, "\n") == etalon);

    str = etalon + "   \n";
    NKIT_TEST_ASSERT(rtrim(str, " \n") == etalon);

    str = etalon + "   \n\t\r";
    NKIT_TEST_ASSERT(rtrim(str, " \n\r\t") == etalon);

    str = etalon + "   \n\r";
    NKIT_TEST_ASSERT(rtrim(str, " \n") != etalon);
  }

  NKIT_TEST_CASE(tools_trim)
  {
    std::string str, etalon("etal \n \t \r on");

    str = " " + etalon + " ";
    NKIT_TEST_ASSERT_WITH_TEXT(trim(str, " ") == etalon, trim(str, " "));

    str = "\n" + etalon + "\n";
    NKIT_TEST_ASSERT(trim(str, "\n") == etalon);

    str = "   \n" + etalon + "   \n";
    NKIT_TEST_ASSERT(trim(str, " \n") == etalon);

    str = "   \n\t\r" + etalon + "   \n\t\r";
    NKIT_TEST_ASSERT(trim(str, " \n\r\t") == etalon);

    str = "   \n\r" + etalon + "   \n\r";
    NKIT_TEST_ASSERT(trim(str, " \n") != etalon);
  }

} // namespace nkit_test
