#ifndef THREADPOOL_H
#defind THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"
#include <memory>

/*
关于worker这个函数的作用做下记录：
    pthread_create()的第三个参数必须是一个静态函数。但这里是一个模板类型T，为了在一个静态函数中使用类的动态成员，只能通过以下两种方式实现：
        1. 通过类的静态对象来调用。
        2. 将类的对象作为参数传给该静态函数，然后再静态函数中引用这个对象，并调用其动态方法
    然后这个代码使用的就是第二种方式：将线程参数设置为this指针，然后再worker中获取该指针并调用其动态方法run()
    然后这里面调用worker的方式就是：
    worker(this) -> this -> run() -> return pool
    这个return没看出来有什么意义
*/

template <typename T>
class threadpool
{
public:
    //thread_number is the num of thread in the threadpool
    //max_requests is the max num of the thread allowed waiting to process
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    //push requests in the requests list
    bool append(T *requests);

private:
    //the function thread to call
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;        //thread number in the pool
    int m_max_requests;         //max request number allowed in the list
    pthread_t *m_threads;       //the list of the thread
    std::list<T *> m_workqueue; //requests list
    locker m_queuelocker;       //locker to protect the requests list
    sem m_queuestat;            //whether there is a work to do
    bool m_stop;                //whether to stop the thread
};

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(nullptr)
{
    if ((thread_number <= 0) || max_requests <= 0)
    {
        printf("[ERROR] thread number is %d, max requests is %d, while both need bigger than 0\n", thread_number, max_requests);
        throw std::exception;
    }
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
    {
        printf("[ERROR] new phread_t[m_thread_number] failed\n");
        throw std::exception();
    }

    //create thread_number thread, and set them detached
    for (int i = 0; i < thread_number; ++i)
    {
        printf("[LOG] create the %dth thread\n", i);
        if (pthread_create(m_threads + i, nullptr, worker, this) != 0)
        {
            printf("[ERROR] create the %dth thread failed\n", i);
            delete[] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]))
        {
            printf("[ERROR] make the %dth thread detached failed\n", i);
            delete[] m_threads;
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

template <typename T>
bool threadpool<T>::append(T *request)
{
    //make sure the locker in locked when operate the request list
    m_queuelocker.lock();
    //the number of requests is overflow
    if (m_workqueue.size() > m_max_requests)
    {
        printf("[LOG] the number is requests is at max level\n");
        m_queuelocker.unlock();
        return false;
    }

    m_workqueue.push_back(request);
    m_queuelocker.unlock;
    m_queuestat.post(); // there is request to process
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg)
{
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            m_queuelocker.unlock();
            continue;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request)
        {
            continue;
        }
        request->process();
    }
}

#endif