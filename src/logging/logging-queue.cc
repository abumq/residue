//
//  logging-queue.cc
//  Residue
//
//  Copyright © 2017 Muflihun Labs
//

#include "src/logging/logging-queue.h"

using namespace residue;


void LoggingQueue::switchContext()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    RESIDUE_UNUSED(lock);

#if RESIDUE_DEBUG
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
    return rawRequest;
}