#ifndef NUPPELVIDEORECORDER
#define NUPPELVIDEORECORDER

#include <qstring.h>
#include <qmap.h>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

#ifdef MMX
#undef MMX
#define MMXBLAH
#endif
#include <lame/lame.h>
#ifdef MMXBLAH
#define MMX
#endif

#include "minilzo.h"
#include "format.h"

extern "C" {
#include "../libavcodec/avcodec.h"
#include "filter.h"
}

using namespace std;

struct video_audio;
struct VBIData;
struct cc;
class RTjpeg;
class RingBuffer;

struct MyFilterData
{
    int bShowDeinterlacedAreaOnly;
    int bBlend;
    // int iThresholdBlend; // here we start blending
    int iThreshold;         // here we start interpolating TODO FIXME
    int iEdgeDetect;
    int picsize;
    unsigned char *src;
};

class NuppelVideoRecorder
{
 public:
    NuppelVideoRecorder();
   ~NuppelVideoRecorder();

    void SetRingBuffer(RingBuffer *rbuf) { ringBuffer = rbuf; }
    void SetAsPIP(void) { pip = true; }
    
    void SetCodec(QString desiredcodec) { codec = desiredcodec; }

    void SetRTJpegMotionLevels(int lM1, int lM2) { M1 = lM1; M2 = lM2; }
    void SetRTJpegQuality(int quality) { Q = quality; }

    void SetMP4TargetBitrate(int rate) { targetbitrate = rate; }
    void SetMP4ScaleBitrate(int scale) { scalebitrate = scale;}
    void SetMP4Quality(int max, int min, int diff) 
                       { maxquality = max; minquality = min; qualdiff = diff; }
    void SetMP4OptionVHQ(int value) 
                       { mp4opts = (value) ? mp4opts | CODEC_FLAG_HQ : 
                                             mp4opts & ~CODEC_FLAG_HQ; }
    void SetMP4Option4MV(int value) 
                       { mp4opts = (value) ? mp4opts | CODEC_FLAG_4MV :
                                             mp4opts & ~CODEC_FLAG_4MV; }

    void SetResolution(int width, int height) { w = width; h = height; }
    void SetAudioSampleRate(int rate) { audio_samplerate = rate; }   
    void SetAudioCompression(bool compress) { compressaudio = compress; }
 
    void SetFilename(QString filename) { sfilename = filename; }
    void SetAudioDevice(QString device) { audiodevice = device; }
    void SetVideoDevice(QString device) { videodevice = device; }
    void SetVbiDevice(QString device) { vbidevice = device; }
    void SetTVFormat(QString tvformat);
    void SetVbiFormat(QString vbiformat);
    void SetHMJPGQuality(int quality) { hmjpg_quality = quality; }
    void SetHMJPGHDecimation(int deci) { hmjpg_hdecimation = deci; }
    void SetHMJPGVDecimation(int deci) { hmjpg_vdecimation = deci; }

    void ChangeDeinterlacer(int deint_mode);   
 
    void Initialize(void);
    void StartRecording(void);
    void StopRecording(void) { encoding = false; } 
    
    void Pause(bool clear = true) 
                     { cleartimeonpause = clear;
                       paused = true; pausewritethread = true;
                       actuallypaused = audiopaused = mainpaused = false; }
    void Unpause(void) { paused = false; pausewritethread = false; }
    bool GetPause(void) { return (audiopaused && mainpaused && 
                                  actuallypaused); }
    
    bool IsRecording(void) { return recording; }
   
    long long GetFramesWritten(void) { return framesWritten; } 
    void ResetFramesWritten(void) { framesWritten = 0; }

    void SetMP3Quality(int quality) { mp3quality = quality; }

    int GetVideoFd(void) { return fd; }
    void Reset(void);

    float GetFrameRate(void) { return video_frame_rate; }
  
    void WriteHeader(bool todumpfile = false);

    void SetVideoFilters(QString &filters) { videoFilterList = filters; }
    void InitFilters(void);

    void TransitionToFile(const QString &lfilename);
    void TransitionToRing(void);

    long long GetKeyframePosition(long long desired);

    void GetBlankFrameMap(QMap<long long, int> &blank_frame_map);

 protected:
    static void *WriteThread(void *param);
    static void *AudioThread(void *param);
    static void *VbiThread(void *param);

    void doWriteThread(void);
    void doAudioThread(void);
    void doVbiThread(void);
    
 private:
    int AudioInit(void);
    void InitBuffers(void);
    
    int SpawnChildren(void);
    void KillChildren(void);
    
