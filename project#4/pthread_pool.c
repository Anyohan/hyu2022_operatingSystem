/*
 * Copyright 2022. Heekuck Oh, all rights reserved
 * 이 프로그램은 한양대학교 ERICA 소프트웨어학부 재학생을 위한 교육용으로 제작되었습니다.
 */
#include <stdlib.h>
#include <stdio.h>
#include "pthread_pool.h"
#include <string.h>

/*
 * 풀에 있는 일꾼(일벌) 스레드가 수행할 함수이다.
 * FIFO 대기열에서 기다리고 있는 작업을 하나씩 꺼내서 실행한다.
 */
static void *worker(void *param)
{   
    pthread_pool_t *pool = param;
    task_t task;
    while(pool->running) {
        pthread_mutex_lock(&(pool->mutex));
        while(pool->q_len == 0 && pool->running) { // q가 비었고 running상태이면 wait
            pthread_cond_wait(&(pool->full),&(pool->mutex)); // q에 task 들어오면 깨어남
        }
        if (pool->running == false) { // pool 의 가동이 shutdown 되었다면 , 스레드 lock 을 내놓고 종료 
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(0); // 종료
        }

        // Loop 에서 빠져나왔다면 , task 큐에 작업 이 있으므로 작업을 빼는 deQueue 작업 실시.
        task.function = pool->q[pool->q_front].function;
        task.param = pool->q[pool->q_front].param;
        
        pool->q_front = (pool->q_front + 1) % (pool->q_size); 
        pool->q_len--; 

        pthread_cond_signal(&(pool->empty));
        pthread_mutex_unlock(&(pool->mutex));
        task.function(task.param);
    }
    pthread_exit(0);
}

/*
 * 스레드풀을 초기화한다. 성공하면 POOL_SUCCESS를, 실패하면 POOL_FAIL을 리턴한다.
 * bee_size는 일꾼(일벌) 스레드의 갯수이고, queue_size는 작업 대기열의 크기이다.
 * 대기열의 크기 queue_size가 최소한 일꾼의 수 bee_size보다 크거나 같게 만든다.
 */
int pthread_pool_init(pthread_pool_t *pool, size_t bee_size, size_t queue_size)
{   
    /* 구조체 변수 초기화 */
    pool->running = true;
    pool->bee_size = bee_size;
    pool->q_size = queue_size;
    pool->q_len = 0;
    pool->q_front = 0;
    pool->q_rear = 0;

    pthread_mutex_init(&(pool->mutex), NULL);
    pthread_cond_init(&(pool->empty), NULL);
    pthread_cond_init(&(pool->full), NULL);

    if (queue_size < bee_size) { // queue_size가 bee_size보다 커야 함
        pool->q_size = pool->bee_size;
    }

    if (bee_size <= POOL_MAXBSIZE && queue_size <= POOL_MAXQSIZE) { // MAXSIZE 예외 처리 후, 메모리 할당 및 스레드 생성
        pool->q = (task_t *)malloc(sizeof(task_t) * (pool->q_size));
        pool->bee = (pthread_t *)malloc(sizeof(pthread_t) * (pool->bee_size));
        for(int i = 0; i < bee_size; i++) {
            if(pthread_create(&(pool->bee)[i], NULL, worker, pool) !=  0) {
                printf("thread create fail…");
                return POOL_FAIL;
            }   
        }
        return POOL_SUCCESS;
    }

    else {
        return POOL_FAIL;
    }
}

/*
 * 스레드풀에서 실행시킬 함수와 인자의 주소를 넘겨주며 작업을 요청한다.
 * 스레드풀의 대기열이 꽉 찬 상황에서 flag이 POOL_NOWAIT이면 즉시 POOL_FULL을 리턴한다.
 * POOL_WAIT이면 대기열에 빈 자리가 나올 때까지 기다렸다가 넣고 나온다.
 * 작업 요청이 성공하면 POOL_SUCCESS를 리턴한다.
 */
int pthread_pool_submit(pthread_pool_t *pool, void (*f)(void *p), void *p, int flag)
{
    pthread_mutex_lock(&(pool->mutex));
    // running이 종료되면 스레드 종료
    if (pool->running == false) {
        pthread_mutex_unlock(&(pool->mutex));
        pthread_exit(0);
    }
    
    // task 큐에 빈자리가 있다면
    if (pool->q_len < pool->q_size) {
        // task 큐에 task 를 push 하는 enQueue 작업 실시,
        pool->q[pool->q_rear].function = f;
        pool->q[pool->q_rear].param = p; 
        pool->q_len ++;
        pool->q_rear = (pool->q_rear + 1) % (pool->q_size); // 원형 큐 쿠현.
        pthread_cond_signal(&(pool->full));
    }

    // task 큐에 빈자리가 없다면
    else if (pool->q_len == pool->q_size) {
        if (flag == POOL_NOWAIT) {
            pthread_mutex_unlock(&(pool->mutex));
            return POOL_FULL;
        }
        else if (flag == POOL_WAIT) {
            while ((pool->q_len == pool->q_size) && pool->running) { // q가 비어있는지 다시한번 확인
                pthread_cond_wait(&(pool->empty), &(pool->mutex)); // 빈자리 생길 때까지 기다림
            }
            if (pool->running == false) {
                pthread_mutex_unlock(&(pool->mutex));
                pthread_exit(0);
            }
            //enQueue 
            pool->q[pool->q_rear].function = f; 
            pool->q[pool->q_rear].param = p;
            pool->q_len++;
            pool->q_rear = (pool->q_rear + 1) % (pool->q_size);
            pthread_cond_signal(&(pool->full));
            pthread_mutex_unlock(&(pool->mutex));
        }
    }
    pthread_mutex_unlock(&(pool->mutex));
    return POOL_SUCCESS;
}

/*
 * 모든 일꾼 스레드를 종료하고 스레드풀에 할당된 자원을 모두 제거(반납)한다.
 * 락을 소유한 스레드를 중간에 철회하면 교착상태가 발생할 수 있으므로 주의한다.
 * 부모 스레드는 종료된 일꾼 스레드와 조인한 후에 할당된 메모리를 반납한다.
 * 종료가 완료되면 POOL_SUCCESS를 리턴한다.
 */
int pthread_pool_shutdown(pthread_pool_t *pool)
{   

    pthread_mutex_lock(&(pool->mutex));
    pool->running = false;
    pthread_mutex_unlock(&(pool->mutex));
    
    //조건 변수에서 기다리고 있는 스레드들을 깨운뒤 락을 풀고 종료 될 수 있게 해줌
    pthread_cond_broadcast(&(pool->full));
    pthread_cond_broadcast(&(pool->empty));
    for (int i = 0; i < pool->bee_size; i++) {
        if (pthread_join(pool->bee[i], NULL) != 0) {
            return POOL_FAIL;
        }
    }

    // init 에서 할당한 메모리 해체
    free(pool->q);
    free(pool->bee);
    return POOL_SUCCESS;
}