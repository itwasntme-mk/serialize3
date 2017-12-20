///\vpi_api.h - example
#pragma once

#if defined(_MSC_VER)
  #pragma warning (disable : 4996)
#endif

#if defined(__GNUC__) && defined(__unix__)
  #define VPI_API __attribute__((__visibility__("default")))
  #define VPI_API_UNIX_ONLY __attribute__((__visibility__("default")))
  #define VPIRUNTIME_API VPI_API 
#else
  #define VPI_API_UNIX_ONLY

#if defined (__TINYC__)
  #ifdef VPI_IS_DLL
    #define VPI_API __attribute__((dllexport))
  #else
    #define VPI_API __attribute__((dllimport))
  #endif
#else
  #ifdef VPI_IS_DLL
    #define VPI_API __declspec(dllexport)
  #else
    #define VPI_API __declspec(dllimport)
  #endif
#endif
#endif
