#pragma once

#include <functional>
#include <condition_variable>
#include <mutex>
#include <string>

namespace core
{
	class AsyncTask
	{
	public:
		explicit AsyncTask(const std::function<void(void)>& requestedPoint);
		AsyncTask():m_state(AsyncTaskState::CREATED){}
		AsyncTask(const AsyncTask& obj);
		AsyncTask& operator=(const AsyncTask& obj);
		virtual ~AsyncTask(){}

	public:
		enum class AsyncTaskState
		{
			CREATED,
			RUNNING,
			CANCELED,
			COMPLETED
		};

	public:
		//Properties
		void ChangeState(AsyncTaskState newState){
			m_state = newState;
		}
		void SetFailureReason(const std::string failureReason){m_failureReason = failureReason;}
		AsyncTaskState GetState() const {
			return m_state;
		}
		std::string GetFailureReason() const {
			return m_failureReason;
		}
		std::function<void(void)> GetRequestedEntryPoint() const{
			return m_requestedEntryPoint;
		}
		void Start();
		void Wait();
		void NotifyOnFailure();

	private:
		std::function<void(void)> m_requestedEntryPoint;
		AsyncTaskState m_state;
		mutable std::condition_variable m_waitCv;
		mutable std::mutex m_waitMut;
		std::string m_failureReason;
	};
}
