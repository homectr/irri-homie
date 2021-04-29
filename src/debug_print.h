#pragma once

#if !defined(NODEBUG_PRINT) && defined(DEBUG_PORT)

#define DEBUG_PRINT(...) DEBUG_PORT.printf( __VA_ARGS__ )
#define DEBUG_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {DEBUG_PORT.printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)DEBUG_PORT.printf("\n");}
#define DEBUG_ARRAY_F(ARR,ARR_L,ARR_F) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {DEBUG_PORT.printf(ARR_F,*(ARR+_aidx)); if (_aidx%20 == 19)DEBUG_PORT.printf("\n");}

#if defined(PGMSPACE_INCLUDE) || defined(PROGMEM)
#define DEBUG_PRINT_PGM(...) DEBUG_PORT.printf_P( __VA_ARGS__ )
#else 
#define DEBUG_PRINT_PGM(...)
#endif

#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINT_PGM(...)
#define DEBUG_ARRAY(...)
#define DEBUG_ARRAY_F(...)

#endif

//--

#if defined(CONSOLE_PORT)

#define CONSOLE(...) CONSOLE_PORT.printf( __VA_ARGS__ )
#define CONSOLE_ARRAY(ARR,ARR_L) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {CONSOLE_PORT.printf("%02X ",*(ARR+_aidx)); if (_aidx%20 == 19)CONSOLE_PORT.printf("\n");}
#define CONSOLE_ARRAY_F(ARR,ARR_L,ARR_F) for (uint16_t _aidx =0; _aidx<ARR_L;_aidx++) {CONSOLE_PORT.printf(ARR_F,*(ARR+_aidx)); if (_aidx%20 == 19)CONSOLE_PORT.printf("\n");}

#if defined(PGMSPACE_INCLUDE) || defined(PROGMEM)
#define CONSOLE_PGM(...) CONSOLE_PORT.printf_P( __VA_ARGS__ )
#else
#define CONSOLE_PGM(...)
#endif

#else

#define CONSOLE(...)
#define CONSOLE_ARRAY(...)
#define CONSOLE_ARRAY_F(...)
#define CONSOLE_PGM

#endif