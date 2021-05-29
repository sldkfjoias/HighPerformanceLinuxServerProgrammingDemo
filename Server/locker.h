#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>

// Encapsulation semaphore
class sem
{
public:
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            printf("[ERROR] semaphore init failed!\n");
            throw std::exception();
        }
    }

    ~sem()
    {
        sem_destory(&m_sem);
    }

    bool P()
    {
        return sem_wait(&m_sem) == 0;
    }

    bool V()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};

// Encapsulation mutex
class locker
{
public:
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            printf("[ERROR] locker init failed\n");
            throw std::exception();
        }
    }

    ~locker()
    {
        pthread_mutex_destory(&m_mutex);
    }

    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;
};

// Encapsulation condition
class cond
{
public:
    cond()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            printf("[ERROR] condition init failed\n");
            throw std::exception();
        }
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //release the resource that has been alloc rightaway
            printf("[ERROR] condition init failed, release the mutex rightaway\n");
            pthread_mutex_destory(&m_mutex);
            throw std::exception;
        }
    }

    ~cond()
    {
        pthread_mutex_destory(&m_mutex);
        pthread_mutex_destory(&m_cond);
    }

    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }

    bool post()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

#endif