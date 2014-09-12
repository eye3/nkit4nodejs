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
  _NKIT_TEST_CASE(vx_star_pref_test)
  {
    std::string error;
    std::string xml_path("../../data/commerce.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    std::string mapping_path("../../data/commerce1.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(mapping_path,
            &mapping), "Could not read file '" + mapping_path + "'.");

    TimeMeter tm;
    tm.Start();
    Dynamic var = DynamicFromXml(xml, mapping, &error);
    tm.Stop();
    CINFO(tm.GetTotal());
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    //CINFO(nkit::json_hr << var);

    mapping = "";
    mapping_path = "../../data/commerce2.json";
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(mapping_path,
            &mapping), "Could not read file '" + mapping_path + "'.");

    tm.Clear();
    tm.Start();
    var = DynamicFromXml(xml, mapping, &error);
    tm.Stop();
    CINFO(tm.GetTotal());
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    //CINFO(nkit::json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_lists)
  {
    CINFO(__FILE__);
    std::string xml_path("./data/sample.xml");
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
    std::string xml_path("./data/sample.xml");
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
    std::string xml_path("./data/sample.xml");
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
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    Dynamic fields_mapping = //DLIST("/person" << DDICT("/*" << "string") );
        DLIST("/person" << DDICT(
              "/married/@firstTime" << "boolean" <<
              "/*/photo -> p" << DLIST("/" << "string")
            )
        );

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    CINFO(nkit::json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_objects_with_list)
  {
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    std::string mapping_path("./data/list_of_objects_with_list.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(mapping_path,
            &mapping), "Could not read file '" + mapping_path + "'.");

    std::string error;
    Dynamic var = DynamicFromXml(xml, mapping, &error);
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
    std::string xml_path("./data/sample.xml");
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
//    std::string xml_path("./data/sample.xml");
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
