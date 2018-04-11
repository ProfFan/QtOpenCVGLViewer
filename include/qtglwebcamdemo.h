#ifndef QTGLWEBCAMDEMO_H
#define QTGLWEBCAMDEMO_H

#include <QMainWindow>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/core/ocl.hpp>
#include <QTime>
#include <QFuture>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <AKAZE.h>

namespace Ui {
    class QtGLWebcamDemo;
}

class ImageReader : public QObject {
public:
    ImageReader(AKAZEOptions opt);
    QFuture<cv::Mat> read(const bool mFlipVert, const bool mFlipHoriz);
    cv::VideoCapture mCapture;
public:
    libAKAZECU::AKAZE det;
};

class QtGLWebcamDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit QtGLWebcamDemo(QWidget *parent = 0);
    ~QtGLWebcamDemo();

private slots:
            void on_actionStart_triggered();

    void on_actionVertical_Flip_triggered(bool checked);

    void on_action_Horizontal_Mirror_triggered(bool checked);

    void on_watcher_finished();

private:
    Ui::QtGLWebcamDemo *ui;

    std::unique_ptr<ImageReader> reader;

    int frameCount = 0;
    QFuture<cv::Mat> future;
    double fps = 0;

    QTime t;
    int timerHandle;

    QFutureWatcher<cv::Mat>* watcher = new QFutureWatcher<cv::Mat>();

protected:
    void timerEvent(QTimerEvent *event);

private:
    bool mFlipVert;
    bool mFlipHoriz;
};

#endif // QTGLWEBCAMDEMO_H