#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QWidget* parent) : QOpenGLWidget(parent), program(NULL)
{
  setFocusPolicy(Qt::StrongFocus);  // per rebre events de teclat
  xClick = yClick = 0; // ***** PARA INTERACTUAR CON EL MOUSE *****
  DoingInteractive = NONE;
  ra = 1.0f; // ***** INICIALIZAMOS RA, SI NO NO SE VERA *****
  rotacionAdicional = 0.0; // ***** ININCIALIZAMOS LA ROTACION *****
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

  carregaShaders();
  createBuffersPatricio();
  createBuffersTerraIParet();
  // ***** CARGAMOS LOS BUFFERS DE LA VACA *****
  createBuffersVaca();

  iniEscena();
  iniCamera();
}

void MyGLWidget::iniEscena ()
{
  // ***** CALCULAMOS A MANO EL RADIO SEGUN LA CAJA DE LA ESCENA
  radiEsc = glm::distance(glm::vec3(-2.0,-1.0,-2.0), glm::vec3(2.0,1,2.0))/2.0;
}

void MyGLWidget::iniCamera ()
{
  angleY = 0.0;
  angleX = 0.0;
  perspectiva = true;

  // ***** CALCULAMOS MANUALMENTE EL CENTRO DE LA ESCENA *****
  vrp = glm::vec3(0,0,0); // ***** RECORDAMOS QUE EL SUELO ESTA EN LA POS -1 *****
  obs = glm::vec3(0,2,2.0*radiEsc);
  FOV = 2.0*asin(radiEsc/(2.0*radiEsc));
  FOV_INI = FOV;
  // ***** ACTUALIZAMOS EL VAOR DEL SLIDER, HAY QUE PASAR DE RAD A GRADOS E INTEGER, IMPORTANTE PARA QUE NO DE TIRONES *****
  updateFOVSlider((int)(FOV/(float)M_PI*180.0));
  projectTransform ();
  viewTransform ();
}

void MyGLWidget::paintGL () 
{
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

  modelTransformPatricio ();

  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, patr.faces().size()*3);

  // ***** PINTAMOS LA VACA *****
  glBindVertexArray (VAO_Vaca);
  modelTransformVaca();
  glDrawArrays(GL_TRIANGLES, 0, vaca.faces().size()*3);

  // ***** DESACIVAMOS VAO *****
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h) 
{
  // ***** IGUALAMOS RA A RAVIEWPORT Y SI RA < 1 RECALCULAMOS FOV *****
  float rViewport = float(w)/float(h);
  ra = rViewport;
  if(rViewport < 1.0){
      FOV = 2.0 * atan(tan(FOV_INI/2)/rViewport);
  }
  else FOV_INI = FOV;
  projectTransform();
  glViewport(0,0,w,h);
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

// ***** CARGAMOS LA VACA *****
void MyGLWidget::createBuffersVaca(){
    // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
    vaca.load("./models/cow.obj");

    // ***** CALCULAMOS LA CAJA DE LA VACA *****
    calculaCapsaModelVaca ();

    // Creació del Vertex Array Object del Patricio
    glGenVertexArrays(1, &VAO_Vaca);
    glBindVertexArray(VAO_Vaca);

    // Creació dels buffers del model patr
    GLuint VBO_Vaca[6];
    // Buffer de posicions
    glGenBuffers(6, VBO_Vaca);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_vertices(), GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // Buffer de normals
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_normals(), GL_STATIC_DRAW);

    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    // En lloc del color, ara passem tots els paràmetres dels materials
    // Buffer de component ambient
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matamb(), GL_STATIC_DRAW);

    glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matambLoc);

    // Buffer de component difusa
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[3]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matdiff(), GL_STATIC_DRAW);

    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    // Buffer de component especular
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3*3, vaca.VBO_matspec(), GL_STATIC_DRAW);

    glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matspecLoc);

    // Buffer de component shininness
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Vaca[5]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vaca.faces().size()*3, vaca.VBO_matshin(), GL_STATIC_DRAW);

    glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matshinLoc);

    glBindVertexArray(0);
}

