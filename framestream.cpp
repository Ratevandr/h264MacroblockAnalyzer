#include "framestream.hpp"

#include <QDebug>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <QStringLiteral>

#include "qp_table.h"
extern "C" {
#include "ffmpegMacroblockdDef.h"
}

FrameStream::FrameStream()
{
    connect(&tUpdate, &QTimer::timeout, this, &FrameStream::getNextFrame);
    ffmpegInit();
}

FrameStream::~FrameStream() {
    cap.release();
    tUpdate.stop();
}

cv::Mat FrameStream::transparrentRectangle(cv::Mat& img, cv::Point p1, cv::Point p2, cv::Vec3i color) {
    cv::Mat overlay;

    img.copyTo(overlay, img);
    cv::rectangle(overlay, p1,
                  p2,
                  color, -1);

    cv::Mat finImg;
    cv::addWeighted(overlay, 0.5, img, 0.5, 0, finImg);
    return finImg;
}

void FrameStream::manyTransparrentRectangle(cv::Mat& img,
                                            std::vector<std::pair<cv::Point, cv::Point> > arr,
                                            cv::Vec3i color) {
    cv::Mat overlay;

    img.copyTo(overlay, img);

    for (auto& pointPair : arr) {
        cv::rectangle(overlay, pointPair.first,
                      pointPair.second,
                      color, -1);
    }

    cv::addWeighted(overlay, 0.4, img, 0.4, 0, img);
}

void FrameStream::selectMacroblock(qint32 x, qint32 y) {
    const int mbSize = 16;

    int xPos = std::floor(x / mbSize);
    int yPos = std::floor(y / mbSize);

    qDebug() << xPos << yPos;
    cv::Mat showImg = transparrentRectangle(_tmpImg, cv::Point(xPos * mbSize, mbSize * yPos),
                                            cv::Point(xPos * mbSize + mbSize, mbSize * yPos + mbSize),
                                            cv::Vec3i(125, 125, 0));


    QImage img = QImage(showImg.data,
                        showImg.cols,
                        showImg.rows,
                        QImage::Format_RGB888
                        ).rgbSwapped();

    MbInfo& info = _curFrameMbInfo[xPos + (yPos) * _macroblockPerLine];
    QString curMbInfoString;
    QString binStr = QString("%1").arg(info.mbType, 32, 2, QChar('0'));;// QString::number(info.num,2);
    QTextStream(&curMbInfoString) << "Pos " << xPos << " " << yPos << "\n" <<
            binStr  << "\n" <<
            "Num: " <<                        info.num << "\n" <<
            "Intra4x4: " <<                        info.isIntra4x4 << "\n" <<
            "Intra16x16: " <<                             info.isIntra16x16 << "\n" <<
            "PCM: " <<                                    info.isPCM       << "\n" <<
            "Intra: " <<                                  info.isIntra     << "\n" <<
            "Inter: " <<                                  info.isInter     << "\n" <<
            "Skip: " <<                                   info.isSkip      << "\n" <<
            "IntraPCM: " <<                               info.isIntraPCM  << "\n" <<
            "Interlaced: " <<                             info.isInterlaced << "\n" <<
            "Direct: " <<                                 info.isDirect    << "\n" <<
            "GMC: " <<                                    info.isGMC       << "\n" <<
            "16x16: " <<                                  info.is16x16     << "\n" <<
            "16x8: " <<                                   info.is16x8      << "\n" <<
            "8x16: " <<                                   info.is8x16      << "\n" <<
            "8x8: " <<                                    info.is8x8       << "\n" <<
            "Sub8x8: " <<                                 info.isSub8x8    << "\n" <<
            "Sub8x4: " <<                                 info.isSub8x4    << "\n" <<
            "Sub4x4: " <<                                 info.isSub4x4    << "\n" <<
            "Sub4x8: " <<                                 info.isSub4x8    << "\n" <<
            "ACPRED: " <<                                 info.isACPRED    << "\n" <<
            "Quant: " <<                                  info.isQuant    << "\n";
    emit newImage(img);
    emit updateTextView(curMbInfoString);
}

void FrameStream::ffmpegInit() {
    _formatContext = std::shared_ptr<AVFormatContext>(avformat_alloc_context(), [](AVFormatContext *avf) {
        avformat_free_context(avf);
    });
    av_log_set_level(AV_LOG_DEBUG);
}

void FrameStream::analyzeQPtable(int8_t *qscale_table, std::vector<macroblockInfo>& mbArray) {}

