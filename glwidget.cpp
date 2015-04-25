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

    std::unique_ptr<glm::vec4[]> test = std::unique_ptr<glm::vec4[]>(new glm::vec4[6]);
    test[0] = glm::vec4(-0.1f, -0.1f, 0.0f, 1.0f);
    test[1] = glm::vec4(0.1f, -0.1f, 0.0f, 1.0f);
    test[2] = glm::vec4(0.0f,  0.1f, 0.0f, 1.0f);
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit to 100 units
    glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    // Camera Matrix
    glm::mat4 View = glm::lookAt(glm::vec3(0,0,-1),glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 Model = glm::mat4(1.0f);
    glm::mat4 MVP = Projection * View * Model;

    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vertexBuffer.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_vertexBuffer.allocate( test.get(), 6 * sizeof(glm::vec4) );

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
    glUniform4f(glGetUniformLocation(m_shader.programId(),"fcolor"),0.0f,1.0f,0.0f,1.0f);
    GLuint MatrixID = glGetUniformLocation(m_shader.programId(), "MVP");
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

}

GLWidget::stlData GLWidget::loadSTLFile(const std::string & fileName) {
    std::ifstream fileStream;
    fileStream.open(fileName.c_str(), std::ios_base::in | std::ios::binary);
    if(fileStream.fail())
    {
       std::cerr << "An error occurred accessing the file "<< fileName <<", does the file exist?" << std::endl;
       fileStream.close();
       return std::move(GLWidget::stlData());
    }
    // Ignore header data
    fileStream.ignore(80,EOF);

    unsigned int numTriangles;
    fileStream.read((char *)&numTriangles, 4);
    if(fileStream.fail())
    {
       std::cerr << "An error occurred reading from file, " << fileName << std::endl;
       fileStream.close();
       return std::move(GLWidget::stlData());
    }
    GLWidget::stlData model;
    model.normals = std::unique_ptr<glm::vec3[]>(new glm::vec3[1]);
    model.vertices = std::unique_ptr<glm::vec4[]>(new glm::vec4[1 * 3]);
    for(unsigned int i = 0; i < numTriangles; ++i) {
        float normX, normY, normZ, x, y, z;
        fileStream.read((char*)&normX,32);
        fileStream.read((char*)&normY,32);
        fileStream.read((char*)&normZ,32);
        fileStream.read((char*)&x,32);
        fileStream.read((char*)&y,32);
        fileStream.read((char*)&z,32);
        fileStream.ignore(2,EOF);
    }

}

void GLWidget::resizeGL( int w, int h )
{
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Draw stuff
    glDrawArrays( GL_TRIANGLES, 0, 6);
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
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
