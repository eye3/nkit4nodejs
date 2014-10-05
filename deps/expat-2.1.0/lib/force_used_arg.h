#ifndef XmlForceUsedArg_INCLUDED
#define XmlForceUsedArg_INCLUDED 1

#ifdef __GNUC__
#define XML_FORCE_USED(v) (void)(v);
#else
#define XML_FORCE_USED(v)
#endif

#endif  // XmlForceUsedArg_INCLUDED
