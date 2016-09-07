#include <srs_core.hpp>

#include <stdlib.h>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#include <srs_kernel_error.hpp>
#include <srs_app_server.hpp>
#include <srs_app_log.hpp>
#include <srs_core_autofree.hpp>
#include <srs_protocol_json.hpp>
#include <srs_app_utility.hpp>
#include <srs_app_transfer_manager.hpp>

#define SRS_TRANSFER_TASK_FILE_DIR "/letv/fet/live/srs_transfer"

SrsAppTransferManager* _transfer_manager = new SrsAppTransferManager();

SrsAppTransferManager::SrsAppTransferManager()
{

}

SrsAppTransferManager::~SrsAppTransferManager()
{

}

int SrsAppTransferManager::write_task_file(ISrsAppTransferTask* task)
{
    char file[256] = {0};

    sprintf(file, "%s/%s", SRS_TRANSFER_TASK_FILE_DIR, task->getId().c_str());
    
    int fd = ::open(file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR |  S_IRGRP | S_IROTH);
    if (fd < 0) {
        srs_error("open task file failed, file=%s", file);
        return -1;
    }

    int size = task->getContent().length();

srs_trace("content:%s size:%d", task->getContent().c_str(), size); 

    if (write(fd, task->getContent().c_str(), size) != size) {
        srs_error("write task file failed, file=%s", file);
        close(fd);
        return -1;
    }
    close(fd);

    return ERROR_SUCCESS;
}

int SrsAppTransferManager::rm_task_file(ISrsAppTransferTask* task)
{
    char file[256] = {0};

    sprintf(file, "%s/%s", SRS_TRANSFER_TASK_FILE_DIR, task->getId().c_str());

    if (unlink(file) < 0) {
        srs_error("rm task file failed, file=%s", file);
        return -1;
    }

    return ERROR_SUCCESS;
}

int SrsAppTransferManager::initialize()
{
    DIR* dir;
    struct dirent* ent;
    char file[512];
    char buf[8192]; 
    
    if (!(dir = opendir(SRS_TRANSFER_TASK_FILE_DIR))) {
        srs_error("open task file dir failed, dir=%s", SRS_TRANSFER_TASK_FILE_DIR);
        return -1;
    }

    while((ent = readdir(dir)) != NULL) {        
        if (0 == strcmp(ent->d_name, ".") || 0 == strcmp(ent->d_name, "..") || DT_DIR == ent->d_type) {
			continue;
		}
                
        snprintf(file, sizeof(buf), "%s/%s", SRS_TRANSFER_TASK_FILE_DIR, ent->d_name);
        FILE* fp = fopen(file, O_RDONLY);

        if (fp) {
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                SrsJsonAny* info = SrsJsonAny::loads(buf);
                if (!info) {
                    srs_error("parse transfer task file error1, file=%s", file);
                    continue;
                }
                SrsAutoFree(SrsJsonAny, info);
                
                if (!info->is_object()) {
                    srs_error("parse transfer task file error2, file=%s", file);
                    continue;
                }
                
                SrsJsonObject* req_info = info->to_object();
                
                SrsJsonAny* req_action = NULL;
                if ((req_action = req_info->ensure_property_string("action")) == NULL) {
                    srs_error("parse transfer task file error3, file=%s", file);
                    continue;
                }

                SrsJsonAny* req_input = NULL;
                if ((req_input = req_info->ensure_property_string("input")) == NULL) {
                    srs_error("parse transfer task file error4, file=%s", file);
                    continue;
                }

                SrsJsonAny* req_output = NULL;
                if ((req_output = req_info->ensure_property_string("output")) == NULL) {
                    srs_error("parse transfer task file error5, file=%s", file);
                    continue;
                }

                srs_trace("hls2rtmp invoke by initialize, action:%s, input:%s, output:%s", req_action->to_str().c_str(), 
                    req_input->to_str().c_str(), req_output->to_str().c_str());

                SrsHls2Rtmp* hls2rtmp = new SrsHls2Rtmp();
                if (ERROR_SUCCESS != hls2rtmp->initialize(req_input->to_str(), req_output->to_str(), string(buf))) {
                    srs_error("input or output is invalid url");
                    delete hls2rtmp;
                    continue;
                }

                hls2rtmp->start();               
            }
            fclose(fp);
        }
    }

    closedir(dir);

    return ERROR_SUCCESS;
}

int SrsAppTransferManager::add_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp)
{
    if (ERROR_SUCCESS != write_task_file(hls2rtmp)) {     
        return -1;
    }

    hls2rtmps.push_back(hls2rtmp);
    
    return ERROR_SUCCESS;
}

int SrsAppTransferManager::del_hls2rtmp_task(SrsHls2Rtmp* hls2rtmp)
{
    if (ERROR_SUCCESS != rm_task_file(hls2rtmp)) {     
        return -1;
    }
    
    std::vector<SrsHls2Rtmp*>::iterator it;
    if ((it = std::find(hls2rtmps.begin(), hls2rtmps.end(), hls2rtmp)) != hls2rtmps.end()) {
        hls2rtmps.erase(it);
    }

    return ERROR_SUCCESS;
}

SrsHls2Rtmp* SrsAppTransferManager::get_hls2rtmp_task(string hlsuri, string rtmpuri)
{
    char buf[1024] = {0};
    
    snprintf(buf, 1024, "%s-%s", hlsuri.c_str(), rtmpuri.c_str());
    string id = srs_get_md5(buf, strlen(buf));

    for (std::vector<SrsHls2Rtmp*>::iterator it = hls2rtmps.begin(); it != hls2rtmps.end(); ++it) {
        SrsHls2Rtmp* hls2rtmp = *it;
        if (hls2rtmp->getId() == id) {
            return hls2rtmp;
        }
    }

    return NULL;
}