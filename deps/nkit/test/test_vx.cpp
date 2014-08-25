#include "nkit/test.h"
#include "nkit/logger_brief.h"

#include "nkit/dynamic_xml.h"

namespace nkit_test
{
  using namespace nkit;

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_wrong_xml)
  {
    Dynamic fields_mapping = DLIST(
        "/person" << DLIST("/*/city" << "string"));

    std::string error;
    Dynamic var = DynamicFromXml("xml", fields_mapping, &error);
    NKIT_TEST_ASSERT(!var && !error.empty());
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_lists)
  {
    CINFO(__FILE__);
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = DLIST(
        "/person" << DLIST("/phone" << "string"));

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("+122233344550" << "+122233344551")
        << DLIST("+122233344553" << "+122233344554"));
    CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT(var == etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list)
  {
    CINFO(__FILE__);
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = DLIST("/person/phone" << "string");

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           "+122233344550" << "+122233344551"
        << "+122233344553" << "+122233344554");
    CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT(var == etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_lists_with_mask)
  {
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = DLIST(
        "/person" << DLIST("/*/city" << "string"));

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("New York" << "Boston")
        << DLIST("Moscow" << "Tula"));

    CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT(var == etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_objects_with_mask)
  {
    CINFO(__FILE__);
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = //DLIST("/person" << DDICT("/*" << "string") );
        DLIST("/person" << DDICT(
            "/birthday" << "datetime|Fri, 22 Aug 2014 13:59:06 +0000|%a, %d %b %Y %H:%M:%S %z"
            << "/*" << "string"
            << "/married/@firstTime -> is" << "boolean")
        );

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    CINFO(nkit::json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_objects_with_list)
  {
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = DLIST("/person" <<
        DDICT("/birthday" << "datetime|Fri, 22 Aug 2014 13:59:06 +0000|%a, %d %b %Y %H:%M:%S %z"
            << "/phone -> phones" << DLIST("/" << "string")
            << "/married/@firstTime" << "boolean")
        );

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(

        DDICT("birthday" << Dynamic(1979, 3, 28, 12, 13, 14)
            << "phones" << DLIST("+122233344550" << "+122233344551")
            << "firstTime" << false) <<

        DDICT("birthday" << Dynamic(1970, 8, 31, 2, 3, 4)
            << "phones" << DLIST("+122233344553" << "+122233344554")
            << "firstTime" << true)
        );

    CINFO(nkit::json_hr << var);
    CINFO(nkit::json_hr << etalon);
    NKIT_TEST_ASSERT(var == etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_sandbox)
  {
    std::string xml_path("./data/vx_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = DLIST(
        "/person" << DLIST("/*/city" << "string"));

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    CINFO("!!! " << nkit::json_hr << var);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("New York" << "Boston")
        << DLIST("Moscow" << "Tula"));

    NKIT_TEST_ASSERT(var == etalon);
//    std::string xml_path("./data/vx_sample.xml");
//    std::string xml;
//    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
//        "Could not read file '" + xml_path + "'.");
//
//    Dynamic fields_mapping = DLIST("/person" <<
//        DDICT("/birthday" << "datetime|1970-01-01|%Y-%m-%d"
//            << "/phone -> phones" << DLIST("/" << "string")
//            << "/married/@firstTime" << "boolean")
//        );
//
//    std::string error;
//    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
//    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
//
//    CINFO(nkit::json_hr << var);
  }

} // namespace nkit_test
