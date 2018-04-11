#include <QOpenGLShader>
#include "cqtopencvviewergl.h"


CQtOpenCVViewerGl::CQtOpenCVViewerGl(QWidget *parent) :
        QOpenGLWidget(parent)
{
    mBgColor = QColor::fromRgb(150, 150, 150);
}

void CQtOpenCVViewerGl::initializeGL()
{
    //makeCurrent();
    initializeOpenGLFunctions();

    float r = ((float)mBgColor.darker().red())/255.0f;
    float g = ((float)mBgColor.darker().green())/255.0f;
    float b = ((float)mBgColor.darker().blue())/255.0f;
    glClearColor(r,g,b,1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
            "#version 120\n"
            "attribute vec2 in_Vertex;\n"
            "attribute vec2 textureCoordinate;\n"
            "varying vec2 varyingTextureCoordinate;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(in_Vertex, 0.0, 1.0);\n"
            "    varyingTextureCoordinate = textureCoordinate;\n"
            "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
            "#version 120\n"
            "uniform sampler2D texture;\n"
            "varying vec2 varyingTextureCoordinate;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = texture2D(texture, varyingTextureCoordinate);\n"
            "}\n";
    fshader->compileSourceCode(fsrc);

    program = new QOpenGLShaderProgram;
    program->addShader(vshader);
    program->addShader(fshader);

    program->link();
    program->bind();
    glActiveTexture(GL_TEXTURE0);
    program->setUniformValue("tex", 0);

    glGenTextures(1, &tex);

}

void CQtOpenCVViewerGl::resizeGL(int width, int height)
{
    drawMutex.lock();
    //makeCurrent();
    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, width, -height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    recalculatePosition();

    emit imageSizeChanged(mRenderWidth, mRenderHeight);

//    glDeleteTextures(1, &tex);
//    glGenTextures(1, &tex);

    drawMutex.unlock();
    updateScene();
}

void CQtOpenCVViewerGl::updateScene()
{
    if (this->isVisible()) update();
}

void CQtOpenCVViewerGl::paintGL()
{
    makeCurrent();

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderImage();
}

void CQtOpenCVViewerGl::renderImage()
{

    drawMutex.lock();
    //makeCurrent();

    //glClear(GL_COLOR_BUFFER_BIT);

    if (!mRenderQtImg.isNull())
    {
//        glLoadIdentity();
//
//        glPushMatrix();
//        {
//            if (mResizedImg.width() <= 0)
//            {
//                if (mRenderWidth == mRenderQtImg.width() && mRenderHeight == mRenderQtImg.height())
//                    mResizedImg = mRenderQtImg;
//                else
//                    mResizedImg = mRenderQtImg.scaled(QSize(mRenderWidth, mRenderHeight),
//                                                      Qt::IgnoreAspectRatio,
//                                                      Qt::SmoothTransformation);
//            }
//
//            // ---> Centering image in draw area
//
//            glRasterPos2i(mRenderPosX, mRenderPosY);
//
//            glPixelZoom(1, -1);
//
//            glDrawPixels(mResizedImg.width(), mResizedImg.height(), GL_RGBA, GL_UNSIGNED_BYTE, mResizedImg.bits());
//
//        }
//        glPopMatrix();


        //glClear(GL_COLOR_BUFFER_BIT);

        program->bind();
        {
            float vertices[] = {-1.0,-1.0,  1.0,-1.0,  1.0,1.0,  -1.0,1.0};

            float coordTexture[] = {0.0,1.0,  1.0,1.0,  1.0,0.0,  0.0,0.0};

            GLint vertexLocation = glGetAttribLocation( program->programId(), "in_Vertex" );
            GLint texcoordLocation = glGetAttribLocation( program->programId(), "textureCoordinate" );

            program->enableAttributeArray(vertexLocation);
            program->enableAttributeArray(texcoordLocation);

            program->setAttributeArray(texcoordLocation, coordTexture, 2);

            program->setAttributeArray(vertexLocation, vertices, 2);



            glBindTexture(GL_TEXTURE_2D, tex);

            glDrawArrays(GL_QUADS, 0, 4);

            glBindTexture(GL_TEXTURE_2D, 0);

            program->disableAttributeArray(vertexLocation);
            program->disableAttributeArray(texcoordLocation);

        }
        program->release();
        // end
        glFlush();
    }

    drawMutex.unlock();
}

void CQtOpenCVViewerGl::recalculatePosition()
{
    const qreal retinaScale = devicePixelRatio();

    mImgRatio = (float)mOrigImage.cols/(float)mOrigImage.rows;

    mRenderWidth = this->size().width();
    mRenderHeight = floor(mRenderWidth / mImgRatio);

    if (mRenderHeight > this->size().height())
    {
        mRenderHeight = this->size().height();
        mRenderWidth = floor(mRenderHeight * mImgRatio);
    }

    mRenderPosX = floor((this->size().width() - mRenderWidth) / 2);
    mRenderPosY = -floor((this->size().height() - mRenderHeight) / 2);

    mRenderHeight *= retinaScale;
    mRenderWidth *= retinaScale;
}

bool CQtOpenCVViewerGl::showImage(const cv::Mat& image)
{
    if(mOrigImage.cols>0) mOrigImage.release();
    if (image.channels() == 3)
        cvtColor(image, mOrigImage, CV_BGR2RGBA);
    else if (image.channels() == 1)
        cvtColor(image, mOrigImage, CV_GRAY2RGBA);
    else if (image.channels() == 4)
        mOrigImage = image;
    else return false;

    mRenderQtImg = QImage((const unsigned char*)(mOrigImage.data),
                          mOrigImage.cols, mOrigImage.rows,
                          mOrigImage.step1(), QImage::Format_RGBA8888);

    drawMutex.lock();

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, mRenderQtImg.width(), mRenderQtImg.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, mRenderQtImg.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture( GL_TEXTURE_2D, 0);

    recalculatePosition();

    updateScene();
    drawMutex.unlock();
    return true;
}
