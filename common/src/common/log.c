/**
Author: Mario J. Martin <dominonurbs$gmail.com>

*******************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "definitions.h"
#include "log.h"

/* Set to 0 or 1 with gw_show_trace() to show the trace messages */
static int show_trace_msg_flag = 0;

static logFpMsg f_info_callback = nullptr;

static int clear_trace_debug = 1;
static int trace_debug_is_bussy = 0;

static int clear_trace_log = 1;
static int trace_log_is_bussy = 0;

/* Sets the callback for the log system.
* If it is set to no-null, messages are handled from outside. */
void log_set_info_callback( logFpMsg callback )
{
    f_info_callback = callback;
}

/* Activates or deactivates the traces */
void log_show_trace( const int value )
{
    show_trace_msg_flag = value;
}

static FILE* open_log_file()
{
    FILE* fp = NULL;
    time_t rawtime;
    struct tm * timeinfo;

    if (clear_trace_debug == 1){
        /* This is the first time the file is open */
        clear_trace_debug = 0;
        fp = fopen( "log.txt", "wt" );
        time( &rawtime );
        timeinfo = localtime( &rawtime );
        if (fp != NULL){
            fprintf( fp, "Time: %d/%d/%d %d:%d:%d\n"
                , timeinfo->tm_mday
                , timeinfo->tm_mon
                , timeinfo->tm_year
                , timeinfo->tm_hour
                , timeinfo->tm_min
                , timeinfo->tm_sec
                );
            fprintf( fp, "--------------------------------\n" );
        }
    }
    else{
        fp = fopen( "log.txt", "at" );
    }

    return fp;
}

/* Handle the error messages. The message stars with !!! 
 * and shows the file and line where the message has been generated */
void log_handle_error( const int line, const char* file, const char* msg, ... )
{
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    if (f_info_callback != nullptr){
        f_info_callback( msg, line, file, LOG_MSG_ERROR );
    }
    else{
        fprintf( stdout, "!!! " );
        vfprintf( stdout, msg, argptr );
        fprintf( stdout, "   line: %i   file: %s", line, file );
        fprintf( stdout, "\n" );

        if (trace_log_is_bussy == 0)
        {
            trace_log_is_bussy = 1;
            fp = open_log_file();
            if (fp != NULL){
                fprintf( fp, "!!! " );
                vfprintf( fp, msg, argptr );
                fprintf( fp, "   line: %i   file: %s", line, file );
                fprintf( fp, "\n" );

                fclose( fp );
            }
            trace_log_is_bussy = 0;
        }
    }
}

/* Handle the warning messages. The message stars with Warning!
* and shows the file and line where the message has been generated */
void log_handle_warning( const int line, const char* file, const char* msg, ... )
{
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    if (f_info_callback != nullptr){
        f_info_callback( msg, line, file, LOG_MSG_WARNING );
    }
    else{
        fprintf( stdout, "Warning! " );
        vfprintf( stdout, msg, argptr );
        fprintf( stdout, "   line: %i   file: %s", line, file );
        fprintf( stdout, "\n" );

        if (trace_log_is_bussy != 0)
        {
            trace_log_is_bussy = 1;
            fp = open_log_file();
            if (fp != NULL){
                fprintf( fp, "Warning! " );
                vfprintf( fp, msg, argptr );
                fprintf( fp, "   line: %i   file: %s", line, file );
                fprintf( fp, "\n" );

                fclose( fp );
            }
            trace_log_is_bussy = 0;
        }
    }
}

/* Shows a trace message, that usually is not relevant information for the user
* unless something goes wrong. By default is not shown on the console unless
* log_show_trace( 1 ); is called. But it is always written in log.txt */
void log_trace( const int line, const char* file, const char* msg, ... )
{
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    if (f_info_callback != nullptr){
        f_info_callback( msg, line, file, LOG_MSG_TRACE );
    }
    else{
        if (show_trace_msg_flag){
            vfprintf( stdout, msg, argptr );
        }
        if (trace_log_is_bussy == 0)
        {
            trace_log_is_bussy = 1;
            fp = open_log_file();
            if (fp != NULL){
                vfprintf( fp, msg, argptr );
                fclose( fp );
            }
            trace_log_is_bussy = 0;
        }
#ifdef _DEBUG
        vfprintf( stdout, msg, argptr );
#endif
    }
}

/* Generates relevant information to the user.
 * This is written in the console and log.txt as well. */ 
void log_info( const int line, const char* file, const char* msg, ... )
{
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    if (f_info_callback != nullptr){
        f_info_callback( msg, line, file, LOG_MSG_INFO );
    }
    else{
        vfprintf( stdout, msg, argptr );

        if (trace_log_is_bussy == 0)
        {
            trace_log_is_bussy = 1;
            fp = open_log_file();
            if (fp != NULL){
                vfprintf( fp, msg, argptr );
                fclose( fp );
            }
            trace_log_is_bussy = 0;
        }
    }
}

/* Shows a trace message with the line, the file and optionally a message
* It is used for debugging to track the code in debug mode. */
void log_trail( const int line, const char* file, const char* msg, ... )
{
#if _DEBUG
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    if (f_info_callback != nullptr){
        f_info_callback( msg, line, file, LOG_MSG_DEBUG );
    }
    else{
        vfprintf( stdout, msg, argptr );
        fprintf( stdout, "   line: %i   file: %s", line, file );
        fprintf( stdout, "\n" );

        if (trace_debug_is_bussy == 0)
        {
            trace_debug_is_bussy = 1;
            if (clear_trace_debug == 1){
                /* This is the first time the file is open */
                clear_trace_debug = 0;
                fp = fopen( "debug.txt", "wt" );
            }
            else{
                fp = fopen( "debug.txt", "at" );
            }
            vfprintf( fp, msg, argptr );
            fprintf( fp, "   line: %i   file: %s", line, file );
            fprintf( fp, "\n" );
            fclose( fp );
            trace_debug_is_bussy = 0;
        }
    }
#endif
}

/* Generates information only fo debugging. It is written in the console and
* debug.txt file. This shows noting in release compilation mode.
* It is not captured by the external handler. */
void log_debug( const char* msg, ... )
{
#if _DEBUG
    FILE* fp = NULL;
    va_list argptr;
    va_start( argptr, msg );

    if (msg == nullptr){
        return;
    }

    vfprintf( stdout, msg, argptr );

    if (trace_debug_is_bussy == 0)
    {
        trace_debug_is_bussy = 1;
        if (clear_trace_debug == 1){
            /* This is the first time the file is open */
            clear_trace_debug = 0;
            fp = fopen( "debug.txt", "wt" );
        }
        else{
            fp = fopen( "debug.txt", "at" );
        }
        vfprintf( fp, msg, argptr );
        fclose( fp );
        trace_debug_is_bussy = 0;
    }
#endif
}

/* Provides the local time in an string */
char* getlocaltime()
{
    time_t rawtime;
    struct tm * timeinfo;

    /* gets time in seconds */
    time( &rawtime );

    /* converts into tm format */
    timeinfo = localtime( &rawtime );

    /* gets in string format */
    return asctime( timeinfo );
}