//
//  logging-queue.h
//  Residue
//
//  Copyright 2017-present @abumq (Majid Q.)
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#ifndef LoggingQueue_h
#define LoggingQueue_h

#include <deque>
#include <mutex>

#include "core/request-handler.h"
#include "logging/log-request.h"


namespace residue {

///
/// \brief Logging queue with context switch feature
///
class LoggingQueue final : NonCopyable
{
public:
    LoggingQueue();

    inline void push(RawRequest&& rawRequest)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_backlogQueue->push_front(std::move(rawRequest));
    }

    RawRequest pull();

    inline bool empty() const
    {
        return m_dispatchQueue->empty();
    }

    inline std::size_t size() const
    {
        return m_dispatchQueue->size();
    }

    inline bool backlogEmpty() const
    {
        return m_backlogQueue->empty();
    }

    inline std::size_t backlogSize() const
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
