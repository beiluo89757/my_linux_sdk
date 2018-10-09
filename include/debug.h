/**
 ******************************************************************************
 * @file    Debug.h
 * @author  lucas
 * @version V1.0.0
 * @date    05-July-2018
 * @brief   This header contains defines, macros, and functions to aid in
 *          debugging the lucas project.
 ******************************************************************************
 */

#ifndef __Debug_h__
#define __Debug_h__

#include "common.h"

#define LUCAS_DEBUG
// ==== LOGGING ====
#ifdef __GNUC__
    //#define SHORT_FILE __FILE__
    #define SHORT_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#else
    #define SHORT_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#endif

#define YesOrNo(x) (x ? "YES" : "NO")

#ifdef LUCAS_DEBUG
#ifndef LUCAS_DISABLE_STDIO
#ifndef NO_OS
    extern pthread_mutex_t stdio_tx_mutex;

    #define custom_log(N, M, ...) do {pthread_mutex_lock( &stdio_tx_mutex );\
                                      printf("[%ld][%s: %s:%4d] " M "\r\n", time(NULL), N, SHORT_FILE, __LINE__, ##__VA_ARGS__);\
                                      pthread_mutex_unlock( &stdio_tx_mutex );}while(0==1)
                    
    #ifndef LUCAS_ASSERT_INFO_DISABLE
        #define debug_print_assert(A,B,C,D,E,F) do {pthread_mutex_lock( &stdio_tx_mutex );\
                                                    printf("[%ld][LUCAS:%s:%s:%4d] **ASSERT** %s""\r\n", time(NULL), D, F, E, (C!=NULL) ? C : "" );\
                                                    pthread_mutex_unlock( &stdio_tx_mutex );}while(0==1)
    #else  // !LUCAS_ASSERT_INFO_ENABLE
        #define debug_print_assert(A,B,C,D,E,F)
    #endif  // LUCAS_ASSERT_INFO_ENABLE

    #ifdef TRACE
        #define custom_log_trace(N) do {pthread_mutex_lock( &stdio_tx_mutex );\
                                        printf("[%s: [TRACE] %s] %s()\r\n", N, SHORT_FILE, __PRETTY_FUNCTION__);\
                                        pthread_mutex_unlock( &stdio_tx_mutex );}while(0==1)
    #else  // !TRACE
        #define custom_log_trace(N)
    #endif // TRACE  
#else // NO_OS  
    #define custom_log(N, M, ...) do {printf("[%s: %s:%4d] " M "\r\n",  N, SHORT_FILE, __LINE__, ##__VA_ARGS__);}while(0==1)


    #ifndef ASSERT_INFO_DISABLE                                       
        #define debug_print_assert(A,B,C,D,E,F) do {printf("[LUCAS:%s:%s:%4d] **ASSERT** %s""\r\n", D, F, E, (C!=NULL) ? C : "" );}while(0==1)
    #else 
        #define debug_print_assert(A,B,C,D,E,F)
    #endif

    #ifdef TRACE
        #define custom_log_trace(N) do {printf("[%s: [TRACE] %s] %s()\r\n", N, SHORT_FILE, __PRETTY_FUNCTION__);}while(0==1)
    #else  // !TRACE
        #define custom_log_trace(N)
    #endif // TRACE  
#endif                                         
#else
    #define custom_log(N, M, ...)

    #define custom_log_trace(N)

    #define debug_print_assert(A,B,C,D,E,F)                                           
#endif   //LUCAS_DISABLE_STDIO                                      
#else // LUCAS_DEBUG = 0
    // IF !LUCAS_DEBUG, make the logs NO-OP
    #define custom_log(N, M, ...)

    #define custom_log_trace(N)

    #define debug_print_assert(A,B,C,D,E,F)
#endif // LUCAS_DEBUG

// ==== PLATFORM TIMEING FUNCTIONS ====
#ifdef TIME_PLATFORM
    #define function_timer_log(M, N, ...) fprintf(stderr, "[FUNCTION TIMER: " N "()] " M "\n", ##__VA_ARGS__)

    #define TIMEPLATFORM( FUNC, FUNC_NAME )                                             \
                do                                                                      \
                {                                                                       \
                    struct timespec startTime;                                          \
                    clock_gettime(CLOCK_MONOTONIC, &startTime);                         \
                    { FUNC; }                                                           \
                    struct timespec endTime;                                            \
                    clock_gettime(CLOCK_MONOTONIC, &endTime);                           \
                    struct timespec timeDiff = TimeDifference( startTime, endTime );    \
                    function_timer_log("%lld us",                                    \
                                        FUNC_NAME,                                   \
                                        ElapsedTimeInMicroseconds( timeDiff ));      \
                }                                                                       \
                while( 1==0 )
