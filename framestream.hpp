#ifndef FRAMESTREAM_H
#define FRAMESTREAM_H

#include <QObject>
#include <QTimer>
#include <QImage>

#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/videoio.hpp>

#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>

}

class FrameStream: public QObject
{
    Q_OBJECT
public:
    FrameStream();
    ~FrameStream();

    struct macroblockInfo {
        int size;

    };

public slots:
    void openVideoCamera(QString path);
    void pause();
    void play();
    void getNextFrame();
    void seekToFrame(qint64 frame);
    void seekToTimecode(QString timecode);
    void selectMacroblock(qint32 x, qint32 y);

private :

    struct MbInfo {
        uint32_t mbType=-1;
        uint32_t num=-1;
        bool isIntra4x4 = false;
        bool isIntra16x16 = false;
        bool isPCM = false;
        bool isIntra = false;
        bool isInter = false;
        bool isSkip = false;
        bool isIntraPCM = false;
        bool isInterlaced = false;
        bool isDirect = false;
        bool isGMC=false;
        bool is16x16=false;
        bool is16x8=false;
        bool is8x16=false;
        bool is8x8=false;
        bool isSub8x8=false;
        bool isSub8x4=false;
        bool isSub4x4=false;
        bool isSub4x8=false;
        bool isACPRED=false;
        bool isQuant=false;
    };


    void drawMacroblockGrid(cv::Mat& frame, std::shared_ptr<AVFrame> avFrame);
    void avFrameToCVmat(cv::Mat& destFrame);
    QString getFrameType();

    void manyTransparrentRectangle(cv::Mat& img, std::vector<std::pair<cv::Point,cv::Point>> arr, cv::Vec3i color);
    void avFrameToCVmatWithConvert(cv::Mat& destFrame);

    void ffmpegInit();
    void analyzeQPtable(int8_t *qscale_table, std::vector<macroblockInfo>& mbArray);
    std::unique_ptr<AVFormatContext> _formatContext2;
    std::shared_ptr<AVFormatContext> _formatContext;
    std::shared_ptr<AVCodecContext> _codecContext;
    std::shared_ptr<AVPacket> _packet;
    std::shared_ptr<AVFrame> _frame;
    int videoStreamIdx = -1;
    cv::Mat transparrentRectangle(cv::Mat& img, cv::Point p1, cv::Point P2, cv::Vec3i color);


    cv::Mat frame;
    cv::VideoCapture cap;
    QTimer tUpdate;
    bool _pause = false;
    int64_t _frameNum = 0;
    int64_t dbgNum = 0;
    QString frameNumToStr(int count);
    QString _frameCountStr;
    double  getDurationSec() ;
    int _videoStreamId = -1;
    int _imgWidth = -1;
    int _imgHeight = -1;

    int _macroblockPerLine = 0;

    cv::Mat _tmpImg;

    double _fps = -1;
    double getFps() const;
    double  eps_zero = 0.000025;

    std::vector<MbInfo> _curFrameMbInfo;

signals:
    void newImage(QImage &);
    void updateFrameNum(QString, int);
    void initSlider(int maxFrameCount);
    void updateTextView(QString mbInfo);
    void updateFrameTypeInfo(QString frameTypeInfo);

};

#endif // FRAMESTREAM_H
