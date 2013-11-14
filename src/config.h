#ifndef _BSML_CONFIG_H
#define _BSML_CONFIG_H


#if defined(WIN32)
 
 #define UNUSED

 #if defined(BSML_BUILD_LIBRARY)
  #define BSML_EXPORT __declspec(dllexport)
 #elif !defined(BSML_STATIC_LIBRARY)
  #define BSML_EXPORT __declspec(dllimport)
 #else
  #define BSML_EXPORT
 #endif

#else

 #define UNUSED __attribute__((__unused__))

 #define BSML_EXPORT

#endif


#endif

