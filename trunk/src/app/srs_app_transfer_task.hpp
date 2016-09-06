#ifndef SRS_APP_TRANSFER_TASK_HPP
#define SRS_APP_TRANSFER_TASK_HPP

class ISrsAppTransferTask
{
public:
    ISrsAppTransferTask();
    virtual ~ISrsAppTransferTask();
public:
    virtual string getId();
    virtual string getContent();
};

#endif