#define GLM_FORCE_RADIANS
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMouseEvent>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "model.h"

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core 
{
  Q_OBJECT

  public:
    MyGLWidget (QWidget *parent=0);
    ~MyGLWidget ();
  public slots:
    void changeFOV(int);
  signals:
    void updateFOVSlider(int);

  protected:
    // initializeGL - Aqui incluim les inicialitzacions del contexte grafic.
    virtual void initializeGL ( );
    // paintGL - Mètode cridat cada cop que cal refrescar la finestra.
    // Tot el que es dibuixa es dibuixa aqui.
    virtual void paintGL ( );
    // resizeGL - És cridat quan canvia la mida del widget
    virtual void resizeGL (int width, int height);
    // keyPressEvent - Es cridat quan es prem una tecla
    virtual void keyPressEvent (QKeyEvent *event);
    // mouse{Press/Release/Move}Event - Són cridades quan es realitza l'event 
    // corresponent de ratolí
    virtual void mousePressEvent (QMouseEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void mouseMoveEvent (QMouseEvent *event);

  private:
    void createBuffersPatricio ();
    void createBuffersTerraIParet ();
    void carregaShaders ();
    void iniEscena ();
    void iniCamera ();
    void projectTransform ();
    void viewTransform ();
    void modelTransformTerra ();
    void modelTransformPatricio ();
    void calculaCapsaModel ();

    // ***** NECESITAMOS OTRA FUNCION PARA CALCULAR LA TG DEL OTRO PATRICIO *****
    void modelTransformPatricio2();

    // ***** FUNCIONES NECESARIAS PARA LA VACA *****
    void calculaCapsaModelVaca();
    void modelTransformVaca();
    void createBuffersVaca();

    // VAO i VBO names
    GLuint VAO_Patr;
    GLuint VAO_Terra;
    // Program
    QOpenGLShaderProgram *program;
    // uniform locations
    GLuint transLoc, projLoc, viewLoc;
    // attribute locations
    GLuint vertexLoc, normalLoc, matambLoc, matdiffLoc, matspecLoc, matshinLoc;

    // model
    Model patr;
    // paràmetres calculats a partir de la capsa contenidora del model
    glm::vec3 centrePatr;
    float escala;
    // radi de l'escena
    float radiEsc;

    typedef  enum {NONE, ROTATE, ZOOM} InteractiveAction; // ***** AÑADIMOS ZOOM A LA ENUMERACION *****
    InteractiveAction DoingInteractive;
    int xClick, yClick;
    float angleY, angleX; // ***** ANGULO X AÑADIDO *****
    bool perspectiva;

    // ***** PARA VISUALIZAR LA ESCENA CORRECTAMENTE *****
    float FOV, FOV_INI;
    glm::vec3 vrp;
    glm::vec3 obs;
    float ra;

    // ***** COSAS DE LA VACA *****
    Model vaca;
    glm::vec3 centroVaca;
    float escalaVaca;
    GLuint VAO_Vaca;

    // ***** PARA LA ROTACION CONJUNTA *****
    float rotacionAdicional;

    // ***** PARA LA SEGUNDA CAMARA *****
    bool secondCam;

    // ***** PARA EL MATERIAL DE LA VACA *****
    GLuint printVacaLoc;
};

