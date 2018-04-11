#include "qtglwebcamdemo.h"
#include "ui_qtglwebcamdemo.h"

#include "AKAZE.h"


ImageReader::ImageReader(AKAZEOptions opt): det(opt) {

}

QFuture<cv::Mat> ImageReader::read(const  bool mFlipVert, const bool mFlipHoriz)
{
    auto that = this;
    auto readImageWorker = [that](const bool mFlipVert, const bool mFlipHoriz) -> cv::Mat {
        cv::Mat image;
        cv::UMat _img;

        that->mCapture >> image;

        if( mFlipVert && mFlipHoriz )
            cv::flip( image,image, -1);
        else if( mFlipVert )
            cv::flip( image,image, 0);
        else if( mFlipHoriz )
            cv::flip( image,image, 1);

        cv::Ptr<cv::FastFeatureDetector> det = cv::FastFeatureDetector::create(50);

//        cv::Ptr<cv::ORB> det = cv::ORB::create(1000);
//        AKAZEOptions opt;
        //opt.descriptor = DESCRIPTOR_TYPE::MLDB;
//        opt.img_width = image.cols;
//        opt.img_height = image.rows;
//        libAKAZECU::AKAZE det = libAKAZECU::AKAZE(opt);

        image.copyTo(_img);
        std::vector<cv::KeyPoint> keypoints;
        det->detect(_img, keypoints);

        _img.release();
//        cv::Mat img_bw;
//        cv::Mat img_32;
//        cv::cvtColor(image, img_bw, CV_BGR2GRAY);
//        img_bw.convertTo(img_32, CV_32F, 1.0 / 255.0, 0);
        //det.Allocate_Memory_Evolution();
//        that->det.Create_Nonlinear_Scale_Space(img_32);
//        that->det.Feature_Detection(keypoints);

//        img_32.release();
//        img_bw.release();
        cv::drawKeypoints(image, keypoints, image, cv::Scalar(2,254,255));
        if (image.channels() == 3)
            cvtColor(image, image, CV_BGR2RGBA);
        else if (image.channels() == 1)
            cvtColor(image, image, CV_GRAY2RGBA);

        //image.copyTo(_img);
        //image.release();
        //det.release();
        return image;
    };
    return QtConcurrent::run(readImageWorker, mFlipVert, mFlipHoriz);
}


QtGLWebcamDemo::QtGLWebcamDemo(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::QtGLWebcamDemo)
{
    ui->setupUi(this);

    mFlipVert=false;
    mFlipHoriz=false;
    QThreadPool::globalInstance()->setMaxThreadCount(4);

    connect(watcher, SIGNAL(finished()), this, SLOT(on_watcher_finished()), Qt::DirectConnection);

    cv::ocl::setUseOpenCL(true);
    if (!cv::ocl::haveOpenCL())
    {
        std::cout << "OpenCL is not available..." << std::endl;
        //return;
    }

    cv::ocl::Context context;
    if (!context.create(cv::ocl::Device::TYPE_GPU))
    {
        std::cout << "Failed creating the context..." << std::endl;
        //return;
    }

    std::cout << context.ndevices() << " GPU devices are detected." << std::endl; //This bit provides an overview of the OpenCL devices you have in your computer
    for (int i = 0; i < context.ndevices(); i++)
    {
        cv::ocl::Device device = context.device(i);
        std::cout << "name:              " << device.name() << std::endl;
        std::cout << "available:         " << device.available() << std::endl;
        std::cout << "imageSupport:      " << device.imageSupport() << std::endl;
        std::cout << "OpenCL_C_Version:  " << device.OpenCL_C_Version() << std::endl;
        std::cout << std::endl;
    }

    AKAZEOptions opt;
    opt.img_width = 1280;
    opt.img_height = 720;

    reader.reset(new ImageReader(opt));
}

QtGLWebcamDemo::~QtGLWebcamDemo()
{
    delete ui;
}

void QtGLWebcamDemo::on_actionStart_triggered()
{
    if(reader->mCapture.isOpened()){
        reader->mCapture.release();
    }
    if( !reader->mCapture.isOpened() )
        if( !reader->mCapture.open( "rtspsrc location=rtsp://10.20.41.40:554/user=admin&password=sustc910910&channel=1&stream=0.sdp?real_stream latency=0 droponlatency=1 ! rtph264depay ! video/x-h264,framerate=30/1 ! h264parse ! queue ! vtdec ! queue ! videoconvert ! queue ! appsink sync=false") )
            return;

    //reader.mCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640); // valueX = your wanted width
    //reader.mCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480); // valueY = your wanted heigth
//    reader.mCapture.set(CV_CAP_PROP_FPS, 30);
//    reader.mCapture.set(CV_CAP_PROP_BUFFERSIZE, 1);

    if( !reader->mCapture.isOpened() )
        if( !reader->mCapture.open( 0, cv::VideoCaptureAPIs::CAP_AVFOUNDATION) )
            return;
    std::cout << "resolution:              " << reader->mCapture.get(cv::CAP_PROP_FRAME_WIDTH) << reader->mCapture.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
    std::cout << "fps:                     " << reader->mCapture.get(cv::CAP_PROP_FPS) << std::endl;
    timerHandle = startTimer(10, Qt::PreciseTimer);
}

void QtGLWebcamDemo::timerEvent(QTimerEvent *event)
{

    if(future.isRunning()) return;

    future = reader->read(mFlipVert, mFlipHoriz);

    watcher->setFuture(future);
}

void QtGLWebcamDemo::on_actionVertical_Flip_triggered(bool checked)
{
    mFlipVert = checked;
}

void QtGLWebcamDemo::on_action_Horizontal_Mirror_triggered(bool checked)
{
    mFlipHoriz = checked;
}

void QtGLWebcamDemo::on_watcher_finished() {
    cv::Mat image;

    // Do what you want with the image :-)
    //.copyTo(image);
    // Show the image
    ui->openCVviewer->showImage(future.result());
    future.result().release();
    frameCount ++;

    int te = t.elapsed();
    if(te>0) {
        fps = fps - 0.1 * (fps - (1000 / te));
        ui->statusBar->showMessage(QString("%1").arg(QString::number(fps)));
    }

    t.start();
}
