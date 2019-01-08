#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
  xClick = yClick = 0;
  DoingInteractive = NONE;
}

MyGLWidget::~MyGLWidget ()
{
  if (program != NULL)
    delete program;
}

void MyGLWidget::initializeGL ()
{
  // Cal inicialitzar l'ús de les funcions d'OpenGL
  initializeOpenGLFunctions();

  glClearColor (0.5, 0.7, 1.0, 1.0); // defineix color de fons (d'esborrat)
  glEnable (GL_DEPTH_TEST);
  real_light = 1.0f;
  whiteLight = 0.0f;
  carregaShaders();
  createBuffersPatricio();
  createBuffersMoo();
  createBuffersTerraIParet();
  ra = 1.0;
  FOV = FOV_INI =2*asin(1.0 /2.0 );
  angleYPatrMoo = 0;
  emit zoom_changed(int(FOV*100/M_PI));
  vrp = glm::vec3(0, 1, 0);
  iniEscena();
  iniCamera();
}

void MyGLWidget::iniEscena ()
{
  radiEsc = glm::distance(glm::vec3(-2.0, -1.0, -2.0), glm::vec3(2.0, 3.0, 2.0))/2;
}

void MyGLWidget::iniCamera ()
{
  angleY = 0.0;
  angleX = 0.0;
  perspectiva = true;

  projectTransform ();
  viewTransform ();
}

void MyGLWidget::paintGL ()
{
  glUniform1f(pintaMooLoc, 0.0f);

  glViewport (0, 0, width(), height()); // Aquesta crida no caldria perquè Qt la fa de forma automàtica amb aquests paràmetres

  // Esborrem el frame-buffer i el depth-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Activem el VAO per a pintar el terra
  glBindVertexArray (VAO_Terra);

  modelTransformTerra ();

  // pintem
  glDrawArrays(GL_TRIANGLES, 0, 12);

  // Activem el VAO per a pintar el Patricio
  glBindVertexArray (VAO_Patr);

  modelTransformPatricio1();

  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, patr.faces().size()*3);

  glUniform1f(pintaMooLoc, 1.0f);

  glBindVertexArray (VAO_Moo);

  modelTransformMoo();

  glDrawArrays(GL_TRIANGLES, 0 , moo.faces().size()*3);

  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h)
{
    float rViewport = float(w)/float(h);
    ra = rViewport;
    if(rViewport < 1.0){
        FOV = 2.0 * atan(tan(FOV_INI/2)/rViewport);
    }
    else FOV_INI = FOV;
    projectTransform();
    glViewport(0,0,w,h);
  // Aquí anirà el codi que cal fer quan es redimensiona la finestra
}

