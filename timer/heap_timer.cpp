#include "heap_timer.h"
#include "../http/http_conn.h"
#include "../log/log.h"

#define BUFFER_SIZE 64

class heap_timer;
void time_heap::add_timer(heap_timer* timer){
    if(!timer){
        return;
    }
    std::cout<<"new connect finish31"<<std::endl;

    if(cur_size >= capacity){
        std::cout<<"new connect finish43"<<std::endl;

        resize();
        std::cout<<"new connect finish55"<<std::endl;

    }
    std::cout<<"new connect finish66"<<std::endl;

    int hole = cur_size++;
    int parent = 0;
    for(;hole > 0; hole=parent){
        parent = (hole - 1) / 2;
        if(array[parent]->expire <= timer->expire){
            break;
        }
        array[hole] = array[parent];
    }
    array[hole] = timer;

}
void time_heap::del_timer(heap_timer* timer){
    if(!timer){
        return;
    }
    timer->cb_func = nullptr;
}

void time_heap::pop_timer(){
    if(empty()){
        return;
    }
    if(array[0]){
        delete array[0];
        array[0] = array[--cur_size];
        time_heap::percolate_down(0);
    }
}
void time_heap::tick(){
    if(empty()){
        return;
    }
    heap_timer* tmp = array[0];
    time_t cur = time(nullptr);
    while(!empty()){
        if(!tmp){
            break;
        }
        if(tmp->expire > cur){
            break;
        }
        if(array[0]->cb_func){
            array[0]->cb_func(array[0]->user_data);
        }
        pop_timer();
        tmp = array[0];
    }
    //cout<<"heap timer"<<endl;
}
void time_heap::percolate_down(int hole){
    heap_timer* temp = array[hole];
    int child = 0;
    for(;((hole * 2 + 1) <= (cur_size - 1)); hole=child){
        child = hole*2 + 1;
        if(child < (cur_size - 1) && (array[child+1]->expire < array[child]->expire)){
            child++;
        }
        if(array[child]->expire < temp->expire){
            array[hole] = array[child];
        }
        else{
            break;
        }
    }
    array[hole] = temp;
}

void time_heap::adjust_timer(heap_timer *timer)
{
    if (!timer)
    {
        return;
    }
    heap_timer *tmp = timer;
    int hole = 0;
    for(int i = 0; i < capacity; i++){
        if(tmp == array[i]){
            hole = i;
            break;
        }
    }
    array[hole] = timer;
    time_heap::percolate_down(hole);
}

void time_heap::resize(){
    capacity = capacity==0?1:2 * capacity;
    heap_timer** temp = new heap_timer* [capacity];
    std::cout<<"new connect finish111"<<std::endl;
    
    for(int i = 0; i < capacity;i++){
        temp[i] = nullptr;
    }
    std::cout<<"new connect finish222"<<std::endl;

    if(!temp){
        throw std::exception();
    }
    //capacity = 2 * capacity;
    std::cout<<"new connect finish333"<<std::endl;

    for(int i = 0; i < cur_size; i++){
        temp[i] = array[i];
    }
    std::cout<<"new connect finish444"<<std::endl;

    //delete [] array;
    std::cout<<"new connect finish555"<<std::endl;

    array = temp;
}

void Utils::init(int timeslot)
{
    m_TIMESLOT = timeslot;
}

//对文件描述符设置非阻塞
int Utils::setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

//将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void Utils::addfd(int epollfd, int fd, bool one_shot, int TRIGMode)
{
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else
        event.events = EPOLLIN | EPOLLRDHUP;

    if (one_shot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//信号处理函数
void Utils::sig_handler(int sig)
{
    //为保证函数的可重入性，保留原来的errno
    int save_errno = errno;
    int msg = sig;
    send(u_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

//设置信号函数
void Utils::addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

//定时处理任务，重新定时以不断触发SIGALRM信号
void Utils::timer_handler()
{
    m_timer_heap.tick();
    alarm(m_TIMESLOT);
}

void Utils::show_error(int connfd, const char *info)
{
    send(connfd, info, strlen(info), 0);
    close(connfd);
}

int *Utils::u_pipefd = 0;
int Utils::u_epollfd = 0;

class Utils;
void cb_func(client_data *user_data)
{
    epoll_ctl(Utils::u_epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    http_conn::m_user_count--;
}
