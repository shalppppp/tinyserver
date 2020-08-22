#ifndef HEAP_TIMER
#define HEAP_TIMER

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>
#include "../log/log.h"

class heap_timer;

struct client_data
{
    sockaddr_in address;
    int sockfd;
    heap_timer *timer;
};
//util_timer
class heap_timer
{
public:
    heap_timer():expire(5){}
    heap_timer(int delay)
    {
        expire = time(NULL) + delay;
    }

public:
    size_t expire;
    void (*cb_func)(client_data *);
    client_data *user_data;
};

//sort_timer_lst
class time_heap
{
public:
    time_heap():capacity(0), cur_size(0){}

    time_heap(int cap) : capacity(cap), cur_size(0)
    {
        array = new heap_timer *[capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; i++)
        {
            array[i] = nullptr;
        }
    }
    time_heap(heap_timer **init_array, int size, int capacity) : cur_size(size), capacity(capacity)
    {
        if (capacity < size)
        {
            throw std::exception();
        }
        array = new heap_timer *[capacity];
        if (!array)
        {
            throw std::exception();
        }
        for (int i = 0; i < capacity; i++)
        {
            array[i] = nullptr;
        }
        if (size != 0)
        {
            for (int i = 0; i < size; i++)
            {
                array[i] = init_array[i];
            }
            for (int i = (cur_size - 1) / 2; i >= 0; i--)
            {
                percolate_down(i);
            }
        }
    }
    ~time_heap()
    {
        for (int i = 0; i < cur_size; i++)
        {
            delete array[i];
        }
        delete[] array;
    }

public:
    void add_timer(heap_timer *timer);
    void adjust_timer(heap_timer *timer);
    void del_timer(heap_timer *timer);

    heap_timer *top() const
    {
        if (empty())
        {
            return nullptr;
        }
        return array[0];
    }

    void pop_timer();

    void tick();
    bool empty() const { return cur_size == 0; }

private:
    void percolate_down(int hole);
    void resize();

private:
    heap_timer **array;
    int capacity;
    int cur_size;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setnonblocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void addfd(int epollfd, int fd, bool one_shot, int TRIGMode);

    //信号处理函数
    static void sig_handler(int sig);

    //设置信号函数
    void addsig(int sig, void(handler)(int), bool restart = true);

    //定时处理任务，重新定时以不断触发SIGALRM信号
    void timer_handler();

    void show_error(int connfd, const char *info);

public:
    static int *u_pipefd;
    //sort_timer_lst m_timer_lst;
    time_heap m_timer_heap;

    static int u_epollfd;
    int m_TIMESLOT;
};

void cb_func(client_data *user_data);

#endif