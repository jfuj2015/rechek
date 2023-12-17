#ifndef __RECHECK_THREAD_TOOLKIT_H__
#define __RECHECK_THREAD_TOOLKIT_H__

#include <queue>
#include <vector>
#include <condition_variable>
#include <functional>
namespace recheck{

	class ThreadPool
	{
	public:
		class Task
		{
		public:
			virtual ~Task() {}
			virtual void DoJob() = 0;
		};

	public:
		ThreadPool(int maxThreads = 8) : m_maxThreads(maxThreads) {}

		~ThreadPool()
		{
			m_exit = true;
			m_condition.notify_all();
			for (auto& thread : m_threads) {
				if (thread.joinable()) {
					thread.join();
				}
			}
		}

		void AddTask(std::shared_ptr<Task> task)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_taskQueue.push(task);
			m_condition.notify_one();

			if (m_threads.size() == 0
				|| (m_taskQueue.size() > 2 && m_threads.size() < m_maxThreads)) {

				int id = static_cast<int>(m_threadActive.size());
				m_threadActive.push_back(false);
				m_threads.push_back(std::thread([this, id]() {

					do {
						std::shared_ptr<Task> task = nullptr;
						{
							std::unique_lock<std::mutex> lock(m_mutex);
							std::chrono::seconds sec(3);
							if ((m_taskQueue.size() == 0 && std::cv_status::timeout == m_condition.wait_for(lock, sec))
								|| m_taskQueue.size() == 0) {
								m_threadActive[id] = false;
								continue;
							}
							//至此
							m_threadActive[id] = true;
							task = m_taskQueue.front();
							m_taskQueue.pop();
						}
						task->DoJob();

					} while (!m_exit);

					m_threadActive[id] = false;
					}));
			}
		}

		//测试是否全部完成
		bool IsDone()
		{
			if (m_taskQueue.size() > 0) {
				return false;
			}
			for (auto active : m_threadActive) {
				if (active) {
					return false;
				}
			}
			return true;
		}

	private:
		bool m_exit = false;
		int m_maxThreads = 8;
		int m_freeThreads = 0;
		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::queue<std::shared_ptr<Task>> m_taskQueue;
		std::vector<std::thread> m_threads;
		std::vector<bool> m_threadActive;
	};


	//template<class T>
	class ThreadPool2
	{
	public:
		ThreadPool2(int maxThreads = 8) : m_maxThreads(maxThreads) {}

		~ThreadPool2()
		{
			m_exit = true;
			m_condition.notify_all();
			for (auto& thread : m_threads) {
				if (thread.joinable()) {
					thread.join();
				}
			}
		}

		//void AddTask(std::function<void()>& func)
		//{
		//	AddTask(std::forward<std::function<void()>&&>(func));
		//}

		void AddTask(std::function<void()>&& func)
		{
			std::unique_lock<std::mutex> lock(m_mutex);

			m_taskQueue.push(func);
			m_condition.notify_one();

			if (m_threads.size() == 0
				|| (m_taskQueue.size() > 2 && m_threads.size() < m_maxThreads)) {

				int id = static_cast<int>(m_threadActive.size());
				m_threadActive.push_back(false);
				m_threads.push_back(
					std::thread([this, id]() {
						do {
							std::function<void()> task = nullptr;
							{
								std::unique_lock<std::mutex> lock(m_mutex);
								std::chrono::seconds sec(3);
								if ((m_taskQueue.size() == 0 && std::cv_status::timeout == m_condition.wait_for(lock, sec))
									|| m_taskQueue.size() == 0) {
									m_threadActive[id] = false;
									continue;
								}
								//至此
								m_threadActive[id] = true;
								task = m_taskQueue.front();
								m_taskQueue.pop();
							}
							task();
						} while (!m_exit);

						m_threadActive[id] = false;
					})
				);
			}
		}

		//测试是否全部完成
		bool IsDone()
		{
			if (m_taskQueue.size() > 0) {
				return false;
			}
			for (auto active : m_threadActive) {
				if (active) {
					return false;
				}
			}
			return true;
		}

	private:
		bool m_exit = false;
		int m_maxThreads = 8;
		int m_freeThreads = 0;
		std::mutex m_mutex;
		std::condition_variable m_condition;
		std::queue<std::function<void()>> m_taskQueue;
		std::vector<std::thread> m_threads;
		std::vector<bool> m_threadActive;
	};

} //recheck

#endif // !__RECHECK_THREAD_TOOLKIT_H__

