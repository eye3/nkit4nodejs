#include "nkit/test.h"
#include "nkit/logger_brief.h"
#include "nkit/dynamic/dynamic_builder.h"
#include "nkit/dynamic_xml.h"
#include "nkit/transcode.h"

namespace nkit_test
{
  using namespace nkit;

  //----------------------------------------------------------------------------
  NKIT_TEST_CASE(encodings)
  {
    std::string error;
    std::string xml_path("./data/sample1251.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
      text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/academi_mapping.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
      text_file_to_string(mapping_path, &mapping, &error), error);

    std::string options;
    std::string options_path("./data/options_attrkey.json");
    NKIT_TEST_ASSERT_WITH_TEXT(
      text_file_to_string(options_path, &options, &error), error);

    Dynamic var = DynamicFromXml(xml, options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    std::string etalon_utf8 = var["academy"]["title"].GetConstString();
    std::string str866, str_utf8, str1251;
    const Transcoder * transcoder = Transcoder::Find("cp866");
    NKIT_TEST_ASSERT(transcoder);
    NKIT_TEST_ASSERT(transcoder->FromUtf8(etalon_utf8, &str866));
    NKIT_TEST_ASSERT(transcoder->ToUtf8(str866, &str_utf8));
    NKIT_TEST_EQ(str_utf8, etalon_utf8);

    NKIT_TEST_ASSERT(transcode("cp866", "cp1251", str866, &str1251));
    str_utf8.clear();
    str866.clear();
    NKIT_TEST_ASSERT(transcode("cp1251", "cp866", str1251, &str866));

    NKIT_TEST_ASSERT(transcoder->ToUtf8(str866, &str_utf8));
    NKIT_TEST_EQ(str_utf8, etalon_utf8);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_wrong_xml)
  {
    Dynamic mapping = DLIST(
        "/person" << DLIST("/*/city" << "string"));

    std::string error;
    Dynamic var = DynamicFromXml("xml", mapping, &error);
    NKIT_TEST_ASSERT(!var && !error.empty());
  }

  //---------------------------------------------------------------------------
  _NKIT_TEST_CASE(xml2var_star_pref_test)
  {
    std::string error;
    std::string xml_path("../../data/commerce.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("../../data/commerce1.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    TimeMeter tm;
    tm.Start();
    Dynamic var = DynamicFromXml(xml, mapping, &error);
    tm.Stop();
    CINFO(tm.GetTotal());
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    //CINFO(json_hr << var);

    mapping = "";
    mapping_path = "../../data/commerce2.json";
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    tm.Clear();
    tm.Start();
    var = DynamicFromXml(xml, mapping, &error);
    tm.Stop();
    CINFO(tm.GetTotal());
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    //CINFO(json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_list_of_lists)
  {
    //CINFO(__FILE__);
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping = DLIST(
        "/person" << DLIST("/phone" << "string"));

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("+122233344550" << "+122233344551")
        << DLIST("+122233344553" << "+122233344554"));

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_list)
  {
    //CINFO(__FILE__);
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping = DLIST("/person/phone" << "string");

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           "+122233344550" << "+122233344551"
        << "+122233344553" << "+122233344554");

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_list_of_lists_with_mask)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping = DLIST(
        "/person" << DLIST("/*/city" << "string"));

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    //CINFO(json_hr << var);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
           DLIST("New York" << "Boston")
        << DLIST("Moscow" << "Tula"));

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_list_of_objects_with_mask)
  {
    //CINFO(__FILE__);
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping = //DLIST("/person" << DDICT("/*" << "string") );
        DLIST("/person" << DDICT(
                  "/married/@firstTime" << "boolean" <<
                  "/photos" << DLIST("/*" << "string")
                )
        );

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
   // CINFO(json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_list_of_objects_with_list)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/list_of_objects_with_list.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

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

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_default_values)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/default_values.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
        DDICT("key_for_default_value" << "default_value") <<
        DDICT("key_for_default_value" << "default_value")
        );

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_default_value_after_value)
  {
    std::string error;
    std::string xml_path("./data/xml2var_default_value_after_value.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/xml2var_default_value_after_value.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    CINFO("+++++++++++++++++ " << var);

//    Dynamic etalon = DLIST(
//        DDICT("key_for_default_value" << "default_value") <<
//        DDICT("key_for_default_value" << "default_value")
//        );
//
//    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_without_trim)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/persons_with_star.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    std::string options;
    std::string options_path("./data/options_no_trim.json");
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(options_path, &options, &error), error);

    Dynamic var = DynamicFromXml(xml, options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
        DDICT(
            "name" << "Jack" <<
            "photos" << "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t" <<
            "age" << "33" <<
            "married" << "Yes" <<
            "phone" << "+122233344551" <<
            "birthday" << "Wed, 28 Mar 1979 12:13:14" <<
            "address" << "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t"
            ) <<
        DDICT(
            "name" << "Boris" <<
            "photos" << "\n\t\t\t\n\t\t\t\n\t\t" <<
            "age" << "34" <<
            "married" << "Yes" <<
            "phone" << "+122233344554" <<
            "birthday" << "Mon, 31 Aug 1970 02:03:04" <<
            "address" << "\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t\t\n\t\t"
            )
        );

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_with_trim)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/persons_with_star.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    std::string options;
    std::string options_path("./data/options_trim.json");
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(options_path, &options, &error), error);

    Dynamic var = DynamicFromXml(xml, options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);

    Dynamic etalon = DLIST(
        DDICT(
            "name" << "Jack" <<
            "photos" << "" <<
            "age" << "33" <<
            "married" << "Yes" <<
            "phone" << "+122233344551" <<
            "birthday" << "Wed, 28 Mar 1979 12:13:14" <<
            "address" << ""
            ) <<
        DDICT(
            "name" << "Boris" <<
            "photos" << "" <<
            "age" << "34" <<
            "married" << "Yes" <<
            "phone" << "+122233344554" <<
            "birthday" << "Mon, 31 Aug 1970 02:03:04" <<
            "address" << ""
            )
        );

    NKIT_TEST_EQ(var, etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_multi_mappings)
  {
    std::string error;
    std::string xml_path("./data/sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    std::string mapping_path("./data/multi_mapping.json");
    std::string mapping;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(mapping_path, &mapping, &error), error);

    std::string options;
    std::string options_path("./data/options_trim.json");
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(options_path, &options, &error), error);

    StructXml2VarBuilder<DynamicBuilder>::Ptr builder = StructXml2VarBuilder<
        DynamicBuilder>::Create(options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(builder, error);
    NKIT_TEST_ASSERT_WITH_TEXT(
        builder->Feed(xml.c_str(), xml.length(), true, &error), error);

    Dynamic academy = builder->var("academy mapping");
    Dynamic persons = builder->var("persons mapping");

    Dynamic academy_etalon = DDICT(
              "link" << "http://www.damsdelhi.com/dams.php"
          <<  "title" << "Delhi Academy Of Medical Sciences"
        );

    Dynamic persons_etalon = DLIST(
        DDICT(
            "name" << "Jack" <<
            "photos" << "" <<
            "age" << "33" <<
            "married" << "Yes" <<
            "phone" << "+122233344551" <<
            "birthday" << "Wed, 28 Mar 1979 12:13:14" <<
            "address" << ""
            ) <<
        DDICT(
            "name" << "Boris" <<
            "photos" << "" <<
            "age" << "34" <<
            "married" << "Yes" <<
            "phone" << "+122233344554" <<
            "birthday" << "Mon, 31 Aug 1970 02:03:04" <<
            "address" << ""
            )
        );

    NKIT_TEST_EQ(academy, academy_etalon);
    NKIT_TEST_EQ(persons, persons_etalon);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_attribute_as_key)
  {
    std::string error;
    std::string xml_path("./data/attribute_as_key_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping =
        DLIST("/Record" << DDICT(
                  "/Field -> @name" << "string"
                )
        );

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(var, error);
    NKIT_TEST_EQ(var[size_t(0)]["ARTIST"], Dynamic("Bob Dylan"));
    NKIT_TEST_EQ(var[size_t(1)]["YEAR"], Dynamic(1988));
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_sandbox)
  {
    std::string error;
    std::string xml_path("./data/attribute_as_key_sample.xml");
    std::string xml;
    NKIT_TEST_ASSERT_WITH_TEXT(
        text_file_to_string(xml_path, &xml, &error), error);

    Dynamic mapping =
        DLIST("/Record" << DDICT(
                  "/Field -> @name" << DDICT(
                        "/ -> @name" << DDICT(
                            "/ -> value" << "string"
                          )
                      )
                )
        );

    Dynamic var = DynamicFromXml(xml, mapping, &error);
    CINFO(json_hr << var);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_any)
  {
    std::string error;
    std::string xml = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
"<rss version=\"2.0\">\n"
"  <channel>\n"
"    <title>FOREX (45 пар валют)</title>\n"
"    <link>http://stock.rbc.ru/online/forex.0/intraday/index.rus.shtml</link>\n"
"    <description>FOREX (45 пар валют) :: Realtime</description>\n"
"    <pubDate>Tue, 09 Dec 2014 10:10:18 GMT</pubDate>\n"
"    <language>ru-ru</language>\n"
"    <item>\n"
"      <title>Австралийский доллар/Канадский доллар</title>\n"
"      <link>http://stock.rbc.ru/online/forex.0/intraday/AUD_CAD.rus.shtml</link>\n"
"      <description>AUD/CAD (AUD_CAD); 0.9516 (-0.04%)</description>\n"
"    </item>\n"
"    <item>\n"
"      <title>Австралийский доллар/Гонконгский доллар</title>\n"
"      <link>http://stock.rbc.ru/online/forex.0/intraday/AUD_HKD.rus.shtml</link>\n"
"      <description>AUD/HKD (AUD_HKD); 6.4257 (-0.02%)</description>\n"
"    </item>\n"
"  </channel>\n"
"</rss>";

    Dynamic options = DDICT(
      "trim" << true <<
      "priority" << DLIST("title"
                      << "link"
                      << "description"
                      << "pubDate"
                      << "language") <<
      "attrkey" << "$" <<
      "textkey" << "_" <<
      "rootname" << "rss" <<
      "itemname" << "item" <<
      "encoding" << "UTF-8" <<
      "xmldec" << DDICT(
         "version" << "1.0" <<
         "standalone" << true
        ) <<
      "pretty" << DDICT(
         "indent" << "  " <<
         "newline" << "\n"
        )
    );

    std::string root_name;
    Dynamic data = DynamicFromAnyXml(xml, options, &root_name, &error);
    CINFO(root_name << "\n" << data);

    std::string out;
    NKIT_TEST_ASSERT_WITH_TEXT(Dynamic2XmlConverter::Process(
        options, data, &out, &error), error);
    NKIT_TEST_EQ(xml, out);
  }
  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_any_explicit_array)
  {
    const char * xml = "<?xml version='1.0' encoding='utf-8'?>"
            "\n" "<root><status v='O'/></root>"
            ;

    Dynamic options = DDICT(
      "trim" << true <<
      "attrkey" << "$" <<
      "textkey" << "_" <<
      "explicit_array" << false
    );

    std::string root_name, error;
    Dynamic data = DynamicFromAnyXml(xml, options, &root_name, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data["status"].IsDict());
    NKIT_TEST_EQ(root_name, "root");
    CINFO(root_name << "\n" << data);

    options["explicit_array"] = Dynamic(true);
    data = DynamicFromAnyXml(xml, options, &root_name, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data["status"].IsList());
    NKIT_TEST_EQ(root_name, "root");
    CINFO(root_name << "\n" << data);

    xml = "<?xml version='1.0' encoding='utf-8'?>"
          "\n" "<root><status v='O'/><status v='O'/></root>"
          ;
    data = DynamicFromAnyXml(xml, options, &root_name, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data["status"].IsList());
    NKIT_TEST_EQ(root_name, "root");
    CINFO(root_name << "\n" << data);

    options["explicit_array"] = Dynamic(false);
    data = DynamicFromAnyXml(xml, options, &root_name, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data["status"].IsList());
    NKIT_TEST_EQ(root_name, "root");
    CINFO(root_name << "\n" << data);
  }

  //---------------------------------------------------------------------------
  NKIT_TEST_CASE(xml2var_true_false_variants)
  {
    const char * xml = "<?xml version='1.0' encoding='utf-8'?>"
            "\n" "<root>"
            "\n" "  <item>"
            "\n" "    <boolean>True</boolean>"
            "\n" "    <custom_boolean>Foo</custom_boolean>"
            "\n" "  </item>"
            "\n" "  <item>"
            "\n" "    <boolean>False</boolean>"
            "\n" "    <custom_boolean>Bar</custom_boolean>"
            "\n" "  </item>"
            "\n" "</root>"
            ;

    Dynamic mapping = DLIST("/item" << DDICT(
              "/boolean" << "boolean|True" <<
              "/custom_boolean" << "boolean|Foo"
            ));
    Dynamic mappings = DDICT("main" << mapping);

    Dynamic options = DDICT(
      "trim" << true <<
      "attrkey" << "$" <<
      "textkey" << "_" <<
      "explicit_array" << false <<
      "true_variants" << DLIST("Bar" << "True") <<
      "false_variants" << DLIST("Foo" << "False")
    );

    std::string root_name, error;
    Dynamic data = DynamicFromXml(xml, options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data.IsList());
    NKIT_TEST_EQ(data[(const size_t )0][(const char * const )"boolean"],
            Dynamic(true));
    NKIT_TEST_EQ(data[(const size_t )0][(const char * const )"custom_boolean"],
            Dynamic(false));
    NKIT_TEST_EQ(data[(const size_t )1][(const char * const )"boolean"],
            Dynamic(false));
    NKIT_TEST_EQ(data[(const size_t )1][(const char * const )"custom_boolean"],
            Dynamic(true));

    options = DDICT(
          "trim" << true <<
          "attrkey" << "$" <<
          "textkey" << "_" <<
          "explicit_array" << false
        );

    data = DynamicFromXml(xml, options, mapping, &error);
    NKIT_TEST_ASSERT_WITH_TEXT(data, error);
    NKIT_TEST_ASSERT(data.IsList());
    NKIT_TEST_EQ(data[(const size_t )0][(const char * const )"boolean"],
            Dynamic(true));
    NKIT_TEST_EQ(data[(const size_t )0][(const char * const )"custom_boolean"],
            Dynamic(false));
    NKIT_TEST_EQ(data[(const size_t )1][(const char * const )"boolean"],
            Dynamic(false));
    NKIT_TEST_EQ(data[(const size_t )1][(const char * const )"custom_boolean"],
            Dynamic(false));
  }

} // namespace nkit_test