void MyGLWidget::createBuffersPatricio ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  patr.load("./models/Patricio.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModel ();

  // Creació del Vertex Array Object del Patricio
  glGenVertexArrays(1, &VAO_Patr);
  glBindVertexArray(VAO_Patr);

  // Creació dels buffers del model patr
  GLuint VBO_Patr[6];
  // Buffer de posicions
  glGenBuffers(6, VBO_Patr);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Patr[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3, patr.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}


void MyGLWidget::createBuffersMoo ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  moo.load("./models/cow.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModelMoo ();

  // Creació del Vertex Array Object del Patricio
  glGenVertexArrays(1, &VAO_Moo);
  glBindVertexArray(VAO_Moo);

  // Creació dels buffers del model patr
  GLuint VBO_Moo[6];
  // Buffer de posicions
  glGenBuffers(6, VBO_Moo);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*moo.faces().size()*3*3, moo.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*moo.faces().size()*3*3, moo.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*moo.faces().size()*3*3, moo.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*moo.faces().size()*3*3, moo.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Moo[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*moo.faces().size()*3, moo.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}


void MyGLWidget::createBuffersTerraIParet ()
{
  // VBO amb la posició dels vèrtexs
  glm::vec3 posterra[12] = {
	glm::vec3(-2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(-2.0, 1.0, -2.0),
	glm::vec3(-2.0, 1.0, -2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(2.0, 1.0, -2.0)
  };

  // VBO amb la normal de cada vèrtex
  glm::vec3 norm1 (0,1,0);
  glm::vec3 norm2 (0,0,1);
  glm::vec3 normterra[12] = {
	norm1, norm1, norm1, norm1, norm1, norm1, // la normal (0,1,0) per als primers dos triangles
	norm2, norm2, norm2, norm2, norm2, norm2  // la normal (0,0,1) per als dos últims triangles
  };

  // Definim el material del terra
  glm::vec3 amb(0.2,0,0.2);
  glm::vec3 diff(0.8,0,0.8);
  glm::vec3 spec(0,0,0);
  float shin = 100;

  // Fem que aquest material afecti a tots els vèrtexs per igual
  glm::vec3 matambterra[12] = {
	amb, amb, amb, amb, amb, amb, amb, amb, amb, amb, amb, amb
  };
  glm::vec3 matdiffterra[12] = {
	diff, diff, diff, diff, diff, diff, diff, diff, diff, diff, diff, diff
  };
  glm::vec3 matspecterra[12] = {
	spec, spec, spec, spec, spec, spec, spec, spec, spec, spec, spec, spec
  };
  float matshinterra[12] = {
	shin, shin, shin, shin, shin, shin, shin, shin, shin, shin, shin, shin
  };

// Creació del Vertex Array Object del terra
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  GLuint VBO_Terra[6];
  glGenBuffers(6, VBO_Terra);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posterra), posterra, GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normterra), normterra, GL_STATIC_DRAW);

  // Activem l'atribut normalLoc
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[2]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matambterra), matambterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[3]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matdiffterra), matdiffterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[4]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matspecterra), matspecterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glBindBuffer(GL_ARRAY_BUFFER, VBO_Terra[5]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matshinterra), matshinterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void MyGLWidget::carregaShaders()
{
  // Creem els shaders per al fragment shader i el vertex shader
  QOpenGLShader fs (QOpenGLShader::Fragment, this);
  QOpenGLShader vs (QOpenGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem
  fs.compileSourceFile("shaders/fragshad.frag");
  vs.compileSourceFile("shaders/vertshad.vert");
  // Creem el program
  program = new QOpenGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);
  // Linkem el program
  program->link();
  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  // Obtenim identificador per a l'atribut “normal” del vertex shader
  normalLoc = glGetAttribLocation (program->programId(), "normal");
  // Obtenim identificador per a l'atribut “matamb” del vertex shader
  matambLoc = glGetAttribLocation (program->programId(), "matamb");
  // Obtenim identificador per a l'atribut “matdiff” del vertex shader
  matdiffLoc = glGetAttribLocation (program->programId(), "matdiff");
  // Obtenim identificador per a l'atribut “matspec” del vertex shader
  matspecLoc = glGetAttribLocation (program->programId(), "matspec");
  // Obtenim identificador per a l'atribut “matshin” del vertex shader
  matshinLoc = glGetAttribLocation (program->programId(), "matshin");

  // Demanem identificadors per als uniforms del vertex shader
  transLoc = glGetUniformLocation (program->programId(), "TG");
  projLoc = glGetUniformLocation (program->programId(), "proj");
  viewLoc = glGetUniformLocation (program->programId(), "view");
  pintaMooLoc = glGetUniformLocation (program->programId(), "printMoo");
  realLightLoc = glGetUniformLocation(program->programId(), "real_light");
  whiteLightLoc = glGetUniformLocation(program->programId(), "whiteLight");
  glUniform1f(realLightLoc, real_light);
  glUniform1f(whiteLightLoc, whiteLight);
}

glm::mat4 MyGLWidget::TGPatricio () {
  glm::mat4 TG = glm::mat4(1.f);  // Matriu de transformació
  TG = glm::rotate(TG, angleYPatrMoo, glm::vec3(0, 1.0, 0));
  TG = glm::translate(TG, glm::vec3(1.0, -0.5+0.125, 0.0));
  TG = glm::scale(TG, glm::vec3(escala, escala, escala));
  TG = glm::translate(TG, -centrePatr);
  return TG;
}