    void BufferIt(unsigned char *buf, int len = -1);
    
    int CreateNuppelFile(void);

    void WriteVideo(Frame *frame);
    void WriteAudio(unsigned char *buf, int fnum, int timecode);
    void WriteText(unsigned char *buf, int len, int timecode, int pagenr);

    Frame * areaDeinterlace(unsigned char *yuvptr, int width, int height);
    Frame * GetField(struct vidbuffertype *vidbuf, bool top_field, 
                     bool interpolate);
    bool SetupAVCodec(void);

    void WriteSeekTable(bool todumpfile);

    void DoMJPEG();

    void FormatTeletextSubtitles(struct VBIData *vbidata);
    void FormatCC(struct ccsubtitle *subtitle, struct cc *cc, int data);
    
    QString sfilename;
    bool encoding;
    
    int fd; // v4l input file handle
    signed char *strm;
    long dropped;
    unsigned int lf, tf;
    int M1, M2, Q;
    int w, h;
    int pid, pid2;
    int inputchannel;
    int compression;
    int compressaudio;
    unsigned long long audiobytes;
    int audio_channels; 
    int audio_bits;
    int audio_bytes_per_sample;
    int audio_samplerate; // rate we request from sounddevice
    int effectivedsp; // actual measured rate

    int ntsc;
    int quiet;
    int rawmode;
    int usebttv;
    char vbimode;
    struct video_audio *origaudio;

    int mp3quality;
    char *mp3buf;
    int mp3buf_size;
    lame_global_flags *gf;

    QMap<long long, int> blank_frames;

    RTjpeg *rtjc;

#define OUT_LEN (1024*1024 + 1024*1024 / 64 + 16 + 3)    
    lzo_byte out[OUT_LEN];
#define HEAP_ALLOC(var,size) \
    long __LZO_MMODEL var [ ((size) + (sizeof(long) - 1)) / sizeof(long) ]    
    HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

    QString audiodevice;
    QString videodevice;
    QString vbidevice;
 
    vector<struct vidbuffertype *> videobuffer;
    vector<struct audbuffertype *> audiobuffer;
    vector<struct txtbuffertype *> textbuffer;

    int act_video_encode;
    int act_video_buffer;

    int act_audio_encode;
    int act_audio_buffer;
    long long act_audio_sample;
   
    int act_text_encode;
    int act_text_buffer;
 
    int video_buffer_count;
    int audio_buffer_count;
    int text_buffer_count;

    long video_buffer_size;
    long audio_buffer_size;
    long text_buffer_size;

    struct timeval stm;
    struct timezone tzone;

    bool childrenLive;

    pthread_t write_tid;
    pthread_t audio_tid;
    pthread_t vbi_tid;

    bool recording;

    RingBuffer *ringBuffer;
    bool weMadeBuffer;

    int keyframedist;
    vector<struct seektable_entry> *seektable;

    long long extendeddataOffset;

    long long framesWritten;
    double video_frame_rate;

    bool livetv;
    bool paused;
    bool pausewritethread;
    bool actuallypaused;
    bool audiopaused;
    bool mainpaused;

    enum DeinterlaceMode {
        DEINTERLACE_NONE =0,
        DEINTERLACE_BOB,
        DEINTERLACE_BOB_FULLHEIGHT_COPY,
        DEINTERLACE_BOB_FULLHEIGHT_LINEAR_INTERPOLATION,
        DEINTERLACE_DISCARD_TOP,
        DEINTERLACE_DISCARD_BOTTOM,
        DEINTERLACE_AREA,
        DEINTERLACE_LAST
    };

    DeinterlaceMode deinterlace_mode;
    double framerate_multiplier;
    double height_multiplier;

    struct MyFilterData myfd;   
 
    int last_block;
    int firsttc;
    long int oldtc;
    int startnum;
    int frameofgop;
    int lasttimecode;
    int audio_behind;
    
    bool useavcodec;

    AVCodec *mpa_codec;
    AVCodecContext *mpa_ctx;
    AVFrame mpa_picture;

    int targetbitrate;
    int scalebitrate;
    int maxquality;
    int minquality;
    int qualdiff;
    int mp4opts;

    QString codec;

    bool pip;

    QString videoFilterList;
    vector<VideoFilter *> videoFilters;

    PixelFormat picture_format;

    bool hardware_encode;
    int hmjpg_quality;
    int hmjpg_hdecimation;
    int hmjpg_vdecimation;
    int hmjpg_maxw;

    bool cleartimeonpause;

    struct ccsubtitle subtitle;
};

#endif
