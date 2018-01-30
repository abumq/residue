//
//  logging-queue.h
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#ifndef LoggingQueue_h
#define LoggingQueue_h

#include <deque>
#include <mutex>
#include "src/core/request-handler.h"
#include "src/logging/log-request.h"


namespace residue {

///
/// \brief Logging queue with context switch feature
///
class LoggingQueue
{
public:
    LoggingQueue() :
        m_backlogQueue(&m_queue1),
        m_dispatchQueue(&m_queue2) {}

    virtual ~LoggingQueue() = default;

    inline std::mutex* lock() { return &m_mutex; }

    inline void push(RawRequest&& rawRequest)
    {
        m_backlogQueue->push_front(std::move(rawRequest));
    }

    RawRequest pull();

    inline bool empty()
    {
        return m_dispatchQueue->empty();
    }

    inline std::size_t size()
    {
        return m_dispatchQueue->size();
    }

    inline bool backlogEmpty()
    {
        return m_backlogQueue->empty();
    }

    inline bool backlogSize()
    {
        return m_backlogQueue->size();
    }

    void switchContext();

private:

    std::deque<RawRequest> m_queue1;
    std::deque<RawRequest> m_queue2;

    std::mutex m_mutex;

    std::deque<RawRequest>* m_backlogQueue;
    std::deque<RawRequest>* m_dispatchQueue;
};
}
#endif /* LoggingQueue_h */