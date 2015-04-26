#include "glwidget.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <stdexcept>
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"

GLWidget::GLWidget( const QGLFormat& format, QWidget* parent )
    : QGLWidget( format, parent ),
      m_vertexBuffer( QOpenGLBuffer::VertexBuffer )
{
}

void GLWidget::initializeGL()
{
    // Resolve OpenGL functions
    glewExperimental = true;
    GLenum GlewInitResult = glewInit();
    if (GlewInitResult != GLEW_OK) {
        const GLubyte* errorStr = glewGetErrorString(GlewInitResult);
        size_t size = strlen(reinterpret_cast<const char*>(errorStr));
        qDebug() << "Glew error "
                 << QString::fromUtf8(
                        reinterpret_cast<const char*>(errorStr), size);
    }

    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
        qWarning() << "Could not enable sample buffers";

    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    // we need a VAO in core!
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // We need us some vertex data. Start simple with a triangle ;-)
    float points[] = { -0.1f, -0.1f, 0.0f, 1.0f,
                        0.1f, -0.1f, 0.0f, 1.0f,
                        0.0f,  0.1f, 0.0f, 1.0f,
                       -0.9f, 0.9f, 0.0f, 1.0f,
                       -0.8f, 0.9f, 0.0f, 1.0f,
                        0.0f,  -0.5f, 0.0f, 1.0f};

    //data = loadSTLFile("/home/brian/CSC3020H_assignment-2/bunny.stl");

    data = GLWidget::stlData();
    data.numTriangles = 12;
    data.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[12*3]);
    data.vertices[0] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[1] = glm::vec4(-1.0f,-1.0f,1.0f,1.0f);
    data.vertices[2] = glm::vec4(-1.0f,1.0f,1.0f,1.0f);
    data.vertices[3] = glm::vec4(1.0f,1.0f,-1.0f,1.0f);
    data.vertices[4] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[5] = glm::vec4(-1.0f,1.0f,-1.0f,1.0f);
    data.vertices[6] = glm::vec4(1.0f,-1.0f,1.0f,1.0f);
    data.vertices[7] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[8] = glm::vec4(1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[9] = glm::vec4(1.0f,1.0f,-1.0f,1.0f);
    data.vertices[10] = glm::vec4(1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[11] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[12] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[13] = glm::vec4(-1.0f,1.0f,1.0f,1.0f);
    data.vertices[14] = glm::vec4(-1.0f,1.0f,-1.0f,1.0f);
    data.vertices[15] = glm::vec4(1.0f,-1.0f,1.0f,1.0f);
    data.vertices[16] = glm::vec4(-1.0f,-1.0f,1.0f,1.0f);
    data.vertices[17] = glm::vec4(-1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[18] = glm::vec4(-1.0f,1.0f,1.0f,1.0f);
    data.vertices[19] = glm::vec4(-1.0f,-1.0f,1.0f,1.0f);
    data.vertices[20] = glm::vec4(1.0f,-1.0f,1.0f,1.0f);
    data.vertices[21] = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    data.vertices[22] = glm::vec4(1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[23] = glm::vec4(1.0f,1.0f,-1.0f,1.0f);
    data.vertices[24] = glm::vec4(1.0f,-1.0f,-1.0f,1.0f);
    data.vertices[25] = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    data.vertices[26] = glm::vec4(1.0f,-1.0f,1.0f,1.0f);
    data.vertices[27] = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    data.vertices[28] = glm::vec4(1.0f,1.0f,-1.0f,1.0f);
    data.vertices[29] = glm::vec4(-1.0f,1.0f,-1.0f,1.0f);
    data.vertices[30] = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    data.vertices[31] = glm::vec4(-1.0f,1.0f,-1.0f,1.0f);
    data.vertices[32] = glm::vec4(-1.0f,1.0f,1.0f,1.0f);
    data.vertices[33] = glm::vec4(1.0f,1.0f,1.0f,1.0f);
    data.vertices[34] = glm::vec4(-1.0f,1.0f,1.0f,1.0f);
    data.vertices[35] = glm::vec4(1.0f,-1.0f,1.0f,1.0f);

    qDebug() << "GETS HERE->";
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit to 100 units
    Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    // Camera Matrix
    View = glm::lookAt(glm::vec3(0,0,5),glm::vec3(0,0,0), glm::vec3(0,1,0));
    Model = glm::mat4(1.0f);
    MVP = Projection * View * zRotMat * yRotMat * xRotMat * Model;

    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vertexBuffer.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_vertexBuffer.allocate( data.vertices.get(), data.numTriangles * 3 * sizeof(glm::vec4) );

    qDebug() << "Attempting vertex shader load from " << VERT_SHADER;
    qDebug() << "Attempting fragment shader load from " << FRAG_SHADER;

    // Prepare a complete shader program...
    if ( !prepareShaderProgram( VERT_SHADER, FRAG_SHADER) )
        std::runtime_error("Failed to load shader");
    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !m_shader.bind() )
    {
        qWarning() << "Could not bind shader program to context";
        return;
    }
    // Enable the "vertex" attribute to bind it to our currently bound
    // vertex buffer.
    m_shader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 4 );
    m_shader.enableAttributeArray( "vertex" );
    color = std::move(glm::vec4(1.0f,1.0f,1.0f,1.0f));
    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    xRotMat = std::move(glm::mat4(1.0f));
    yRotMat = std::move(glm::mat4(1.0f));
    zRotMat = std::move(glm::mat4(1.0f));
    scaleMat = std::move(glm::mat4(1.0f));
}

GLWidget::stlData GLWidget::loadSTLFile(const std::string & fileName) {
    std::ifstream fileStream;
    fileStream.open(fileName, std::ios_base::in | std::ios::binary);
    if(fileStream.fail())
    {
       qWarning() << "An error occurred accessing the file does the file exist?";
       fileStream.close();
       return std::move(GLWidget::stlData());
    }

    //DELETE THIS
    char header[80];
    fileStream.read((char *)&header, 80);
    //for(int i=0; i < 80; i++){
  //      qDebug() << header[i];
  //  }

    // Number of triangles
    unsigned int numTriangles;
    fileStream.read((char *)&numTriangles, 4);
    if(fileStream.fail())
    {
       qWarning() << "An error occurred reading from file";
       fileStream.close();
       return std::move(GLWidget::stlData());
    }

    GLWidget::stlData model;
    model.numTriangles = numTriangles;
    qDebug() << "numTrianles " << numTriangles;
    model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[numTriangles]);
    model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[numTriangles * 3]);
    for(unsigned int i = 0; i < numTriangles; ++i) {
        float normX, normY, normZ, x, y, z;
     //   qDebug() << "TEST0";
        fileStream.read((char*)&normX,4);

     //   qDebug() << "TEST1" << normX;
        fileStream.read((char*)&normY,4);
      //  qDebug() << "TEST2" << normY;
        fileStream.read((char*)&normZ,4);
      //  qDebug() << "TEST3" << normZ;
        fileStream.read((char*)&x,4);
     //   qDebug() << "TEST4" << x;
        fileStream.read((char*)&y,4);
    //    qDebug() << "TEST5" << y;
        fileStream.read((char*)&z,4);
    //    qDebug() << "TEST6" << z;
        if(fileStream.fail())
        {
           qWarning() << "An error occurred reading from file";
           fileStream.close();
           return std::move(GLWidget::stlData());
        }

        // Normal Vector
        model.normals[i] = glm::vec3(normX,normY,normZ);
        //qDebug() << "x1" << x;
//        qDebug() << "y1" << y;
       // qDebug() << "z1" << z;
        // Vertex 1
        x=x*10.0f;
        y=y*10.0f;
        z=z*10.0f;
        model.vertices[i] = glm::vec4(x,y,z,1.0f);

        // Vertex 2
        x=0.0f;y=0.0f;z=0.0f;
        fileStream.read((char*)&x,4);
        fileStream.read((char*)&y,4);
        fileStream.read((char*)&z,4);
        if(fileStream.fail())
        {
           qWarning() << "An error occurred reading from file";
           fileStream.close();
           return std::move(GLWidget::stlData());
        }
        x=x*10.0f;
        y=y*10.0f;
        z=z*10.0f;
        model.vertices[i+1] = glm::vec4(x,y,z,1.0f);
        //qDebug() << "x2" << x;
      //  qDebug() << "y2" << y;
      //  qDebug() << "z2" << z;
        // Vertex 3
        x=0.0f;y=0.0f;z=0.0f;
        fileStream.read((char*)&x,4);
        fileStream.read((char*)&y,4);
        fileStream.read((char*)&z,4);
        if(fileStream.fail())
        {
           qWarning() << "An error occurred reading from file";
           fileStream.close();
           return std::move(GLWidget::stlData());
        }
   //     qDebug() << "x3" << x;
   //     qDebug() << "y3" << y;
   //     qDebug() << "z3" << z;
        x=x*10.0f;
        y=y*10.0f;
        z=z*10.0f;
        model.vertices[i+2] = glm::vec4(x,y,z,1.0f);

        // Attribute byte count
        //fileStream.ignore(2,EOF);

        //TEST
        unsigned short test2 =-1;
        fileStream.read((char*)&test2,2);
    }
    return std::move(model);
}

