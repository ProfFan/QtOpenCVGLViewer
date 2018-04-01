//
// Created by Fan Jiang on 29/03/2018.
//

#include <iostream>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "GL/glew.h"
#include <GL/freeglut.h>

GLuint txt;
cv::Mat img;
cv::VideoCapture vcp;

int loadImage(GLuint target, const cv::Mat& _image){
    CV_Assert(_image.cols > 0);

    cv::Mat image;
    _image.copyTo(image);

    cv::flip(image, image, 0);

    //std::cout << "CV OK\n" << std::endl;

    glBindTexture(GL_TEXTURE_2D, target);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //std::cout << "TexParam OK\n" << std::endl;

    glTexImage2D(GL_TEXTURE_2D,     // Type of texture
                 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,            // Internal colour format to convert to
                 image.cols,          // Image width  i.e. 640 for Kinect in standard mode
                 image.rows,          // Image height i.e. 480 for Kinect in standard mode
                 0,                 // Border width in pixels (can either be 1 or 0)
                 GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,  // Image data type
                 image.ptr());        // The actual image data itself

    //glGenerateMipmap(GL_TEXTURE_2D);

    return 0;
}

void display (void)
{
    glClearColor (0.0,0.0,0.0,1.0);
    glClear (GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glEnable( GL_TEXTURE_2D );

    glBindTexture(GL_TEXTURE_2D, txt);

    glBegin (GL_QUADS);
    glTexCoord2d(0.0,0.0); glVertex2d(0.0,0.0);
    glTexCoord2d(1.0,0.0); glVertex2d(img.cols,0.0);
    glTexCoord2d(1.0,1.0); glVertex2d(img.cols,img.rows);
    glTexCoord2d(0.0,1.0); glVertex2d(0.0,img.rows);
    glEnd();

    vcp >> img;
    loadImage(txt, img);

//    glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);//GL_FASTEST);
//    glGenerateMipmap(GL_TEXTURE_2D); // Generate mip mapping
//    glFlush();
    glutSwapBuffers();
}


void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 1.0, 100.0);
    glMatrixMode (GL_MODELVIEW);
}


int main (int argc, char **argv)
{

    if(!vcp.open(0)) return -1;

    while(img.cols < 1) vcp >> img;


    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize (img.cols, img.rows);
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("A basic OpenGL Window");
    glutDisplayFunc (display);
    glutIdleFunc (display);
    glutReshapeFunc (reshape);
    glEnable( GL_TEXTURE_2D );

    glGenTextures(1, &txt);

    //cv::cvtColor(img, img, CV_BGR2RGB);
    loadImage(txt, img);

    glutMainLoop ();

    return 0;
}
