#ifndef SRS_APP_TRANSFER_MANAGER_HPP
#define SRS_APP_TRANSFER_MANAGER_HPP

#include <srs_app_hls2rtmp.hpp>
    
class SrsAppTransferManager
{
private:
    static std::vector<SrsHls2Rtmp*> hls2rtmps;
    
public:
    SrsAppTransferManager();
    ~SrsAppTransferManager();
    static int initialize();
    static int write_task_file(ISrsAppTransferTask* task);
    static int rm_task_file(ISrsAppTransferTask* task);
    static int add_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp);
    static int del_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp);
    static SrsHls2Rtmp* get_hls2rtmp_task(string hlsuri, string rtmpuri);
};

#endif
