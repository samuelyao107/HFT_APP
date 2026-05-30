#pragma once

#include <iostream>
#include <thread>
#include <atomic>
#include <unistd.h>
#include <sys/syscall.h>
#include <future>

inline auto setThreadCore(int core_id) noexcept {
    cpu_set_t cpuset; //[01010101] 8 cores
    CPU_ZERO(&cpuset); //[00000000] clear all cores
    CPU_SET(core_id, &cpuset);//[00000010] set the core_id core

    return pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0;
}

template<typename T, typename... A>
inline auto createAndRunThread(int core_id, 
    const std::string &name, T &&func, A &&...args) noexcept{

    std::promise<bool> startup_promise;
    std::future<bool> startup_future = startup_promise.get_future();

    auto thread_func = [core_id, name, 
                        p = std::move(startup_promise), 
                        f = std::forward<T>(func), 
                        args_tuple = std::make_tuple(std::forward<A>(args)...)] ()mutable {
       if(core_id >=0 && !setThreadCore(core_id)){
           std::cerr << "Failed to set thread affinity for thread "
            << name <<" "<< pthread_self() << " " << " to core " << core_id << std::endl;
           p.set_value(false);
           return;
       }
       std::cout << "Thread " << name <<" "<< 
       pthread_self() << " is running on core " << core_id << std::endl;
       p.set_value(true);

       std::apply(f, std::move(args_tuple));
      
    };

   std::thread t(std::move(thread_func));

   bool success = startup_future.get();//it's blocking, we use it cause we need to know if the thread started successfully before we can return it
   if(!success){
      if(t.joinable()){
          t.join();
      }
      return std::thread{};
   }


    return t;
  
}

