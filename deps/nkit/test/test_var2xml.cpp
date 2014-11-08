#include <stack>

#include "nkit/logger_brief.h"
#include "nkit/test.h"
#include "nkit/var2xml.h"

namespace nkit_test
{
  using namespace nkit;

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(var2xml)
  {
    Dynamic options = DDICT(
         "rootname" << "ROOT"
      << "itemname" << "item"
      << "xmldec" << DDICT(
               "version" << "1.0"
               //<< "encoding" << "UTF-8"
               << "encoding" << "windows-1251"
               << "standalone" << true
         )
      << "pretty" << DDICT(
               "indent" << "  "
            << "newline" << "\n"
         )
      << "attrkey" << "$"
      << "charkey" << "_"
    );

    Dynamic data = DDICT(
         "$" << DDICT("p1" << "в1&v2\"'" << "p2" << "v2")
      << "_" << "Hello(Привет) world(мир)"
      << "int(число)" << 1
      << "float" << 1.1
      << "string" << "text < > & \" '"
      << "list" << DLIST(DLIST(1) << 2 << 3)
      << "dict" << DDICT(
               "$" << DDICT("a1" << "V1" << "a2" << "V2")
            << "int" << 1
            << "float" << 1.1
            << "string" << "text"
            << "list" << DLIST(1 << 2 << 3)
         )
    );

    std::string out, error;
    NKIT_TEST_ASSERT_WITH_TEXT(Var2XmlConverter::Run(
        options, data, &out, &error), error);
    CINFO(out);

    out.clear();
    data = DLIST(DLIST(1) << 2 << 3);
    NKIT_TEST_ASSERT_WITH_TEXT(Var2XmlConverter::Run(
        options, data, &out, &error), error);
    CINFO(out);
  }

}  // namespace nkit_test
