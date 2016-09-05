#ifndef SRS_APP_INGEST_HLS_HPP
#define SRS_APP_INGEST_HLS_HPP

#ifdef SRS_AUTO_INGEST

#include <srs_core.hpp>
#include <srs_kernel_ts.hpp>
#include <srs_raw_avc.hpp>

class ISrsAacHandler
{
public:
    /**
     * handle the aac frame, which in ADTS format(starts with FFFx).
     * @param duration the duration in seconds of frames.
*/
virtual int on_aac_frame(char* frame, int frame_size, double duration) = 0;
};

// the context to ingest hls stream.
class SrsIngestSrsInput
{
private:
    struct SrsTsPiece {
        double duration;
        std::string url;
        std::string body;
        
        // should skip this ts?
        bool skip;
        // already sent to rtmp server?
        bool sent;
        // whether ts piece is dirty, remove if not update.
        bool dirty;
        
        SrsTsPiece() {
            skip = false;
            sent = false;
            dirty = false;
        }
        
        int fetch(std::string m3u8);
    };
private:
    SrsHttpUri* in_hls;
    std::vector<SrsTsPiece*> pieces;
    int64_t next_connect_time;
private:
    SrsStream* stream;
    SrsTsContext* context;
public:
    SrsIngestSrsInput(SrsHttpUri* hls);
    virtual ~SrsIngestSrsInput();
    /**
     * parse the input hls live m3u8 index.
     */
    virtual int connect();
    /**
     * parse the ts and use hanler to process the message.
     */
    virtual int parse(ISrsTsHandler* ts, ISrsAacHandler* aac);
private:
    /**
     * parse the ts pieces body.
     */
    virtual int parseAac(ISrsAacHandler* handler, char* body, int nb_body, double duration);
    virtual int parseTs(ISrsTsHandler* handler, char* body, int nb_body);
    /**
     * parse the m3u8 specified by url.
     */
    virtual int parseM3u8(SrsHttpUri* url, double& td, double& duration);
    /**
     * find the ts piece by its url.
     */
    virtual SrsTsPiece* find_ts(string url);
    /**
     * set all ts to dirty.
     */
    virtual void dirty_all_ts();
    /**
     * fetch all ts body.
     */
    virtual int fetch_all_ts(bool fresh_m3u8);
    /**
     * remove all ts which is dirty.
     */
    virtual void remove_dirty();
};

// the context to output to rtmp server
class SrsIngestSrsOutput : virtual public ISrsTsHandler, virtual public ISrsAacHandler
{
private:
    SrsHttpUri* out_rtmp;
private:
    bool disconnected;
    std::multimap<int64_t, SrsTsMessage*> queue;
    int64_t raw_aac_dts;
private:
    SrsRequest* req;
    st_netfd_t stfd;
    SrsStSocket* io;
    SrsRtmpClient* client;
    int stream_id;
private:
    SrsRawH264Stream* avc;
    std::string h264_sps;
    bool h264_sps_changed;
    std::string h264_pps;
    bool h264_pps_changed;
    bool h264_sps_pps_sent;
private:
    SrsRawAacStream* aac;
    std::string aac_specific_config;
public:
    SrsIngestSrsOutput(SrsHttpUri* rtmp);
    virtual ~SrsIngestSrsOutput();
// interface ISrsTsHandler
public:
    virtual int on_ts_message(SrsTsMessage* msg);
// interface IAacHandler
public:
    virtual int on_aac_frame(char* frame, int frame_size, double duration);
private:
    virtual int do_on_aac_frame(SrsStream* avs, double duration);
    virtual int parse_message_queue();
    virtual int on_ts_video(SrsTsMessage* msg, SrsStream* avs);
    virtual int write_h264_sps_pps(u_int32_t dts, u_int32_t pts);
    virtual int write_h264_ipb_frame(std::string ibps, SrsCodecVideoAVCFrame frame_type, u_int32_t dts, u_int32_t pts);
    virtual int on_ts_audio(SrsTsMessage* msg, SrsStream* avs);
    virtual int write_audio_raw_frame(char* frame, int frame_size, SrsRawAacStreamCodec* codec, u_int32_t dts);
private:
    virtual int rtmp_write_packet(char type, u_int32_t timestamp, char* data, int size);
public:
    /**
     * connect to output rtmp server.
     */
    virtual int connect();
    /**
     * flush the message queue when all ts parsed.
     */
    virtual int flush_message_queue();
private:
    virtual int connect_app(std::string ep_server, std::string ep_port);
    // close the connected io and rtmp to ready to be re-connect.
    virtual void close();
};

class SrsIngestSrsContext
{
private:
    SrsIngestSrsInput* ic;
    SrsIngestSrsOutput* oc;
public:
    SrsIngestSrsContext(SrsHttpUri* hls, SrsHttpUri* rtmp);
    virtual ~SrsIngestSrsContext();
    virtual int proxy();
};

class SrsHls2Rtmp : public ISrsReusableThreadHandler
{
private:
    SrsReusableThread* pthread;
    SrsHttpUri hls_uri;
    SrsHttpUri rtmp_uri;
    SrsIngestSrsContext ingest_context;
    
public:
    SrsHls2Rtmp();
    ~SrsHls2Rtmp();
    int initialize(string hlsuri, string rtmpuri);
    virtual int start();
    virtual void stop();
    virtual int cycle();
    virtual void on_thread_stop();   
};

#endif
#endif
