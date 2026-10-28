#pragma once
#include "frame.h"

/* event types */
#define SCHED_READ 1
#define SCHED_WRITE 2
#define SCHED_TIMEOUT 4
#define SCHED_SIGNAL 8

namespace rw
{
// Returns the time in microseconds.
AIDL_API clock_t sched_clock_usec();

// Returns the time in time units.
inline clock_t sched_clock_tu()
    { return (clock_t)((double)sched_clock_usec() / (double)_TU); }

// Returns the timestamp in TUs.
AIDL_API clock_t sched_usec_to_tu(clock_t us);

// Returns the TUs converted back to microseconds.
AIDL_API clock_t sched_tu_to_usec(clock_t TU);

// Returns the current AW from time shifted by _sync_offset_ (TU).
AIDL_API clock_t mesh80211_get_aw_counter(u16 sync_offset = NO);

typedef struct sched_watcher;
typedef struct sched_io;
typedef struct sched_timer;
typedef struct sched_signal;
typedef struct sched_loop;

typedef void(*sched_cb)(sched_loop*, sched_watcher*, clock_t, int);

struct sched_watcher {
    sched_watcher* prev;
    sched_watcher* next;
    sched_cb cb;
    void* data;
    int active;
    int pending;
};

// watches a socket
struct sched_io : sched_watcher {
    int fd;
    int events;
};

struct sched_timer : sched_watcher {
    clock_t at;
    clock_t repeat;
};

struct sched_signal : sched_watcher {
};

struct sched_loop {
    sched_timer* timer;
    sched_io* io;
    sched_signal* signal;
};

AIDL_API int sched_io_init(sched_io*, sched_cb cb, int fd, int events);
AIDL_API int sched_timer_init(sched_timer*, sched_cb cb, clock_t after, clock_t interval);
AIDL_API int sched_signal_init(sched_signal*, sched_cb cb);

AIDL_API int sched_timer_start(sched_loop*, sched_timer*);
AIDL_API int sched_io_start(sched_loop*, sched_io*);
AIDL_API int sched_signal_start(sched_loop* loop, sched_signal* sig);

AIDL_API void sched_watcher_stop(sched_watcher*);

AIDL_API int sched_run(sched_loop*);
}