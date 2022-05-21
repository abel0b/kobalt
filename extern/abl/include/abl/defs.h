#ifndef ABLDEFS__H
#define ABLDEFS__H

// See https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
  #define ABL_HELPER_DLL_IMPORT __declspec(dllimport)
  #define ABL_HELPER_DLL_EXPORT __declspec(dllexport)
  #define ABL_HELPER_DLL_INTERNAL
#else
  #define ABL_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
  #define ABL_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
  #define ABL_HELPER_DLL_INTERNAL __attribute__ ((visibility ("hidden")))
#endif

#ifdef ABL_DLL
  #ifdef ABL_DLL_EXPORTS
    #define ABL_API ABL_HELPER_DLL_EXPORT
  #else
    #define ABL_API ABL_HELPER_DLL_IMPORT
  #endif
  #define ABL_INTERNAL ABL_HELPER_DLL_INTERNAL
#else
  #define ABL_API
  #define ABL_INTERNAL
#endif

#define abl_typecheck(type,x) \
({  type __dummy; \
    typeof(x) __dummy2; \
    (void)(&__dummy == &__dummy2); \
    1; \
})


#endif
