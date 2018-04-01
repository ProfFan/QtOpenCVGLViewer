#include <QApplication>
#include <QException>
#include <QDebug>
#include <QSurfaceFormat>

#include "qtglwebcamdemo.h"

int main(int argc, char *argv[])
{
    int res=-1;

    try
    {
        QApplication a(argc, argv);

        QSurfaceFormat glFormat;
        glFormat.setVersion(2,2);
        glFormat.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(glFormat);

        QtGLWebcamDemo w;
        w.show();

        res = a.exec();
    }
    catch(QException &e)
    {
        qCritical() << QString("Exception: %1").arg( e.what() );
    }
    catch(...)
    {
        qCritical() << QString("Unhandled Exception");
    }

    return res;
}