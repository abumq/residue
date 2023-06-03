//
//  logging-queue.cc
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

#include "logging/logging-queue.h"

using namespace residue;

LoggingQueue::LoggingQueue() :
    m_backlogQueue(&m_queue1),
    m_dispatchQueue(&m_queue2)
{
}

void LoggingQueue::switchContext()
{
    std::lock_guard<std::mutex> lock(m_mutex);

#ifdef RESIDUE_DEBUG
    DRVLOG_IF(!m_queue1.empty() || !m_queue2.empty(), RV_DEBUG)
                    << "Context switched, queue 1: " << m_queue1.size()
                    << " items, queue 2: " << m_queue2.size() << " items";
#endif

    std::swap(m_dispatchQueue, m_backlogQueue);
}

RawRequest LoggingQueue::pull()
{
    RawRequest rawRequest = m_dispatchQueue->back();
    m_dispatchQueue->pop_back();
    return std::move(rawRequest);
}
