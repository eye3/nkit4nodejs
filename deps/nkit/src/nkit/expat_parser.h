#ifndef VX_EXPAT_PARSER_H
#define VX_EXPAT_PARSER_H

#include "nkit/tools.h"
#include "expat.h"

#include "nkit/transcode.h"

namespace nkit
{
  //----------------------------------------------------------------------------
  template<typename T>
  class ExpatParser
  {
  public:
    ExpatParser() :
        parser_(XML_ParserCreate(NULL))
    {
      Reset();
    }

    bool Feed(const char* chunk, size_t len, bool last, std::string * error)
    {
      bool result = true;
      if (!XML_Parse(parser_, chunk, len, last))
      {
        XML_Error code = XML_GetErrorCode(parser_);
        if (code == XML_ERROR_ABORTED)
          static_cast<T*>(this)->GetCustomError(error);
        else
          *error = "Parse error at (line:"
              + nkit::string_cast(
                  static_cast<uint64_t>(XML_GetCurrentLineNumber(parser_)))
              + ", column:"
              + nkit::string_cast(
                  static_cast<uint64_t>(XML_GetCurrentColumnNumber(parser_)))
              + ") " + XML_ErrorString(code);

        result = false;
      }

      if (last)
        Reset();
      return result;
    }

  protected:
    // dtor is non-virtual because it is protected and will not be
    // used explicitly
    ~ExpatParser()
    {
      XML_ParserFree(parser_);
    }

    void Reset()
    {
      XML_ParserReset(parser_, NULL);
      XML_SetUserData(parser_, this);
      XML_SetElementHandler(parser_, &ExpatParser::OnStartElement,
          &ExpatParser::OnEndElement);
      XML_SetCharacterDataHandler(parser_, &ExpatParser::OnText);
      XML_SetUnknownEncodingHandler(parser_, &ExpatParser::OnUnknownEncoding,
          this);
    }

  private:
    void AbortParsing()
    {
      XML_StopParser(parser_, 0);
    }

    static void OnStartElement(void *data, const char *el, const char **attr)
    {
      T * derived = static_cast<T *>(data);
      if (!derived->OnStartElement(el, attr))
        derived->AbortParsing();
    }

    static void OnEndElement(void *data, const char *el)
    {
      T * derived = static_cast<T *>(data);
      if (!derived->OnEndElement(el))
        derived->AbortParsing();
    }

    static void OnText(void *data, const char *txt, int len)
    {
      T * derived = static_cast<T *>(data);
      if (!derived->OnText(txt, len))
        derived->AbortParsing();
    }

    static int OnUnknownEncoding(void * NKIT_UNUSED(data),
        const XML_Char * name,
        XML_Encoding * info)
    {
#ifdef XML_UNICODE
      char enc_name[0x100], *p = enc_name;
      while( p != enc_name + 0x100 && *name ) *p++ = (char)*name++;
      if( *name )
        return XML_STATUS_ERROR;
      *p = 0;
#else
      const char * enc_name = name;
#endif

      const Transcoder * tr = Transcoder::Find(enc_name);
      if (!tr)
        return XML_STATUS_ERROR;
      tr->FillExpatEncodingInfo(info->map);
      return XML_STATUS_OK;
    }

  private:
    XML_Parser parser_;
  };

} // namespace nkit

#endif // VX_EXPAT_PARSER_H
