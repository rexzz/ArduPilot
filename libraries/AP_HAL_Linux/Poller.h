/*
 * Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include "AP_HAL/utility/RingBuffer.h"
#include "Semaphores.h"

namespace Linux {

class Poller;

class Pollable {
    friend class Poller;
public:
    Pollable(int fd) : _fd(fd) { }
    Pollable() { }

    virtual ~Pollable();

    int get_fd() const { return _fd; }

    /* Called whenever the underlying file descriptor has data to be read. */
    virtual void on_can_read() { }

    /*
     * Called whenever the underlying file descriptor is ready to receive new
     * data, i.e. its buffer is not full.
     */
    virtual void on_can_write() { }

    /*
     * Called when an error occurred and is signaled by the OS - its meaning
     * depends on the file descriptor being used.
     */
    virtual void on_error() { }

    /*
     * Called when the other side closes its end - the exact meaning
     * depends on the file descriptor being used.
     */
    virtual void on_hang_up() { }

protected:
    int _fd = -1;
};

class Poller {
public:
    Poller() : _epfd(epoll_create1(EPOLL_CLOEXEC)) { }
    ~Poller() { close(_epfd); }


    /*
     * Register @p in this poller so calls to poll() will wait for
     * events specified in @events argument.
     */
    bool register_pollable(Pollable *p, uint32_t events);

    /*
     * Unregister @p from this Poller so it doesn't generate any more
     * event. Note that this doesn't destroy @p.
     */
    void unregister_pollable(const Pollable *p);

    /*
     * Wait for events on all Pollable objects registered with
     * register_pollable(). New Pollable objects can be registered at any
     * time, including when a thread is sleeping on a poll() call.
     */
    int poll() const;

private:
    int _epfd;
};

}