void FrameStream::drawMacroblockGrid(cv::Mat& frame, std::shared_ptr<AVFrame> avFrame) {
    _curFrameMbInfo.clear();
    _curFrameMbInfo.reserve(_codecContext->width * _codecContext->height);

    _macroblockPerLine = _codecContext->width / 16 + 1;
    const int mbSize = 16;
    int  allMacroblockCount = ((_codecContext->height + 15) >> 4) * _macroblockPerLine;
    auto avFramePtr = avFrame.get();

    std::vector<std::pair<cv::Point, cv::Point> > mbCoordArray;

    for (int i = 0; i < allMacroblockCount; ++i) {
        int row = i / _macroblockPerLine;

        int col = i % (_macroblockPerLine);

        uint32_t   *mb_type_array = (uint32_t *)avFramePtr->opaque;
        uint32_t    mbType = mb_type_array[i];
        std::string kek = std::to_string(avFramePtr->qscale_table[i]);

        MbInfo info;
        info.mbType = mbType;
        info.num = avFramePtr->qscale_table[i];

        // with ffmpeg predefined macros
        if (IS_INTRA4x4(mbType)) {
            info.isIntra4x4 = true;
        }

        if (IS_INTRA16x16(mbType)) {
            info.isIntra16x16 = true;
        }

        if (IS_PCM(mbType)) {
            info.isPCM = true;
        }

        if (IS_INTRA(mbType)) {
            info.isIntra = true;
        }

        if (IS_INTER(mbType)) {
            info.isInter = true;
        }

        if (IS_SKIP(mbType)) {
            info.isSkip = true;
            mbCoordArray.push_back(std::pair(cv::Point(col * mbSize, mbSize * row),
                                             cv::Point(col * mbSize + mbSize, (mbSize) * row + mbSize)));
        }

        if (IS_INTRA_PCM(mbType)) {
            info.isIntraPCM = true;
        }

        if (IS_INTERLACED(mbType)) {
            info.isInterlaced = true;
        }

        if (IS_DIRECT(mbType)) {
            info.isDirect = true;
        }

        if (IS_GMC(mbType)) {
            info.isGMC = true;
        }

        if (IS_16X8(mbType)) {
            info.is16x8 = true;
        }

        if (IS_8X8(mbType)) {
            info.is8x8 = true;
        }

        if (IS_SUB_8X8(mbType)) {
            info.isSub8x8 = true;
        }

        if (IS_SUB_8X4(mbType)) {
            info.isSub8x4 = true;
        }

        if (IS_SUB_4X8(mbType)) {
            info.isSub4x8 = true;
        }

        if (IS_SUB_4X4(mbType)) {
            info.isSub4x4 = true;
        }

        if (IS_ACPRED(mbType)) {
            info.isACPRED = true;
        }

        if (IS_QUANT(mbType)) {
            info.isQuant = true;
        }

        cv::rectangle(frame, cv::Point(col * mbSize, mbSize * row),
                      cv::Point(col * mbSize + mbSize, (mbSize) * row + mbSize),
                      cv::Vec3i(125, 125, 0));
        int font = cv::FONT_HERSHEY_SIMPLEX;
        cv::putText(frame, kek, cv::Point(col * mbSize + 2, mbSize * row + mbSize - 4), font, 0.3,
                    cv::Vec3i(0, 255, 0), 1);


        _curFrameMbInfo.push_back(info);
    }

    manyTransparrentRectangle(frame, mbCoordArray, cv::Vec3i(0, 255, 255));
}

void FrameStream::avFrameToCVmatWithConvert(cv::Mat& destFrame) {
    int width = _frame->width;
    int height = _frame->height;

    assert(destFrame.rows == height && destFrame.cols == width && destFrame.type() == CV_8UC3);

    int cvLinesizes[1];

    /*
        image.step1() = (width of a single matrix row in bytes) / (element
           size in bytes)
        width of a single matrix row in bytes = image.step1() * element size
           in bytes
     */
    cvLinesizes[0] = static_cast<int>(destFrame.step1());

    // Convert the colour format and write directly to the opencv matrix
    SwsContext *conversion = sws_getContext(width,
                                            height,
                                            static_cast<AVPixelFormat>(_frame->format),
                                            width,
                                            height,
                                            AV_PIX_FMT_BGR24,
                                            SWS_FAST_BILINEAR,
                                            nullptr,
                                            nullptr,
                                            nullptr);
    sws_scale(conversion, _frame->data, _frame->linesize, 0, height, &destFrame.data, cvLinesizes);
    sws_freeContext(conversion);
}

