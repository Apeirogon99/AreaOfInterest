#pragma once
#include <iostream>
#include <functional>
#include <queue>
#include <map>
#include <set>
#include <mutex>
#include <chrono>
#include <thread>

class TaskNode
{
public:
	TaskNode() : mPriority(0)
	{

	}

	~TaskNode()
	{

	}

public:
	bool operator<(const TaskNode& inOtherNode) const
	{
		return this->mPriority < inOtherNode.mPriority;
	}

	template<typename T, typename Ret, typename... Args>
	void Init(const int inPriority, const std::unique_ptr<T>& inOwner, Ret(T::* inMemberFunc)(Args...), Args&&... inArgs)
	{
		this->mPriority = inPriority;
		T* owner = inOwner.get();
		mCallBack = [owner, inMemberFunc, inArgs...]()
			{
				if (owner)
				{
					(owner->*inMemberFunc)(inArgs...);
				}
			};
	}

	void Execute()
	{
		mCallBack();
	}

	const int GetPriority() const
	{
		return mPriority;
	}

private:
	std::function<void()>	mCallBack;
	int						mPriority;
};

class TaskQueue
{
public:
	TaskQueue() {}
	~TaskQueue() {}

	TaskQueue(TaskQueue&&) = delete;
	TaskQueue(const TaskQueue&) = delete;

	TaskQueue& operator=(TaskQueue&&) = delete;
	TaskQueue& operator=(const TaskQueue&) = delete;

public:
	template<typename T, typename Ret, typename... Args>
	void PushTask(const int inPriority, const std::unique_ptr<T>& inObject, Ret(T::* inMemberFunc)(Args...), Args... inArgs)
	{
		std::unique_ptr<TaskNode> newTaskNode = std::make_unique<TaskNode>();
		newTaskNode->Init(inPriority, inObject, inMemberFunc, std::forward<Args>(inArgs)...);

		std::lock_guard<std::mutex> lock(mTaskQueueMutex);
		mTaskQueue.insert(std::move(newTaskNode));
	}

	void ExecuteTasks(const long long inServiceTimeStamp)
	{
		std::vector<std::unique_ptr<TaskNode>> executeTaskNode;

		{
			std::lock_guard<std::mutex> lock(mTaskQueueMutex);
			for (auto iter = mTaskQueue.begin(); iter != mTaskQueue.end();)
			{
				if ((*iter)->GetPriority() > inServiceTimeStamp) break;

				auto node = mTaskQueue.extract(iter++);
				executeTaskNode.emplace_back(std::move(node.value()));
			}
		}

		for (const std::unique_ptr<TaskNode>& node : executeTaskNode)
		{
			node->Execute();
		}
	}

private:
	std::multiset<std::unique_ptr<TaskNode>> mTaskQueue;
	std::mutex mTaskQueueMutex;
};