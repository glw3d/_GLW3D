/***
Author: Mario J. Martin <dominonurbs$gmail.com>

All thread management is implemented here.
Some compilers has not yet implemented C++11 thread standard.
MinGW is one of them, and there is also the posix issue (I don't know exactly).
Use boost/threads when <threads.h> is not available
In that case, define the compiler variable -DUSE_BOOST_THREADS

*******************************************************************************/

/* Standard C++11 threads are not supported by VS2010 */
#ifdef _MSC_VER
#if (_MSC_VER <= 1600) 
#define USE_BOOST_THREADS
#endif
#endif   

#ifdef USE_BOOST_THREADS 
#include "boost/thread/thread.hpp"
#define STD boost
#else
#include <thread>
#include <mutex>
#include <condition_variable>
#define STD std
#endif

/* A mutex to blocks all other threads.
* Usually to avoid conflicts with other threads while handling data. */
static STD::mutex _mutex;
bool _mutex_is_locked = false;

/* The thread where rendering and OpenGL operation are executed when it is 
 * detached from the main thread */
STD::thread* render_thread_loop = nullptr;

/* Semafore that forces the main thread to wait until a message is processed
* in the render thread. */
static STD::condition_variable _condition;
int _condition_is_locked = 0;

extern "C"
void gw_sleep( const int miliseconds )
{
    STD::this_thread::sleep_for( STD::chrono::milliseconds( miliseconds ) );
}

extern "C"
void gw_launch_thread( void( *f )() )
{
    render_thread_loop = new STD::thread( f );
    //render_thread_loop->join();
}

/* Locks the render thread */
extern "C"
void lock_threads()
{
    _mutex.lock();
    _mutex_is_locked = true;
}

/* Unlocks the render thread */
extern "C"
void unlock_threads()
{
    if (_mutex_is_locked == true){
        _mutex_is_locked = false;
        _mutex.unlock();
    }
}

/* Locks the threads until all messages are processed. */
extern "C"
void conditional_wait( const int milliseconds )
{
    if (render_thread_loop != nullptr
        && render_thread_loop->get_id() != STD::this_thread::get_id())
    {
        STD::unique_lock< STD::mutex > lock( _mutex );
        _condition_is_locked++;
        if (milliseconds > 0){
            /* Wait until the message is processed up to a limit of miliseconds */
            _condition.wait_for
                ( lock, STD::chrono::milliseconds( milliseconds ) );
        }
        else{
            /* Process the message or die */
            _condition.wait( lock );
        }
    }
}

/* Broadcast to all threads that the condition is no longer locked */
extern "C"
void condition_notify()
{
    STD::unique_lock< STD::mutex > lock( _mutex );

    if (_condition_is_locked > 0){
        _condition_is_locked--;
    }
    if (_condition_is_locked <= 0){
        _condition.notify_all();
    }
}
