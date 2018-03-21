#ifndef FAILURE_NOTIFY_H
#define FAILURE_NOTIFY_H

#include <residue/extensions.h>

using namespace residue;

class FailureNotify : public DispatchErrorExtension {
public:

	FailureNotify() : DispatchErrorExtension("failure-notify"), m_failureCount(0UL), m_sent(false)
	{
		
	}
	
	virtual ~FailureNotify() = default;

	virtual Extension::Result execute(const DispatchErrorExtension::Data* const) override;
	virtual void reset() override;
private:
	unsigned long m_failureCount;
        bool m_sent;

	void notifyRecipients(const DispatchErrorExtension::Data* const data);
};

#endif