#else
    #define function_timer_log(M, N, ...)

    #define TIMEPLATFORM( FUNC, FUNC_NAME )                                             \
                do                                                                      \
                {                                                                       \
                    { FUNC; }                                                           \
                }                                                                       \
                while( 1==0 )
#endif


// ==== BRANCH PREDICTION & EXPRESSION EVALUATION ====
#if( !defined( unlikely ) )
    //#define unlikely( EXPRESSSION )     __builtin_expect( !!(EXPRESSSION), 0 )
   #define unlikely( EXPRESSSION )     !!(EXPRESSSION)
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check
    @abstract   Check that an expression is true (non-zero).
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method.
    
    Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check ) )
    #define check( X )                                                                                  \
        do                                                                                              \
        {                                                                                               \
            if( unlikely( !(X) ) )                                                                      \
            {                                                                                           \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );             \
            }                                                                                           \
                                                                                                        \
        }   while( 1==0 )
#endif
              
//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    check_string
    @abstract   Check that an expression is true (non-zero) with an explanation.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method.
    
    Code inside check() statements is not compiled into production builds.
*/

#if( !defined( check_string ) )
    #define check_string( X, STR )                                                                                  \
        do                                                                                              \
        {                                                                                               \
            if( unlikely( !(X) ) )                                                                      \
            {                                                                                           \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );              \
                // ASSERTION_FAIL_ACTION(); //lucas                                                           \
            }                                                                                           \
                                                                                                        \
        }   while( 1==0 )
#endif              

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require
    @abstract   Requires that an expression evaluate to true.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require ) )
    #define require( X, LABEL )                                                                             \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_string
    @abstract   Requires that an expression evaluate to true with an explanation.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) and a custom explanation string using the default debugging output method then jumps to a label.
*/

#if( !defined( require_string ) )
    #define require_string( X, LABEL, STR )                                                                 \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                  \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_quiet
    @abstract   Requires that an expression evaluate to true.
    @discussion
    
    If expression evalulates to false, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_quiet ) )
    #define require_quiet( X, LABEL )                                                                       \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr ) )
    #define require_noerr( ERR, LABEL )                                                                     \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );        \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_string
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.), and a custom explanation string using the default debugging output method using the 
    default debugging output method then jumps to a label.
*/

#if( !defined( require_noerr_string ) )
    #define require_noerr_string( ERR, LABEL, STR )                                                         \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );         \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_string
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.), and a custom explanation string using the default debugging output method using the 
    default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action_string ) )
    #define require_noerr_action_string( ERR, LABEL, ACTION, STR )                                          \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );         \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_quiet
    @abstract   Require that an error code is noErr (0).
    @discussion
    
    If the error code is non-0, this jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_quiet ) )
    #define require_noerr_quiet( ERR, LABEL )                                                               \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion
    
    If the error code is non-0, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_noerr_action ) )
    #define require_noerr_action( ERR, LABEL, ACTION )                                                      \
        do                                                                                                  \
        {                                                                                                   \
            OSStatus        localErr;                                                                       \
                                                                                                            \
            localErr = (OSStatus)(ERR);                                                                     \
            if( unlikely( localErr != 0 ) )                                                                 \
            {                                                                                               \
                debug_print_assert( localErr, NULL, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );        \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_noerr_action_quiet
    @abstract   Require that an error code is noErr (0) with an action to execute otherwise.
    @discussion
    
    If the error code is non-0, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_noerr_action_quiet ) )
    #define require_noerr_action_quiet( ERR, LABEL, ACTION )                                                \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( (ERR) != 0 ) )                                                                    \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) using the default debugging output method then executes an action and jumps to a label.
*/

#if( !defined( require_action ) )
    #define require_action( X, LABEL, ACTION )                                                              \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, NULL, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                 \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_string
    @abstract   Requires that an expression evaluate to true with an explanation and action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this prints debugging information (actual expression string, file, line number, 
    function name, etc.) and a custom explanation string using the default debugging output method then executes an
    action and jumps to a label.
*/

#if( !defined( require_action_string ) )
    #define require_action_string( X, LABEL, ACTION, STR )                                                  \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                debug_print_assert( 0, #X, STR, SHORT_FILE, __LINE__, __PRETTY_FUNCTION__ );                  \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    require_action_quiet
    @abstract   Requires that an expression evaluate to true with an action to execute otherwise.
    @discussion
    
    If expression evalulates to false, this executes an action and jumps to a label. No debugging information is printed.
*/

#if( !defined( require_action_quiet ) )
    #define require_action_quiet( X, LABEL, ACTION )                                                        \
        do                                                                                                  \
        {                                                                                                   \
            if( unlikely( !(X) ) )                                                                          \
            {                                                                                               \
                { ACTION; }                                                                                 \
                goto LABEL;                                                                                 \
            }                                                                                               \
                                                                                                            \
        }   while( 1==0 )
#endif



#endif // __Debug_h__

