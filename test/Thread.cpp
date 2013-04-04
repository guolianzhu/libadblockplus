#include <gtest/gtest.h>
#include <queue>
#include <vector>

#include "../src/Thread.h"

namespace
{
  class Mock : public AdblockPlus::Thread
  {
  public:
    int timesCalled;
    AdblockPlus::Mutex mutex;

    Mock() : timesCalled(0)
    {
    }

    void Run()
    {
      AdblockPlus::Sleep(5);
      AdblockPlus::Lock lock(mutex);
      timesCalled++;
    }
  };

  class LockingMock : public AdblockPlus::Thread
  {
  public:
    bool working;

    LockingMock(const std::string& name, std::vector<std::string>& log,
                AdblockPlus::Mutex& logMutex)
      : name(name), log(log), logMutex(logMutex)
    {
    }

    void Run()
    {
      AdblockPlus::Lock lock(logMutex);
      log.push_back(name);
    }

  private:
    const std::string name;
    std::vector<std::string>& log;
    AdblockPlus::Mutex& logMutex;
  };

  class Enqueuer : public AdblockPlus::Thread
  {
  public:
    Enqueuer(std::queue<int>& queue, AdblockPlus::Mutex& queueMutex,
             AdblockPlus::ConditionVariable& notEmpty)
      : queue(queue), queueMutex(queueMutex), notEmpty(notEmpty)
    {
    }

    void Run()
    {
      AdblockPlus::Lock lock(queueMutex);
      queue.push(1);
      notEmpty.Signal();
    }

  private:
    std::queue<int>& queue;
    AdblockPlus::Mutex& queueMutex;
    AdblockPlus::ConditionVariable& notEmpty;
  };

  class Dequeuer : public AdblockPlus::Thread
  {
  public:
    Dequeuer(std::queue<int>& queue, AdblockPlus::Mutex& queueMutex,
             AdblockPlus::ConditionVariable& notEmpty)
      : queue(queue), queueMutex(queueMutex), notEmpty(notEmpty)
    {
    }

    void Run()
    {
      AdblockPlus::Lock lock(queueMutex);
      if (!queue.size())
        notEmpty.Wait(queueMutex);
      queue.pop();
    }

  private:
    std::queue<int>& queue;
    AdblockPlus::Mutex& queueMutex;
    AdblockPlus::ConditionVariable& notEmpty;
  };
}

TEST(ThreadTest, Run)
{
  Mock mock;
  ASSERT_EQ(0, mock.timesCalled);
  mock.Start();
  mock.mutex.Lock();
  ASSERT_EQ(0, mock.timesCalled);
  mock.mutex.Unlock();
  mock.Join();
  ASSERT_EQ(1, mock.timesCalled);
}

TEST(ThreadTest, Mutex)
{
  std::vector<std::string> log;
  AdblockPlus::Mutex logMutex;
  LockingMock mock1("mock1", log, logMutex);
  LockingMock mock2("mock2", log, logMutex);
  mock1.Start();
  AdblockPlus::Sleep(5);
  mock2.Start();
  mock1.Join();
  mock2.Join();
  ASSERT_EQ("mock1", log[0]);
  ASSERT_EQ("mock2", log[1]);
}

TEST(ThreadTest, ConditionVariable)
{
  std::queue<int> queue;
  AdblockPlus::Mutex queueMutex;
  AdblockPlus::ConditionVariable notEmpty;
  Dequeuer dequeuer(queue, queueMutex, notEmpty);
  Enqueuer enqueuer(queue, queueMutex, notEmpty);
  dequeuer.Start();
  AdblockPlus::Sleep(5);
  enqueuer.Start();
  enqueuer.Join();
  dequeuer.Join();
  ASSERT_EQ(0, queue.size());
}
