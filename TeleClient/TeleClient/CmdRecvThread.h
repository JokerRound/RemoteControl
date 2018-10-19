#pragma once

class CCmdRecvThread :
    public CThread
{
public:
    CCmdRecvThread();
    virtual ~CCmdRecvThread();

    virtual bool OnThreadEventRun(LPVOID lpParam);
};