void MyGLWidget::createBuffersTerraIParet ()
{
  // VBO amb la posició dels vèrtexs
  glm::vec3 posterra[12] = {
    // ***** TRIANGULO 1 *****
	glm::vec3(-2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(-2.0, -1.0, -2.0),
    // ***** TRIANGULO 2 *****
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, 2.0),
	glm::vec3(2.0, -1.0, -2.0),
    // ***** TRIANGULO 3 *****
	glm::vec3(-2.0, -1.0, -2.0),
	glm::vec3(2.0, -1.0, -2.0),
	glm::vec3(-2.0, 1.0, -2.0),
    // ***** TRIANGULO 4 *****
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
  glm::vec3 amb(0.2,0,0.2); // ***** COMPONENTE AMBIENTE *****
  glm::vec3 diff(0.8,0,0.8); // ***** COMPONENTE DIFUSA *****
  glm::vec3 spec(0,0,0);// ***** COMPONENTE ESPECULAR *****
  float shin = 100;// ***** DUREZA DEL BRILLO ESPECULAR, A MAS BAJO MAS SUAVE *****

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
  // ***** BUSCA EN ALGUN SHADER SI HAY ALGUNA VARIABLE LLAMADA "LO QUE SEA" Y LO PONE EN
  // EN LA UNIFORM *****
  transLoc = glGetUniformLocation (program->programId(), "TG");
  projLoc = glGetUniformLocation (program->programId(), "proj");
  viewLoc = glGetUniformLocation (program->programId(), "view");
}

void MyGLWidget::modelTransformPatricio ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  // ***** ROTAMOS SOBRE EL EJE Y *****
  TG = glm::rotate(TG, rotacionAdicional, glm::vec3(0,1,0));
  TG = glm::translate(TG, glm::vec3(1,-0.375,0));
  TG = glm::scale(TG, glm::vec3(escala, escala, escala));
  TG = glm::translate(TG, -centrePatr);
  // ***** ENVIAMOS LA MATRIZ DE TRANSORMACION A LA GRÁFICA, USAMOS EL LOCALIZADOR TRANSLOC *****
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

// ***** TG DEL SEGUNDO PATRICIO *****
void MyGLWidget::modelTransformPatricio2 ()
{
  glm::mat4 TG(1.f);  // Matriu de transformació
  TG = glm::translate(TG, glm::vec3(0,2,0)); // ***** SUBIMOD EL PATRICIO *****
  TG = glm:: rotate(TG, (float)M_PI, glm::vec3(0,0,1)); // ***** LE DAMOS LA VUELTA *****
  TG = glm::scale(TG, glm::vec3(escala, escala, escala));
  TG = glm::translate(TG, -centrePatr);
  // ***** ENVIAMOS LA MATRIZ DE TRANSORMACION A LA GRÁFICA, USAMOS EL LOCALIZADOR TRANSLOC *****
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

// ***** TG VACA *****
void MyGLWidget::modelTransformVaca(){
    glm::mat4 TG(1.f);  // Matriu de transformació
    // ***** ROTAMOS SOBRE EL EJE Y *****
    TG = glm::rotate(TG, rotacionAdicional, glm::vec3(0,1,0));
    TG = glm::translate(TG, glm::vec3(1,-0.75,0)); // ***** MOVEMOS LA VACA A SU SITIO *****
    TG = glm:: rotate(TG, (float)M_PI/-2.0f, glm::vec3(1,0,0)); // ***** LE DAMOS LA VUELTA *****
    TG = glm::scale(TG, glm::vec3(escalaVaca, escalaVaca, escalaVaca));
    TG = glm::translate(TG, -centroVaca);
    // ***** ENVIAMOS LA MATRIZ DE TRANSORMACION A LA GRÁFICA, USAMOS EL LOCALIZADOR TRANSLOC *****
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
    Proj = glm::perspective(FOV, ra, radiEsc, 3.0f*radiEsc);
  else{
    // ***** HAGO LA SUCIA Y PASO DE PRESPECTIVA A ORTOGONAL PARA REAPROVECHAR EL RESIZE *****
    float height = tan(FOV/2.0)*(3.0f*radiEsc);
    Proj = glm::ortho(-height*ra, height*ra, -height, height, radiEsc, 3.0f*radiEsc);
  }

  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform ()
{
  glm::mat4 View;  // Matriu de posició i orientació
  // ***** AÑADIMOS -D (ver formula de angulos de euler) *****
  View = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, -2.0*radiEsc));
  // ***** AÑADIMOS LA ROTACION EN X *****
  View = glm::rotate(View, angleX, glm::vec3(1,0,0));
  View = glm::rotate(View, -angleY, glm::vec3(0, 1, 0));
  View = glm::translate(View,-vrp);

  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}

void MyGLWidget::calculaCapsaModel ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = patr.vertices()[0];
  miny = maxy = patr.vertices()[1];
  minz = maxz = patr.vertices()[2];

  // ***** ITERAMOS POR TODOS LOS VERTICES PARA ENCONTRAR EL MAXIMO Y MINIMO ****
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

  // ***** A QUE TAMAÑO EN Y QUEREMOS EL PATRICIO, EN ESTE CASO 2 *****
  escala = 0.25/(maxy-miny);
  // ***** ENCONTRAMOS EL CENTRO  DEL PATRICIO *****
  centrePatr[0] = (minx+maxx)/2.0; centrePatr[1] = (miny+maxy)/2.0; centrePatr[2] = (minz+maxz)/2.0;
}

