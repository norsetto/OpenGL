#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <functional>

#define STOPPED 0
#define STARTED 1

class ThreadPool
{
 private:
  struct job
  {
    std::function<void()> call;
  };

 public:
  
  ThreadPool();
  ThreadPool(unsigned int pool_size);
  ~ThreadPool();

  bool initialize();
  template<typename F, typename... Args>
    bool add(F func, Args... args);
  bool free();

 private:
  void worker();
  unsigned int pool_size;
  volatile unsigned int pool_state = STOPPED;
  std::thread *worker_thread;
  std::deque<job> job_queue;
  std::mutex task_lock;
  std::condition_variable new_job_added_or_stopped;
};

template<typename F, typename... Args>
bool ThreadPool::add(F func, Args... args)
{
  std::unique_lock<std::mutex> lock(task_lock);

  job task = { std::bind(func, args...) };
  job_queue.push_back(task);
  new_job_added_or_stopped.notify_one();

  return true;
}

ThreadPool::ThreadPool(): pool_size(std::thread::hardware_concurrency()) 
{
}

ThreadPool::ThreadPool(unsigned int pool_size) : pool_size(pool_size)
{
}

ThreadPool::~ThreadPool()
{
  if (pool_state != STOPPED) {
    ThreadPool::free();
  }
}

bool ThreadPool::initialize()
{
  pool_state = STARTED;
  worker_thread = new std::thread [pool_size];
  
  for (unsigned int i = 0; i < pool_size; i++)
    {
      worker_thread[i] = std::thread(&ThreadPool::worker, this);
    }

  return true;
}

bool ThreadPool::free()
{
  std::unique_lock<std::mutex> lock(task_lock);
  
  pool_state = STOPPED;
  new_job_added_or_stopped.notify_all();
  lock.unlock();
  
  for (unsigned int i = 0; i < pool_size; i++)
    {
      if(worker_thread[i].joinable())
	{
	  worker_thread[i].join();
	}
    }

  delete [] worker_thread;
  
  job_queue.clear();
  
  return true;
}

void ThreadPool::worker()
{
  job task;
  
  while(true) {

    std::unique_lock<std::mutex> lock(task_lock);
    
    new_job_added_or_stopped.wait(lock,
				  [this](){return
				      (pool_state == STOPPED) ||
				      (!job_queue.empty());});
    if (pool_state == STOPPED)
      {
	return void();
      }

    task = job_queue.front();
    job_queue.pop_front();

    task.call();
  }
  
}

