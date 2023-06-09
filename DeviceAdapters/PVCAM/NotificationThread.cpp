#include "NotificationThread.h"

#include "PVCAMAdapter.h"

NotificationThread::NotificationThread(Universal* pCamera)
    : universal_(pCamera)
{
}

NotificationThread::~NotificationThread()
{
    requestStop();
}

void NotificationThread::Reset()
{
    overflowed_ = false;
}

void NotificationThread::SetQueueCapacity(int capacity)
{
    maxSize_ = capacity;
}

int  NotificationThread::Capacity() const
{
    return maxSize_;
}

bool NotificationThread::PushNotification(const NotificationEntry& entry)
{
    std::lock_guard<std::mutex> lock(threadMutex_);

    bool bRet = true;

    deque_.push_back(entry);
    if (static_cast<int>(deque_.size()) > maxSize_)
    {
        // We delete the first notification in the queue because it's the oldest
        // entry and there is a risk that the frame pointer will get soon overwritten
        // by PVCAM.
        deque_.pop_front();
        overflowed_ = true;
        bRet = false; // The queue has overflowed
    }
    else
    {
        frameReadyCondition_.notify_one();
    }

    return bRet;
}

// From MMDeviceThreadBase
int NotificationThread::svc()
{
    while (!requestStop_)
    {
        NotificationEntry n;
        if (waitNextNotification(n))
        {
            universal_->ProcessNotification(n);
        }
        else
        {   // Request stop flagged
            break;
        }
    }
    return 0;
}

//===================================================================== PRIVATE

void NotificationThread::requestStop()
{
    // Request the thread to stop
    {
        std::lock_guard<std::mutex> lock(threadMutex_);
        requestStop_ = true;
    }
    frameReadyCondition_.notify_one();

    // Wait for the thread function to exit
    this->wait();
}

bool NotificationThread::waitNextNotification(NotificationEntry& e)
{
    std::unique_lock<std::mutex> threadLock(threadMutex_);

    if (deque_.size() == 0)
    {
        frameReadyCondition_.wait(threadLock);
        if (requestStop_)
            return false;
    }

    // There is a new frame notification in the queue
    e = deque_.front();
    deque_.pop_front();

    return true;
}
