/**
Author: Mario J. Martin <dominonurbs$gmail.com>

The log system differentites between traces, info, warnings, and errors.
All messages can be catched by an external callback with log_set_info_callback()

There are different options
_handle_error_  is used for error messages; shows the line and file
_warning_       is the same as _handle_error_, but all messages are
preceded by the word "warning"; also the line and file.
_log_ & _info_  are mean to provide relevant information to the user.
_trace_         provide not relevant information, but can be useful to
track the code. This information is not shown in the console
unless log_show_trace( true ) is set, but in the log file.
_trail_         is similar to _trace_, but also provides the line and file.
_debug_         is treated independly in a debug.txt file, only if the
compilation flag _DEBUG is actived. It is used to easy
generate data while the program is debugging.

*******************************************************************************/

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#define LOG_MSG_DEBUG    0
#define LOG_MSG_TRACE    1
#define LOG_MSG_INFO     2
#define LOG_MSG_WARNING  3
#define LOG_MSG_ERROR    4


#if defined(__cplusplus)
extern "C" {
#endif

/* Macro used for error messages. */
#define _handle_error_(...) log_handle_error( __LINE__, __FILE__, __VA_ARGS__ );

/* Macro used for warning messages. */
#define _warning_(...) log_handle_warning( __LINE__, __FILE__, __VA_ARGS__ );

/* Macro used to provide information that might be useful to the user. 
* It is shown in the cosole and log.txt */
#define _log_(...)log_info( __LINE__, __FILE__, __VA_ARGS__ );
#define _info_(...)log_info( __LINE__, __FILE__, __VA_ARGS__ );

/* Macro for traces. By default, it is written in log.txt, but not the screen */
#define _trace_(...) log_trace( __LINE__, __FILE__, __VA_ARGS__ );

/* Only shows the line and the file.
* It is used for debugging, to track the code */
#define _trail_(...) log_trace( __LINE__, __FILE__, __VA_ARGS__ );

/* Macro used for debugging messages. Only works if -D_DEBUG flag is activated 
* This is written in debug.txt and the console. */
#define _debug_(...)log_debug( __VA_ARGS__ );

    /* A convenient macro used to check if a malloc or calloc returns null. */
#ifdef  __cplusplus
    /* C++ definition. In practice it is used for malloc(), and calloc() */
#define _check_(FUNCTION) \
    if ( (FUNCTION) == 0){ _handle_error_("Out of memory!"); }
#else 
    /* ANSI C definition. In practice it is used for malloc(), and calloc() */
#define _check_(FUNCTION) \
    if ( (FUNCTION) == ((void*)0)){ _handle_error_("Out of memory!"); }
#endif

/* Callback function pointers to catch the messages */
typedef void( *logFpMsg )
    (const char* msg   /* The info, warning, or error message */
    , const int line    /* The line of code where it has been generated */
    , const char* file  /* The file where the message has been generated */
    , const int level   /* Indicates the type of the message GW_MSG_LEVEL */
    );

/* Sets the callback to handle the messages from an external application. */
void log_set_info_callback( logFpMsg callback );

/* Shows or hides the traces.
* If the log is handled from the callback, it has no effect. */
void log_show_trace( const int value );

/* Called by the _handle_error_ macro. */
void log_handle_error( const int line, const char* file, const char *msg, ... );

/* Called by the _warning_ macro. */
void log_handle_warning(const int line, const char* file, const char *msg, ...);

/* Called by the _info_ macro. */
void log_info( const int line, const char* file, const char *msg, ... );

/* Called by the _trace_ macro. */
void log_trace( const int line, const char* file, const char *msg, ... );

/* Called by the _trail_ macro. */
void log_trail( const int line, const char* file, const char* msg, ... );

/* Called by the _debug_ macro. */
void log_debug( const char *msg, ... );

/* Returns a string with the local time. */
char* getlocaltime();

#if defined(__cplusplus)
}
#endif

#endif 

/**/