void GLWidget::resizeGL( int w, int h )
{
    width = w;
    height = h;
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLWidget::paintGL()
{
    MVP = Projection * View * scaleMat * zRotMat * yRotMat * xRotMat * Model;
    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform4f(glGetUniformLocation(m_shader.programId(),"fcolor"),color[0],color[1],color[2],color[3]);
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Draw stuff
    glDrawArrays( GL_TRIANGLES, 0, data.numTriangles * 3);
    /*
    for(int i=0; i<data.numTriangles * 3; i+=3) {
        glDrawArrays( GL_LINE_LOOP, i, 3);
    }
    */
    //glDrawArrays( GL_LINES, 0, data.numTriangles * 3);
}
void GLWidget::mouseMoveEvent(QMouseEvent* e) {
    if(mode==0 || mode == 1) {
        int x = (width/2) - e->x();
        int y = (height/2) - e->y();
        int dx = lastX - x;
        int dy = lastY - y;
        if(dx>5 || dx < -5) {
            dx = 0;
        }
        if(dy > 5 || dy < -5) {
            dy = 0;
        }
        lastX = x;
        lastY = y;
    if(mode == 0) {
        if(rotMode == 0) { //x
            if(dy != 0) {
                xAngle -= rotateSpeed * dy;
                if(xAngle > 360.0f) {
                    xAngle -= 360.0f;
                }
                if(xAngle < -360.0f) {
                    xAngle += 360.0f;
                }
                glm::vec4 xRot(1.0f,0.0f,0.0f,0.0f);
                glm::vec4 yRot(0.0f,cosf(xAngle),-sinf(xAngle),0.0f);
                glm::vec4 zRot(0.0f,sinf(xAngle),cosf(xAngle),0.0f);
                glm::vec4 wRot(0.0f,0.0f,0.0f,1.0f);
                xRotMat = glm::mat4(xRot,yRot,zRot,wRot);
                repaint();
            }
        }
        else if(rotMode == 1) { //y
            if(dx != 0) {
                yAngle -= rotateSpeed * dx;
                if(yAngle > 360.0f) {
                    yAngle -= 360.0f;
                }
                if(yAngle < -360.0f) {
                    yAngle += 360.0f;
                }
                glm::vec4 xRot(cosf(yAngle),0.0f,sinf(yAngle),0.0f);
                glm::vec4 yRot(0.0f,1.0f,0.0f,0.0f);
                glm::vec4 zRot(-sinf(yAngle),0.0f,cosf(yAngle),0.0f);
                glm::vec4 wRot(0.0f,0.0f,0.0f,1.0f);
                yRotMat = glm::mat4(xRot,yRot,zRot,wRot);
                repaint();
            }
        }
        else if(rotMode == 2) { //z
            if(dy != 0) {
                zAngle -= rotateSpeed * dy;
                if(zAngle > 360.0f) {
                    zAngle -= 360.0f;
                }
                if(zAngle < -360.0f) {
                    zAngle += 360.0f;
                }
                glm::vec4 xRot(cosf(zAngle),-sinf(zAngle),0.0f,0.0f);
                glm::vec4 yRot(sinf(zAngle),cosf(zAngle),0.0f,0.0f);
                glm::vec4 zRot(0.0f,0.0f,1.0f,0.0f);
                glm::vec4 wRot(0.0f,0.0f,0.0f,1.0f);
                zRotMat = glm::mat4(xRot,yRot,zRot,wRot);
                repaint();
            }
        }
    }
    if(mode == 1) {
    // Up -> increase scale
    // Down -> decrease scale
        if(dy != 0) {
            scale -= dy * scaleSpeed;
            if(scale < 0.0f) {
                scale = 0.0f;
            }
            glm::vec4 scaleX(scale,0.0f,0.0f,0.0f);
            glm::vec4 scaleY(0.0f,scale,0.0f,0.0f);
            glm::vec4 scaleZ(0.0f,0.0f,scale,0.0f);
            glm::vec4 scaleW(0.0f,0.0f,0.0f,1.0f);
            scaleMat = glm::mat4(scaleX,scaleY,scaleZ,scaleW);
            repaint();
        }

    }
    }
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_R:
            lastX=20000;
            lastY=20000;
            mode = 0;
            rotMode += 1;
            if(rotMode >= 3) {
                rotMode = 0;
            }
            if(rotMode==0) {
                qDebug() << "x rotate mode";
            }
            if(rotMode==1) {
                qDebug() << "y rotate mode";
            }
            if(rotMode==2) {
                qDebug() << "z rotate mode";
            }
            break;
        case Qt::Key_S:
            qDebug() << "scale mode";
            lastX=20000;
            lastY=20000;
            mode = 1;
            break;
        case Qt::Key_1:
            color = std::move(glm::vec4(1.0f,1.0f,1.0f,1.0f));
            repaint();
            break;
        case Qt::Key_2:
            color = std::move(glm::vec4(1.0f,0.0f,0.0f,1.0f));
            repaint();
            break;
        case Qt::Key_3:
            color = std::move(glm::vec4(0.0f,1.0f,0.0f,1.0f));
            repaint();
            break;
        case Qt::Key_4:
            color = std::move(glm::vec4(0.0f,0.0f,1.0f,1.0f));
            repaint();
            break;
        case Qt::Key_5:
            color = std::move(glm::vec4(0.8f,0.72f,0.878f,1.0f));
            repaint();
            break;
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;

        default:
            QGLWidget::keyPressEvent( e );
    }
}

bool GLWidget::prepareShaderProgram( const QString& vertexShaderPath,
                                     const QString& fragmentShaderPath )
{
    // First we load and compile the vertex shader...
    bool result = m_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...now the fragment shader...
    result = m_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...and finally we link them to resolve any references.
    result = m_shader.link();
    if ( !result )
        qWarning() << "Could not link shader program:" << m_shader.log();

    return result;
}
