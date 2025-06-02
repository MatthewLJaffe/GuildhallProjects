#include "Engine/Core/JobSystem.hpp"
#include <thread>

JobSystem* g_theJobSystem = nullptr;

JobWorkerThread::JobWorkerThread(int threadId, JobSystem* jobSystem)
	: m_threadID(threadId)
	, m_jobSystem(jobSystem)
{
	my_thread = std::thread(&JobWorkerThread::JobWorkerThreadMain, this);
}

void JobWorkerThread::JobWorkerThreadMain()
{
	while (!m_jobSystem->m_isQuitting)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(1));

		m_jobSystem->m_queuedJobsMutex.lock();
		Job* claimedJob = nullptr;
		if (m_jobSystem->m_queuedJobs.size() > 0)
		{
			claimedJob = m_jobSystem->GetJobForWorker(this);
		}
		m_jobSystem->m_queuedJobsMutex.unlock();
		if (claimedJob == nullptr)
		{
			continue;
		}

		m_jobSystem->m_claimedJobsMutex.lock();
		m_jobSystem->m_claimedJobs.push_back(claimedJob);
		claimedJob->m_jobStatus = JobStatus::CLAIMED;
		m_jobSystem->m_claimedJobsMutex.unlock();
		claimedJob->Execute();

		m_jobSystem->m_claimedJobsMutex.lock();
		for (auto it = m_jobSystem->m_claimedJobs.begin(); it != m_jobSystem->m_claimedJobs.end(); ++it)
		{
			if (*it == claimedJob)
			{
				m_jobSystem->m_claimedJobs.erase(it);
				break;
			}
		}
		m_jobSystem->m_claimedJobsMutex.unlock();

		m_jobSystem->m_completedJobsMutex.lock();
		m_jobSystem->m_completedJobs.push_back(claimedJob);
		claimedJob->m_jobStatus = JobStatus::COMPLETED;
		m_jobSystem->m_completedJobsMutex.unlock();
	}
}

JobSystem::JobSystem(JobSystemConfig const& config)
	: m_config(config)
{
}

int JobSystem::GetQueuedJobsWithBit(uint32_t jobBit)
{
	int jobsInQueue = 0;
	m_queuedJobsMutex.lock();
	for (Job* job : m_queuedJobs)
	{
		if (job->m_jobTypeBit == jobBit)
		{
			jobsInQueue++;
		}
	}
	m_queuedJobsMutex.unlock();

	m_claimedJobsMutex.lock();
	for (Job* job : m_claimedJobs)
	{
		if (job->m_jobTypeBit == jobBit)
		{
			jobsInQueue++;
		}
	}
	m_claimedJobsMutex.unlock();

	return jobsInQueue;
}


void JobSystem::GetCompletedJobs(std::vector<Job*>& out_completedJobs, int maxJobsToRetrieve)
{
	m_completedJobsMutex.lock();
	if (maxJobsToRetrieve < 0)
	{
		maxJobsToRetrieve = (int)m_completedJobs.size();
	}
	while (m_completedJobs.size() > 0 && maxJobsToRetrieve > 0)
	{
		Job* currJob = m_completedJobs.front();
		m_completedJobs.pop_front();
		currJob->m_jobStatus = JobStatus::RETRIEVED;
		out_completedJobs.push_back(currJob);
		maxJobsToRetrieve--;
	}
	m_completedJobsMutex.unlock();
}

Job* JobSystem::GetCompletedJob()
{
	m_completedJobsMutex.lock();
	if (m_completedJobs.size() == 0)
	{
		m_completedJobsMutex.unlock();
		return nullptr;
	}
	Job* completedJob = m_completedJobs.front();
	completedJob->m_jobStatus = JobStatus::RETRIEVED;
	m_completedJobs.pop_front();
	m_completedJobsMutex.unlock();

	return completedJob;
}

void JobSystem::QueueJob(Job* job)
{
	m_queuedJobsMutex.lock();
	m_queuedJobs.push_back(job);
	job->m_jobStatus = JobStatus::QUEUED;
	m_queuedJobsMutex.unlock();
}

void JobSystem::QueueJobs(std::vector<Job*>& jobs)
{
	m_queuedJobsMutex.lock();
	for (int i = 0; i < (int)jobs.size(); i++)
	{
		if (jobs[i] == nullptr)
		{
			continue;
		}
		m_queuedJobs.push_back(jobs[i]);
		jobs[i]->m_jobStatus = JobStatus::QUEUED;
		
	}
	m_queuedJobsMutex.unlock();
}

Job* JobSystem::GetJobForWorker(JobWorkerThread* worker)
{
	for (auto it = m_queuedJobs.begin(); it != m_queuedJobs.end(); ++ it)
	{
		if ((*it)->m_jobTypeBit & worker->m_acceptedJobsBitmask)
		{
			Job* job = *it;
			m_queuedJobs.erase(it);
			return job;
		}
	}
	return nullptr;
}

void JobSystem::StartUp()
{
	int numThreadsToStartup = m_config.m_threadsToCreate;
	if (numThreadsToStartup < 0)
	{
		numThreadsToStartup = std::thread::hardware_concurrency();
	}
	for (int i = 0; i < numThreadsToStartup; i++)
	{
		m_jobWorkerThreads.push_back(new JobWorkerThread(i+1, this));
	}

	int currWorkerIdx = 0;
	for (int i = 0; i < m_config.m_jobWorkerTypesToCreate.size(); i++)
	{
		for (int jt = 0; jt < m_config.m_jobWorkerTypesToCreate[i].m_jobWorkersOfType; jt++)
		{
			m_jobWorkerThreads[currWorkerIdx]->m_acceptedJobsBitmask = m_config.m_jobWorkerTypesToCreate[i].m_jobWokerTypeBitmask;
			currWorkerIdx++;
		}
	}
}

void JobSystem::Shutdown()
{
	m_isQuitting = true;
	for (int i = 0; i < m_jobWorkerThreads.size(); i++)
	{
		m_jobWorkerThreads[i]->my_thread.join();
		delete m_jobWorkerThreads[i];
		m_jobWorkerThreads[i] = nullptr;
	}
	m_jobWorkerThreads.clear();

	for (Job* job : m_queuedJobs)
	{
		delete job;
		job = nullptr;
	}
	m_queuedJobs.clear();

	for (Job* job : m_claimedJobs)
	{
		delete job;
		job = nullptr;
	}
	m_claimedJobs.clear();

	for (Job* job : m_completedJobs)
	{
		delete job;
		job = nullptr;
	}
	m_completedJobs.clear();
}

void JobSystem::BeginFrame()
{
}

void JobSystem::EndFrame()
{
}

Job::Job(unsigned int jobTypeBit)

	: m_jobTypeBit(jobTypeBit)
{
}