// ***** CALCULAMO LA CAJA DE LA VACA *****
void MyGLWidget::calculaCapsaModelVaca(){

    // Càlcul capsa contenidora i valors transformacions inicials
    float minx, miny, minz, maxx, maxy, maxz;
    minx = maxx = vaca.vertices()[0];
    miny = maxy = vaca.vertices()[1];
    minz = maxz = vaca.vertices()[2];

    // ***** ITERAMOS POR TODOS LOS VERTICES PARA ENCONTRAR EL MAXIMO Y MINIMO ****
    for (unsigned int i = 3; i < vaca.vertices().size(); i+=3)
    {
      if (vaca.vertices()[i+0] < minx)
        minx = vaca.vertices()[i+0];
      if (vaca.vertices()[i+0] > maxx)
        maxx = vaca.vertices()[i+0];
      if (vaca.vertices()[i+1] < miny)
        miny = vaca.vertices()[i+1];
      if (vaca.vertices()[i+1] > maxy)
        maxy = vaca.vertices()[i+1];
      if (vaca.vertices()[i+2] < minz)
        minz = vaca.vertices()[i+2];
      if (vaca.vertices()[i+2] > maxz)
        maxz = vaca.vertices()[i+2];
    }

    // ***** A QUE TAMAÑO EN Y QUEREMOS LA VACA, EN ESTE CASO 0.5, OJO LA VACA ESTA TUMBADA *****
    escalaVaca = 0.5/(maxz-minz);
    // ***** ENCONTRAMOS EL CENTRO  DE LA VACA *****
    centroVaca[0] = (minx+maxx)/2.0; centroVaca[1] = (miny+maxy)/2.0; centroVaca[2] = (minz+maxz)/2.0;
}

void MyGLWidget::keyPressEvent(QKeyEvent* event) 
{
  // ***** NECESARIO PARA TRATAR LAS TECLAS *****
  makeCurrent();
  switch (event->key()) {
    case Qt::Key_O: { // canvia òptica entre perspectiva i axonomètrica
      perspectiva = !perspectiva;
      // ***** ES NECESARIO RECALCULAR LA PROJECT MATRIX *****
      projectTransform ();
      break;
    }
    case Qt::Key_R: { // ***** AÑADIMOS LA ROTACION *****
      rotacionAdicional += (float)M_PI/6.0f;
      break;
    }
    default: event->ignore(); break;
  }
  // ***** LLAMA A PAINTGL *****
  update();
}

void MyGLWidget::mousePressEvent (QMouseEvent *e)
{
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE; // ***** PERMITIRA HACER LA ROTACION DE CAMARA *****
  }
  // ***** INDICAMOS QUE SE QUIERE HACER ZOOM *****
  else if (e->button() & Qt::RightButton &&
        ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
    {
      DoingInteractive = ZOOM; // ***** PERMITE HACER ZOOM *****
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
    //***** CALCULAMOS PARA EL EJE X *****
    angleX += (e->y() - yClick) * M_PI / 180.0;
    // ***** CADA VEZ QUE CAMBIA EL ANGULO CAMBIA EL OBSERVADOR POR LO QUE HAY QUE CALCULAR LA VIEW MATRIX *****
    viewTransform ();
  }

  // ***** TRATAMOS EL ZOOM *****
  else if( DoingInteractive == ZOOM){
      // ***** MIRAMOS QUE EL FOV NO SE PASE  DE 180 O SE VUELVA NEGATIVO
      if(FOV + (float)M_PI/32.0f * (e->x() - xClick)>= 0.0f and FOV + (float)M_PI/32.0f * (e->x() - xClick) <= (float)M_PI){
          FOV += (float)M_PI/32.0f * (e->x() - xClick);
          FOV_INI = FOV;
          // ***** IMPORTANTE LLAMAR A RECALCULAR LA PROJECT MATRIX *****
          projectTransform();

          // ***** ACTUALIZAMOS EL VAOR DEL SLIDER, HAY QUE PASAR DE RAD A GRADOS E INTEGER *****
          updateFOVSlider((int)(FOV/(float)M_PI*180.0));
      }
  }

  xClick = e->x();
  yClick = e->y();
  // ***** CADA VEZ QUE MOVEMOS EL MOUSE REPINTAMOS *****
  update ();
}

void MyGLWidget:: changeFOV(int value){
    makeCurrent();
    // ***** SE NOS PASA UN VALOR DE 0 A 180 HAY QUE CONVERTIRLO EN RADIANES *****
    float valueInRads = ((float)value/180.0f)*(float)M_PI;
    FOV = valueInRads;
    FOV_INI = FOV;
    // ***** RECALCULAMOS LA PROJECT MATRIX Y REPINTAMOS *****
    projectTransform();
    update();
}

