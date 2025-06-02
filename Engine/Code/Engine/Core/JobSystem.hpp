#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <deque>
#include <atomic>
#include <mutex>
#include <thread>

class JobSystem;
class Job;
class JobWorkerThread;

enum class JobStatus
{
	CREATED,
	QUEUED,
	CLAIMED,
	COMPLETED,
	RETRIEVED,
};

struct JobworkerTypeToCreate
{
	int m_jobWorkersOfType = 0;
	uint32_t m_jobWokerTypeBitmask = (uint32_t) -1;
};

class Job
{
public:
	Job(uint32_t jobTypeBit);
	Job() = default;
	virtual ~Job() = default;
	virtual void Execute() = 0;
	std::atomic<JobStatus> m_jobStatus = JobStatus::CREATED;
	uint32_t m_jobTypeBit = 1;
};

class JobWorkerThread
{
	friend class JobSystem;
public:
	JobWorkerThread(int threadId, JobSystem* jobSystem);
	void JobWorkerThreadMain();
private:
	//default to accepting all jobs
	uint32_t m_acceptedJobsBitmask = (uint32_t)-1;
	JobSystem* m_jobSystem = nullptr;
	int m_threadID = 0;
	std::thread my_thread;
};

struct JobSystemConfig
{
	int m_threadsToCreate = 0;
	std::vector<JobworkerTypeToCreate> m_jobWorkerTypesToCreate;
};

class JobSystem
{
	friend class JobWorkerThread;
public:
	void StartUp();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	JobSystem(JobSystemConfig const& config);
	int GetQueuedJobsWithBit(uint32_t jobBit);
	void GetCompletedJobs(std::vector<Job*>& out_completedJobs, int maxJobsToRetrieve = -1);
	Job* GetCompletedJob();
	void QueueJob(Job* job);
	void QueueJobs(std::vector<Job*>& jobs);

private:
	Job* GetJobForWorker(JobWorkerThread* worker);
	std::atomic<bool> m_isQuitting = false;
	JobSystemConfig m_config;
	std::mutex m_queuedJobsMutex;
	std::deque<Job*> m_queuedJobs;
	std::mutex m_claimedJobsMutex;
	std::deque<Job*> m_claimedJobs;
	std::mutex m_completedJobsMutex;
	std::deque<Job*> m_completedJobs;
	std::vector<JobWorkerThread*> m_jobWorkerThreads;

};