void FrameStream::avFrameToCVmat(cv::Mat& destFrame) {
    if (_frame->format == AV_PIX_FMT_YUV420P) {// 64
        int width = _frame->width;
        int height = _frame->height;

        int numBytes = av_image_get_buffer_size((AVPixelFormat)_frame->format, width, height, 1);
        uint8_t *buffer = reinterpret_cast<uint8_t *>(av_malloc(numBytes));

        av_image_copy_to_buffer(buffer,
                                numBytes,
                                _frame->data,
                                _frame->linesize,
                                (AVPixelFormat)_frame->format,
                                width,
                                height,
                                1);

        qDebug() << (int)_frame->format;
        cv::Mat yuv = cv::Mat(_frame->height + _frame->height / 2,
                              _frame->width, CV_8UC1, buffer);


        cv::Mat gray = cv::Mat(_frame->height, _frame->width, CV_8UC1, buffer);

        destFrame = cv::Mat(_frame->height, _frame->width, CV_8UC3);

        if (!yuv.empty()) {
            cvtColor(yuv, destFrame, cv::COLOR_YUV420p2RGB);


            drawMacroblockGrid(destFrame, _frame);
            _tmpImg = destFrame.clone();
        }

        av_free(buffer);
    } else if (_frame->format == AV_PIX_FMT_YUV420P10LE) {
        int width = _frame->width;
        int height = _frame->height;

        int numBytes = av_image_get_buffer_size((AVPixelFormat)_frame->format, width, height, 1);
        uint8_t *buffer = reinterpret_cast<uint8_t *>(av_malloc(numBytes));

        av_image_copy_to_buffer(buffer,
                                numBytes,
                                _frame->data,
                                _frame->linesize,
                                (AVPixelFormat)_frame->format,
                                width,
                                height,
                                1);

        qDebug() << (int)_frame->format;
        cv::Mat yuv = cv::Mat(_frame->height + _frame->height / 2,
                              _frame->width, CV_8UC1, buffer);


        cv::Mat gray = cv::Mat(_frame->height, _frame->width, CV_8UC1, buffer);

        destFrame = cv::Mat(_frame->height, _frame->width, CV_8UC3);

        if (!yuv.empty()) {
            cvtColor(yuv, destFrame, cv::COLOR_YUV420p2RGB);


            drawMacroblockGrid(destFrame, _frame);
            _tmpImg = destFrame.clone();
        }

        av_free(buffer);
    } else {
        destFrame = cv::Mat(_frame->height, _frame->width, CV_8UC3);

        if (_frame->width * _frame->height > 0) {
            avFrameToCVmatWithConvert(destFrame);
            drawMacroblockGrid(destFrame, _frame);
            _tmpImg = destFrame.clone();
        }
    }
}

QString FrameStream::frameNumToStr(int count) {
    return QStringLiteral("00:00:00:%1").arg(count);
}

QString FrameStream::getFrameType() {
    switch (_frame->pict_type) {
      case AV_PICTURE_TYPE_NONE:
          return "None";
          break;
      case AV_PICTURE_TYPE_I:
          return "I";
          break;
      case AV_PICTURE_TYPE_P:
          return "P";
          break;
      case AV_PICTURE_TYPE_B:
          return "B";
          break;
      case AV_PICTURE_TYPE_S:
          return "S";
          break;
      case AV_PICTURE_TYPE_SI:
          return "SI";
          break;
      case AV_PICTURE_TYPE_SP:
          return "SP";
          break;
      case AV_PICTURE_TYPE_BI:
          return "BI";
          break;
    }
}

void FrameStream::getNextFrame() {
    auto formatContextPtr = _formatContext.get();
    auto pkg = _packet.get();
    auto pCodecContext = _codecContext.get();
    auto pFrame = _frame.get();

    if (av_read_frame(formatContextPtr, pkg) >= 0) {
        if (pkg->stream_index == videoStreamIdx) {
            if (avcodec_send_packet(pCodecContext, pkg) < 0) {
                qDebug() << "Fail send packet";
            }

            if (avcodec_receive_frame(pCodecContext, pFrame) < 0) {
                qDebug() << "Fail recieve frame";
            }
            cv::Mat cvMatFrame;
            avFrameToCVmat(cvMatFrame);

            if (!cvMatFrame.empty()) {
                QImage img = QImage(cvMatFrame.data,
                                    cvMatFrame.cols,
                                    cvMatFrame.rows,
                                    QImage::Format_RGB888
                                    ).rgbSwapped();

                _frameNum = _formatContext->streams[_videoStreamId]->cur_dts;
                _frameNum /= 1000;

                emit newImage(img);
                emit updateFrameNum(frameNumToStr(_frameNum), _frameNum);
                emit updateFrameTypeInfo(getFrameType());
            } else {
                getNextFrame();
            }
        } else {
            getNextFrame();
        }
    } else {
        qDebug() << "Fail read name";
    }
}

