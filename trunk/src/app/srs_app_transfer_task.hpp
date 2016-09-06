#ifndef SRS_APP_TRANSFER_TASK_HPP
#define SRS_APP_TRANSFER_TASK_HPP

class ISrsAppTransferTask
{
public:
    virtual string getId() = 0;
    virtual string getContent() = 0;
};

#endif