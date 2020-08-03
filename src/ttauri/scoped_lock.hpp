// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <mutex>
#include <optional>
#include <tuple>

namespace tt {

template<typename T> class scoped_unlock;

template<typename MutexType>
class scoped_lock {
public:
    using mutex_type = MutexType;

private:
    mutex_type &mutex;

    void lock() {
        mutex.lock();
    }

    void unlock() {
        mutex.unlock();
    }

public:
    explicit scoped_lock(MutexType &mutex) :
        mutex(mutex)
    {
        lock();
    }

    ~scoped_lock()
    {
        unlock();
    }

    scoped_lock(scoped_lock const &) = delete;
    scoped_lock &operator=(scoped_lock const &) = delete;

    friend class scoped_unlock<MutexType>;
};

template<typename MutexType>
class scoped_unlock {
public:
    using lock_type = scoped_lock<MutexType>;

private:
    lock_type &slock;

    void lock() {
        slock.lock();
    }

    void unlock() {
        slock.unlock();
    }

public:
    explicit scoped_unlock(scoped_lock<MutexType> &slock) :
        slock(slock)
    {
        unlock();
    }

    ~scoped_unlock() {
        lock();
    }

    scoped_unlock(scoped_unlock const &) = delete;
    scoped_unlock &operator=(scoped_unlock const &) = delete;

};


}