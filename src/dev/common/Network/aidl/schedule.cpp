#include <Engine.Network/aidl/schedule.h>
#include <Engine.Network/aidl/error.h>

namespace rw {

clock_t sched_clock_usec()
{
    return (clock_t) Time::getSystemUsTime();
}

clock_t sched_usec_to_tu(clock_t us)
{
    return (clock_t)((double)us / (double)_TU);
}

clock_t sched_tu_to_usec(clock_t us)
{
    return (clock_t)(us * _TU);
}

clock_t mesh80211_get_aw_counter(u16 sync_offset)
{
    return (sched_clock_tu() + sync_offset) / AW_PERIOD;
}

int sched_timer_init(sched_timer* timer, sched_cb cb, clock_t after, clock_t interval)
{
    if(!timer)
        return DL_ERR_PARAMETER_NULL;

    if(timer->active)
        return DL_OK;

    timer->prev = NO;
    timer->next = NO;
    timer->cb = cb;
    timer->data = NO;
    timer->active = NO;
    timer->pending = YES;
    timer->at = sched_clock_usec() + after;
    timer->repeat = interval;
    return DL_OK;
}

int sched_io_init(sched_io* io, sched_cb cb, int fd, int events)
{
    if(!io)
        return DL_ERR_PARAMETER_NULL;

    if(io->active)
        return DL_OK;

    io->prev = NO;
    io->next = NO;
    io->cb = cb;
    io->data = NO;
    io->active = NO;
    io->pending = YES;

    io->fd = fd;
    io->events = events;
    return DL_OK;
}

int sched_signal_init(sched_signal* sig, sched_cb cb)
{
    if(!sig || !cb)
        return DL_ERR_PARAMETER_NULL;

    if(sig->active)
        return DL_OK;

    sig->prev = NO;
    sig->next = NO;
    sig->cb = cb;
    sig->data = NO;
    sig->active = NO;
    sig->pending = YES;
    return DL_OK;
}

int sched_timer_start(sched_loop* loop, sched_timer* timer)
{
    if(!loop || !timer)
        return DL_ERR_PARAMETER_NULL;

    if(timer->active)
        return DL_OK;

    if(!loop->timer)
    {
        loop->timer = timer;
        return DL_OK;
    }

    sched_timer* prev = nullptr;
    sched_timer* curr = loop->timer;

    while(curr)
    {
        prev = curr;
        curr = (sched_timer*) curr->next;

        if(prev->at >= timer->at)
            break;
    }

    if(!curr)
    {
        if(!prev)
            return DL_ERR_FAILED;
            
        timer->prev = prev;
        prev->next = timer;
    }
    else
    {
        curr->next = timer;
        timer->prev = curr;
    }

    timer->active = YES;
    timer->pending = NO;
    return DL_OK;
}

int sched_io_start(sched_loop* loop, sched_io* io)
{
    if(!loop || !io)
        return DL_ERR_PARAMETER_NULL;

    if(io->active)
        return DL_OK;

    if(!loop->io)
    {
        loop->io = io;
        return DL_OK;
    }

    sched_io* prev = nullptr;
    sched_io* curr = loop->io;

    while(curr)
    {
        prev = curr;
        curr = (sched_io*) curr->next;

        if(prev->fd >= io->fd)
            break;
    }

    if(!curr)
    {
        EASSERT(prev, "no prev");
        return DL_ERR_FAILED;
    }

    curr->next = io;
    io->prev = curr;

    io->active = YES;
    io->pending = NO;

    return DL_OK;
}

int sched_signal_start(sched_loop* loop, sched_signal* sig)
{
    if(!loop || !sig)
        return DL_ERR_PARAMETER_NULL;

    if(!loop->signal)
    {
        loop->signal = sig;
        return DL_OK;
    }

    sched_signal* prev = nullptr;
    sched_signal* curr = loop->signal;

    while(curr->next)
    {
        prev = curr;
        curr = (sched_signal*) curr->next;
    }

    if(!curr)
    {
        EASSERT(prev, "no prev");
        return DL_ERR_FAILED;
    }

    curr->next = sig;
    sig->prev = curr;
    sig->pending = NO;

    return DL_OK;
}

void sched_watcher_stop(sched_watcher* w)
{
    if(!w)
        return;

    w->active = NO;
    w->pending = NO;

    // eliminates one-time triggers automatically
    if(w->prev)
        w->prev->next = w->next;
}

#define sched_fine_wait_timer(t) do { \
    while(sched_clock_usec() < t->at) { \
        _mm_pause(); \
    } \
} while(false)

int sched_run(sched_loop* loop)
{
    if(!loop)
        return DL_ERR_PARAMETER_NULL;

    if(!loop->timer)
        return DL_OK;

    // phase 1: wait nearest
    sched_fine_wait_timer(loop->timer);
    
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    for(;;)
    {
        clock_t now = sched_clock_usec();

        if(loop->signal)
        {
            for(sched_signal* sig = loop->signal; sig; sig = (sched_signal*) sig->next)
            {
                if(sig->active)
                {
                    sig->cb(loop, sig, now, SCHED_SIGNAL);
                    sig->active = NO;
                }
            }
        }

        for(sched_timer* timer = loop->timer; timer; timer = (sched_timer*) timer->next)
        {
            if(now >= timer->at)
            {
                timer->cb(loop, timer, now, SCHED_TIMEOUT);
                if(expect_true(timer->repeat))
                {
                    timer->at += timer->repeat;
                }
                else
                {
                    sched_watcher_stop(timer);
                }
            }
        }

        if(loop->io)
        {
            fd_set readfds;
            FD_ZERO(&readfds);

            fd_set writefds;
            FD_ZERO(&writefds);

            for(sched_io* io = loop->io; io; io = (sched_io*) io->next)
            {
                if(io->events & SCHED_READ)
                    FD_SET(io->fd, &readfds);
    
                if(io->events & SCHED_WRITE)
                    FD_SET(io->fd, &writefds);
            }

            int ret = select(0, &readfds, &writefds, NO, &tv);
            if(ret > 0)
            {
                for(sched_io* io = loop->io; io; io = (sched_io*) io->next)
                {
                    if(FD_ISSET(io->fd, &readfds))
                        io->cb(loop, io, sched_clock_usec(), SCHED_READ);
                
                    if(FD_ISSET(io->fd, &writefds))
                        io->cb(loop, io, sched_clock_usec(), SCHED_WRITE);
                }
            }
        }
    }

    return DL_OK;
}
}