double FrameStream::getFps() const
{
    auto   formatContextPtr = _formatContext.get();
    double fps = av_q2d(av_guess_frame_rate(formatContextPtr, _formatContext->streams[_videoStreamId], NULL));

    if (fps < eps_zero)
    {
        fps = 1.0 / av_q2d(_formatContext->streams[_videoStreamId]->codec->time_base);
    }

    if (fps < eps_zero) {
        fps = av_q2d(_formatContext->streams[_videoStreamId]->avg_frame_rate);
    }
    return fps;
}

double  FrameStream::getDurationSec() {
    double sec = (double)_formatContext->duration / (double)AV_TIME_BASE;

    if (sec < 0.00001)
    {
        sec = (double)_formatContext->streams[_videoStreamId]->duration *
              av_q2d(_formatContext->streams[_videoStreamId]->time_base);
    }

    qDebug() << sec;
    return sec;
}

void FrameStream::seekToTimecode(QString timecode) {
    qDebug() << timecode;
    QRegExp rx("(\\d+)");
    QStringList list;
    int pos = 0;

    while ((pos = rx.indexIn(timecode, pos)) != -1) {
        list << rx.cap(1);
        pos += rx.matchedLength();
    }
    int hours = list[0].toInt();
    int minutes = list[1].toInt();
    int seconds = list[2].toInt();
    int frames = list[3].toInt();
    qint64 totalFrames = hours * 60 * 60 * _fps + minutes * 60 * _fps + seconds * _fps + frames;
    seekToFrame(totalFrames);
    qDebug() << hours << minutes << seconds << frames;
    qDebug() << totalFrames;
}

void FrameStream::seekToFrame(qint64 frame)
{
    auto formatContextPtr = _formatContext.get();

    auto pCodecContext = _codecContext.get();

    if (avformat_seek_file(formatContextPtr, _videoStreamId, INT64_MIN, frame * 1000, INT64_MAX, 0) < 0) {
        av_log(NULL, AV_LOG_ERROR, "ERROR av_seek_frame: %lld\n", frame);
    } else {
        avcodec_flush_buffers(pCodecContext);
    }

    while (frame != _frameNum) {
        getNextFrame();
        qDebug() << frame << _frameNum;
    }
}

void FrameStream::openVideoCamera(QString path) {
    std::string filePath = path.toStdString();

    auto formatContextPtr = _formatContext.get();

    int ret = avformat_open_input(&formatContextPtr, filePath.c_str(), nullptr, nullptr);

    qDebug() << ret;
    char err[1024] = { 0 };
    av_strerror(ret, err, 1024);
    qDebug() << err;

    if (avformat_find_stream_info(formatContextPtr, nullptr) < 0) {
        qDebug() << "Error find stream info";
    }

    for (size_t i = 0; i < _formatContext->nb_streams; ++i) {
        AVCodecParameters *streamParameters = _formatContext->streams[i]->codecpar;

        AVCodec *codec = avcodec_find_decoder(streamParameters->codec_id);

        if (streamParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            _videoStreamId = i;
            _fps = getFps();

            double durationSec = getDurationSec();
            qDebug() << "Fps: " << _fps << " duration: " << durationSec;

            videoStreamIdx = i;
            _codecContext =
                std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(codec), [](AVCodecContext *ctx) {
                avcodec_free_context(&ctx);
            });

            auto pCodecContext = _codecContext.get();

            if (avcodec_parameters_to_context(pCodecContext, streamParameters) < 0) {
                qDebug() << "Error copy decoder context";
            }

            avcodec_open2(pCodecContext, codec, nullptr);
            qDebug() << _formatContext->streams[i]->nb_frames;
            emit initSlider(_formatContext->streams[i]->nb_frames);
            break;
        }
    }

    _packet = std::shared_ptr<AVPacket>(av_packet_alloc(), [](AVPacket *pkg) {
        av_packet_free(&pkg);
    });

    _frame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame *frm) {
        av_freep(frm->opaque);
        av_frame_free(&frm);
    });

    _pause = true;
    getNextFrame();
}

void FrameStream::pause() {
    if (_pause != true) {
        tUpdate.stop();
    }
    _pause = true;
}

void FrameStream::play() {
    if (_pause != false) {
        qDebug() << _fps;
        tUpdate.start(1000 / _fps);
    }
    _pause = false;
}
