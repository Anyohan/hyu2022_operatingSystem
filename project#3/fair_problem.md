- ### 공정한 reader-writer problem

  ****

1. 사용한 mutex lock 변수

   - Reader_lock: reader_cnt 를 접근하기 위한 변수로, reader 스레드끼리 사용한다
   - Fair_lock: reader 와 writer 가 공유하는 변수로, 임계구역에 접근하는 모든 reader 와 writer 들의 접근을 제한한다.
   - Mutex_lock: reader 와 writer 가 공유하는 변수로, reader 스레드나  writer 스레드가 각각의  임계구역에 들어와 스레드의 진행을 방해하는 것을 막기 위해 접근권한을 주는 변수이다 

2. reader 함수

       

    ~~~c
    
    pthread_mutex_lock(&fair_lock);
    pthread_mutex_lock(&reader_lock);
    reader_cnt++;
    if(reader_cnt == 1){
        pthread_mutex_lock(&mutex_lock);
    }
    pthread_mutex_unlock(&reader_lock);
    pthread_mutex_unlock(&fair_lock);
    
    /* reader 출력 진행 */
    
    pthread_mutex_lock(&reader_lock);
    reader_cnt--; 
    if(reader_cnt == 0){
         pthread_mutex_unlock(&mutex_lock);
    }
    pthread_mutex_unlock(&reader_lock);
    ```
    ~~~

    - fair_lock 을 걸고 reader_cnt 변수를 사용해야 하니 reader_lock 까지 걸어줌

    - Reader_cnt 값을 하나 올려주고, 만약 이값이 올라갔다면, reader 스레드가 실행중이니, writer 스레드가 앞지르지 못하게, mutex_lock 을 걸어줌

    - 이후 사용한 reader_lock 과 fair_lock 을 풀고 reader 스레드의 출력을 진행함.

    - 출력 이후 사용한 reader_cnt 변수를 줄여주기 위해 reader_lock 을 걸고 변수값을 하나씩 내림.

    - 만약 writer 바로 이전에 선착순으로 도착한 reader 스레드가 전부 실행 됐다면 reader_cnt 는 0일 것이므로 writer 의 실행을 위해  mutex_lock 을 풀고, 함께 reader_lock 도 풀어줌

3. writer 함수

     

    ```c
    pthread_mutex_lock(&fair_lock); 
    pthread_mutex_lock(&mutex_lock); 
    pthread_mutex_unlock(&fair_lock);
    
    /* writer 출력 실행 */
    
    pthread_mutex_unlock(&mutex_lock);
    ```

    - reader 와 동일하게 스레드 시작시 공용 변수인 fair_lock 을 걸어줌.

    - 이후 reader 와 같은 논리로, writer 가 실행 되는 상황에선 reader 가 방해를 하면 안되니, mutex_lock 을 걸어줌

    - 이후 fair_lock 은 다른 스레드들이 들어와야하니 풀어줌.

    - writer 출력 실행후 , writer 스레드의 할일이 끝났으니 reader 의 실행을 위해 mutex_lock 을 풀어줌.

      

4. 실행결과물에 대한 설명 및 차이점

    - fair_reader_writer problem 의경우 모든 스레드가 선착순으로 실행되며 각각의 스레드는 선행한 스레드를 앞지를수 없으며 reader 와 writer 는 각각의 스레드가 끝나지 않는 이상 중간에 실행될 수 없다
    - 위의 설명과 같이 공정하게 스레드에게 자원을 넘겨줌으로서 reader , writer 선호 알고리즘과는 다르게 특정 스레드가 기아 상태에 빠지는것을 방지할 수 있다.