void MyGLWidget::modelTransformPatricio1 ()
{
  glm::mat4 TG = TGPatricio();
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformMoo ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  TG = glm::rotate(TG, angleYPatrMoo, glm::vec3(0, 1.0, 0));
  TG = glm::translate(TG, glm::vec3(1.0, -0.75, 0.0));
  TG = glm::rotate(TG, float(-M_PI/2.0), glm::vec3(1.0, 0.0, 0.0));
  TG = glm::scale(TG, glm::vec3(escalaMoo, escalaMoo, escalaMoo));
  TG = glm::translate(TG, -centreMoo);

  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformTerra ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::projectTransform ()
{
  glm::mat4 Proj;  // Matriu de projecció
  if (perspectiva)
    Proj = glm::perspective(float(M_PI/3.0), ra, 0.1f, 2.0f*radiEsc);
  else
    Proj = glm::ortho(-radiEsc, radiEsc, -radiEsc, radiEsc, radiEsc, 3.0f*radiEsc);

  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform ()
{
  //no funciona esto no sé por qué
  // glm::mat4 TGP = TGPatricio();
  // glm::mat4 View(1.f);
  // View = View * TGP * glm::vec4(centrePatr, 1);  // Matriu de posició i orientació
  // hasta acá, ahora viene lo viejo
  // View = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2*radiEsc));
  // View = glm::rotate(View, -angleX, glm::vec3(1, 0, 0));
  // View = glm::rotate(View, -angleY, glm::vec3(0, 1, 0));
  // View = glm::translate(View, -vrp);
  glm::vec4 posicion = glm::vec4(glm::vec3(1,-0.375,0), 1.0f); // posición del centro de Patricio calculada a mano
  // ***** APLICAMOS UNA TRANSFORMACION GEOMETRICA A LA POSICION A LA QUE MIRAMOS *****
  posicion = glm::rotate(glm::mat4(1.f), angleYPatrMoo, glm::vec3(0,1,0))*posicion;
  glm::mat4 View = glm::lookAt(glm::vec3(-1, 1, -1), glm::vec3(posicion), glm::vec3(0, 1, 0));
  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::calculaCapsaModel ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = patr.vertices()[0];
  miny = maxy = patr.vertices()[1];
  minz = maxz = patr.vertices()[2];
  for (unsigned int i = 3; i < patr.vertices().size(); i+=3)
  {
    if (patr.vertices()[i+0] < minx)
      minx = patr.vertices()[i+0];
    if (patr.vertices()[i+0] > maxx)
      maxx = patr.vertices()[i+0];
    if (patr.vertices()[i+1] < miny)
      miny = patr.vertices()[i+1];
    if (patr.vertices()[i+1] > maxy)
      maxy = patr.vertices()[i+1];
    if (patr.vertices()[i+2] < minz)
      minz = patr.vertices()[i+2];
    if (patr.vertices()[i+2] > maxz)
      maxz = patr.vertices()[i+2];
  }
  escala = 0.25/(maxy-miny);
  centrePatr[0] = (minx+maxx)/2.0; centrePatr[1] = (miny+maxy)/2.0; centrePatr[2] = (minz+maxz)/2.0;
}


void MyGLWidget::calculaCapsaModelMoo ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = moo.vertices()[0];
  miny = maxy = moo.vertices()[1];
  minz = maxz = moo.vertices()[2];
  for (unsigned int i = 3; i < moo.vertices().size(); i+=3)
  {
    if (moo.vertices()[i+0] < minx)
      minx = moo.vertices()[i+0];
    if (moo.vertices()[i+0] > maxx)
      maxx = moo.vertices()[i+0];
    if (moo.vertices()[i+1] < miny)
      miny = moo.vertices()[i+1];
    if (moo.vertices()[i+1] > maxy)
      maxy = moo.vertices()[i+1];
    if (moo.vertices()[i+2] < minz)
      minz = moo.vertices()[i+2];
    if (moo.vertices()[i+2] > maxz)
      maxz = moo.vertices()[i+2];
  }
  escalaMoo = 0.5/(maxz-minz);
  centreMoo[0] = (minx+maxx)/2.0; centreMoo[1] = (miny+maxy)/2.0; centreMoo[2] = (minz+maxz)/2.0;
}

void MyGLWidget::keyPressEvent(QKeyEvent* event)
{
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_O: { // canvia òptica entre perspectiva i axonomètrica
      perspectiva = !perspectiva;
      projectTransform ();
      break;
    }
    case Qt::Key_R: { // canvia òptica entre perspectiva i axonomètrica
      angleYPatrMoo = angleYPatrMoo + M_PI/6.0;
      projectTransform ();
      viewTransform();
      break;
    }
    case Qt::Key_X: { // canvia òptica entre perspectiva i axonomètrica
      if(real_light == 0) {
        real_light = 1.0f;
      } else {
        real_light = 0.0f;
      }
      glUniform1f(realLightLoc, real_light);
      break;
    }
    case Qt::Key_L: { // canvia òptica entre perspectiva i axonomètrica
      if(whiteLight == 0) {
        whiteLight = 1.0f;
      } else {
        whiteLight = 0.0f;
      }
      glUniform1f(whiteLightLoc, whiteLight);
      break;
    }
    default: event->ignore(); break;
  }
  update();
}

void MyGLWidget::mousePressEvent (QMouseEvent *e)
{
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE;
  } else if (e->button() & Qt::RightButton) {
    DoingInteractive = ZOOM;
  }
}

void MyGLWidget::mouseReleaseEvent( QMouseEvent *)
{
  DoingInteractive = NONE;
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e)
{
  makeCurrent();
  // Aqui cal que es calculi i s'apliqui la rotacio o el zoom com s'escaigui...
  if (DoingInteractive == ROTATE)
  {
    // Fem la rotació
    angleY += (e->x() - xClick) * M_PI / 180.0;
    angleX += (e->y() - yClick) * M_PI / 180.0;
    viewTransform ();
  } else if (DoingInteractive == ZOOM) {
    //std::cout << "entre";
    FOV += (e->y() - yClick) * M_PI / 180.0;
    if (FOV > M_PI) {
        FOV = M_PI - 0.01;
    } else if (0 > FOV) {
        FOV = 0.01;
    }
    emit zoom_changed(int(FOV*100/M_PI));
    projectTransform();
  }

  xClick = e->x();
  yClick = e->y();

  update ();
}

void MyGLWidget::change_zoom(int new_value) {
    makeCurrent();
    FOV = new_value * M_PI / 100.0;
    if (FOV > M_PI) {
        FOV = M_PI - 0.01;
    } else if (0 > FOV) {
        FOV = 0.01;
    }
    projectTransform();
    update();
}