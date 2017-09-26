#pragma once

#include <chrono>
#include <map>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <memory>
#include "AsyncExecutor.h"
#include "Exception.h"
#include "Thread.h"

namespace core
{
	//TimedAsyncExecutor provides the mean to schedule tasks at a given time.
	//the executor provides the capability to attach a time stamp with each
	//newly received task, making sure the task will be schedule for execution
	//at best possible future time. ensuring the execution of received task time t1 will happen
	//in time t2 which will hold the following t2>=t1 while trying to make the delta between the two
	//the smallest it can.
	class TimedAsyncExecutor : public AsyncExecutor
	{
	public:
		explicit TimedAsyncExecutor(int threadPoolSize = 1);
		virtual ~TimedAsyncExecutor();
		//Will spawn the received task with a "now" time stamp.
		void SpawnTask(AsyncTask* task) override;
		//SpawnTimedTask receives a new commited task and its due time.
		//the function will verify if the received time is the next min time
		//requiring the attention of the task publishing part of the execturor.
		//if yes - the publishing part will be notified.
		void SpawnTimedTask(AsyncTask* task, std::chrono::system_clock::time_point dueTime);

	private:
		void TasksPublisher();
	private:
		std::multimap<std::chrono::system_clock::time_point, AsyncTask*> m_upcomingTasks;
		mutable std::condition_variable m_cv;
		mutable std::mutex m_mut;
		std::unique_ptr<Thread> m_publishingThread;
		std::atomic_bool m_running;
	};
}
