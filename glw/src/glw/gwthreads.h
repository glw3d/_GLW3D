/**
Author: Mario J. Martin <dominonurbs$gmail.com>

Header for those functions that handle threads.

*******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

    /** Pauses the current thread during the especified miliseconds */
    void gw_sleep( const int miliseconds );

    /** Launches the render thread */
    void gw_launch_thread( void( *f )() );

    /** Locks the render thread */
    void lock_threads();

    /** Unlocks the render thread */
    void unlock_threads();

    /* Locks the threads until all messages are processed. */
    void conditional_wait( const int milliseconds );

    /* Broadcast to all threads that the condition is no longer locked */
    void condition_notify();

#if defined(__cplusplus)
}
#endif
