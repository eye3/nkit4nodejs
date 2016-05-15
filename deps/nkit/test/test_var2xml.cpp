#include "nkit/logger_brief.h"
#include "nkit/test.h"
#include "nkit/dynamic/dynamic_builder.h"
#include "nkit/dynamic_xml.h"

namespace nkit_test
{
  using namespace nkit;

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(var2xml)
  {
    Dynamic options = DDICT(
         "rootname" << "ROOT"
//      << "itemname" << "item_"
      << "encoding" << "UTF-8"
      //"encoding" << "windows-1251"
      << "xmldec" << DDICT(
               "version" << "1.0" <<
               "standalone" << true
        )
      << "pretty" << DDICT(
               "indent" << "\t" <<
               "newline" << "\n"
        )
      << "attrkey" << "$"
      << "textkey" << "_"
      << "cdata" << DLIST("cdata"
          << "cdata1"
          << "cdata2"
          << "cdata3"
          << "cdata4"
          << "cdata5")
      << "bool_true" << "true"
      << "bool_false" << "false"
      << "float_precision" << 6
    );

    Dynamic data = //DLIST(
          DDICT(
               "$" << DDICT("p1" << "в1&v2\"'" << "p2" << "v2")
            << "_" << "Hello(Привет) world(мир)"
            << "int_число" << DLIST(1)
            << "float" << DLIST(1.1)
            << "true" << DLIST(true)
            << "false" << DLIST(false)
            << "cdata" << DLIST("text < > & \" ']]> | <![CDATA[")
//            << "list" << DLIST(DLIST(1 << 2) << 2 << 3)
            << "dict" << DLIST(DDICT(
                 "$" << DDICT("a1" << "V1" << "a2" << "V2")
              << "int" << DLIST(1)
              << "float" << DLIST(1.1)
              << "sub_string" << DLIST("text < > & \" '")
              << "list" << DLIST(1 << 2 << 3)
              << "_" << "Hello(Привет) world(мир)"
              << "cdata1" << DLIST("<![CDATA[ text < > & \" '")
              << "cdata2" << DLIST("text <![CDATA[ < > & \" ' ]]>")
              << "cdata3" << DLIST("text <![CDATA[ < > & \" ' ]] www>")
              << "cdata4" << DLIST("text <qqq ![CDATA[ < > & \" ' ]] www>")
              << "cdata5" << DLIST("text << > & \" '")
//                 "list" << DLIST(1 << 2 << 3)
//                "_" << "Hello(Привет) world(мир)"
           ))
//      )
    );

    std::string out, error;
    NKIT_TEST_ASSERT_WITH_TEXT(Dynamic2XmlConverter::Process(
        options, data, &out, &error), error);

    std::string root_name;
    Dynamic _data = DynamicFromAnyXml(out, options, &root_name, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(_data, error);
    std::string _out;
    NKIT_TEST_ASSERT_WITH_TEXT(Dynamic2XmlConverter::Process(
        options, data, &_out, &error), error);
    NKIT_TEST_EQ(out, _out);

    out.clear();
    data = Dynamic::List();
    NKIT_TEST_ASSERT_WITH_TEXT(Dynamic2XmlConverter::Process(
        options, data, &out, &error), error);
    std::string etalon("<?xml version=\"1.0\""
        " encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<ROOT></ROOT>");
    NKIT_TEST_EQ(out, etalon);
  }

}  // namespace nkit_test
