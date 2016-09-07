#ifndef SRS_APP_TRANSFER_MANAGER_HPP
#define SRS_APP_TRANSFER_MANAGER_HPP

#include <srs_app_hls2rtmp.hpp>
    
class SrsAppTransferManager
{
private:
    std::vector<SrsHls2Rtmp*> hls2rtmps;
    
public:
    SrsAppTransferManager();
    ~SrsAppTransferManager();
    int initialize();
    int write_task_file(ISrsAppTransferTask* task);
    int rm_task_file(ISrsAppTransferTask* task);
    int add_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp);
    int del_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp);
    SrsHls2Rtmp* get_hls2rtmp_task(string hlsuri, string rtmpuri);
};

extern SrsAppTransferManager* _transfer_manager;

#endif
