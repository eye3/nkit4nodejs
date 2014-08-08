#include "nkit/test.h"
#include "nkit/logger_brief.h"

#include "nkit/dynamic_xml.h"

namespace nkit_test
{
  using namespace nkit;

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_lists)
  {
    CINFO(__FILE__);
    std::string xml_path("./data/vx_sample.xml");
    std::string spec_file = "./data/list_of_lists.json";
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    std::string fields_mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        nkit::text_file_to_string(spec_file, &fields_mapping),
        "Could not read file '" + spec_file + "'.");

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("+122233344550" << "+122233344551")
        << DLIST("+122233344553" << "+122233344554"));
    //CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT(var == etalon);
  }

#ifndef NKIT_WINNT
  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(vx_list_of_objects_with_list)
  {
    std::string xml_path("./data/vx_sample.xml");
    std::string spec_file = "./data/list_of_objects_with_list.json";
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(nkit::text_file_to_string(xml_path, &xml),
        "Could not read file '" + xml_path + "'.");

    std::string fields_mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        nkit::text_file_to_string(spec_file, &fields_mapping),
        "Could not read file '" + spec_file + "'.");

    std::string error;
    Dynamic var = DynamicFromXml(xml, fields_mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(

        DDICT("birthday" << Dynamic(1980, 2, 28, 0, 0, 0)
            << "phones" << DLIST("+122233344550" << "+122233344551")
            << "isMerriedFirstTime" << false) <<

        DDICT("birthday" << Dynamic(1979, 5, 16, 0, 0, 0)
            << "phones" << DLIST("+122233344553" << "+122233344554")
            << "isMerriedFirstTime" << true)

        );
    //CINFO(nkit::json_hr << var);
    NKIT_TEST_ASSERT(var == etalon);
  }
#endif

} // namespace nkit_test
