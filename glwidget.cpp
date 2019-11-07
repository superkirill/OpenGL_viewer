#include <QFile>
#include <QString>
#include <QTransform>
#include <QtGui>
#include <QtOpenGL>
#include <QOpenGLWidget>
#include <QVector3D>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <clocale>
#include "viewer_widget.h"
#include "glwidget.h"

#include <iostream>

static const float doublePi = float(M_PI);
static const float radiansToDegrees = 360.0f / doublePi;
static const bool DEBUG = false; // Change to true for view debug info
static const int NUM_COLOLORS = 8;

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
  x_translation = 0.0;
  y_translation = 0.0;
  z_translation = 0.0;
  scale = -0.0f;
  accelerated = false;
  acc_factor = 5.0;
  alpha = 1.0;
  zsorting = false;
  draw_edges = false;
  colorization=false;
  show_axes=false;
  parent_widget = parent;
  cmap = new QVector3D[NUM_COLOLORS];
  cmap[0] = QVector3D(1.0, 0.0, 0.0);
  cmap[1] = QVector3D(0.0, 1.0, 0.0);
  cmap[2] = QVector3D(0.0, 0.0, 1.0);
  cmap[3] = QVector3D(1.0, 1.0, 0.0);
  cmap[4] = QVector3D(1.0, 0.0, 1.0);
  cmap[5] = QVector3D(0.0, 1.0, 1.0);
  cmap[6] = QVector3D(0.3, 0.7, 0.2);
  cmap[7] = QVector3D(0.5, 0.2, 0.9);
  setFocusPolicy(Qt::StrongFocus);
  setlocale(LC_NUMERIC, "C");
}

GLWidget::~GLWidget() {
  delete[] vertices;
}

/**
  * Check if the input string consists of
  * the following characters only:
  * /eE-+.,0123456789 \r\n
  * Input: const std::string - the string to check
  * Output: boolean
  */
bool GLWidget::is_digits(const std::string &str)
{
    return str.find_first_not_of("/eE-+.,0123456789 \r\n") == std::string::npos;
}

/**
  * Load a model with .json extension
  * Input: const QString - path to the file
  * Output: FaceCollection - faces and normals of the model
  */
FaceCollection GLWidget::loadJson(const QString &path){
  FaceCollection result;
  QFile json_file(path);
  if (!json_file.open(QIODevice::ReadOnly)) {
    qWarning("Failed to open file");
    exit(-1);
  }
  QByteArray json_data = json_file.readAll();
  QJsonDocument json_document(QJsonDocument::fromJson(json_data));
  result.fromJson(json_document.array());
  return result;
}

/**
  * Load a model with .stl extension
  * Input: const QString - path to the file
  * Output: FaceCollection - faces and normals of the model
  */
FaceCollection GLWidget::loadStl(const QString &path){
  QMessageBox messageBox;
  FaceCollection result;
  double x, y, z;
  std::string line, token, value;
  bool end_reached = false; 
  std::string::size_type sz;

  std::ifstream infile(path.toUtf8().constData());
  if(!infile){
    messageBox.critical(0,"Error","File not found");
    throw std::runtime_error("File not found");
  }

  /* Read input file */
  if(DEBUG){
    qDebug() << "\tReading the file";
  }
  std::getline(infile, line); // Read file format line

  if(line != "solid Onshape"){
    messageBox.critical(0,"Error","File format is not supported (not STL).");
    throw std::runtime_error("File format is not supported (not STL).");
  }
  if(DEBUG){
    qDebug() << "\tReading the first line";
  }
  while(std::getline(infile, line)){
    Face new_face;
    new_face.c = 1;
    if(line.substr(2, 12) != "facet normal"){
      if(line.substr(0, 16) == "endsolid Onshape"){
        end_reached = true;
        break;
      }
      else{
        messageBox.critical(0,"Error","File is corrupted. Unexpected format");
        throw std::runtime_error("File is corrupted. Unexpected format");
      }
    }
    if(DEBUG){
      qDebug() << "\tEntered the loop";
      qDebug() << "\t\tRead \'facet normal\'";
    }
    value = line.substr(14, strlen(line.c_str()));
    if (!is_digits(value)){
      messageBox.critical(0,"Error","The definition of a normal contains non-digit characters");
      throw std::runtime_error("The definition of a normal contains non-digit characters");
    }
    value = line.substr(15, strlen(line.c_str()));
    x = std::stod(value,&sz);
    value = value.substr(sz);
    y = std::stod(value, &sz);
    value = value.substr(sz);
    z = std::stod(value, &sz);
    value = value.substr(sz);
    if(DEBUG){
      qDebug() << "\t\t" << x <<", "<<y<<", "<<z;
    }
    new_face.normal.push_back(QVector3D(x,y,z));
    new_face.normals = true;
    new_face.label = 0;
    if (!(value == "\0")){
      messageBox.critical(0,"Error","Unexpected number of dimensions in a normal vector");
      throw std::runtime_error("Unexpected number of dimensions in a normal vector");
    }
    if(!std::getline(infile, line)){
      messageBox.critical(0,"Error","Unexpected end of file");
      throw std::runtime_error("Unexpected end of file");
    }
    if(line.substr(4, 10) != "outer loop"){
      messageBox.critical(0,"Error","File is corrupted. Expected an outer loop after the normal vector");
      throw std::runtime_error("File is corrupted. Expected an outer loop after the normal vector");
    }
    /*Read 3 vertices*/
    for (int i=0; i<3; i++){
        if(!std::getline(infile, line)){
          messageBox.critical(0,"Error","Unexpected end of file");
          throw std::runtime_error("Unexpected end of file");
        }
        if(line.substr(6, 6) != "vertex"){
          messageBox.critical(0,"Error","File is corrupted. Expected a vertex in the outer loop");
          throw std::runtime_error("File is corrupted. Expected a vertex in the outer loop");
        }
        value = line.substr(13, strlen(line.c_str()));
        x = std::stod(value,&sz);
        value = value.substr(sz);
        y = std::stod(value, &sz);
        value = value.substr(sz);
        z = std::stod(value, &sz);
        value = value.substr(sz);
        new_face.vertices.push_back(QVector3D(x,y,z));
        if (!(value == "\0")){
          messageBox.critical(0,"Error","Unexpected number of dimensions in a vertex");
          throw std::runtime_error("Unexpected number of dimensions in a vertex");
        }
        if(DEBUG){
          qDebug() << "\t\t\t" << x <<", "<<y<<", "<<z;
        }
    }
    if(!std::getline(infile, line)){
      messageBox.critical(0,"Error","Unexpected end of file");
      throw std::runtime_error("Unexpected end of file");
    }
    if(line.substr(4, 7) != "endloop"){
      messageBox.critical(0,"Error","File is corrupted. Expected an endloop statement");
      throw std::runtime_error("File is corrupted. Expected an endloop statement");
    }
    if(!std::getline(infile, line)){
      messageBox.critical(0,"Error","Unexpected end of file");
      throw std::runtime_error("Unexpected end of file");
    }
    if(line.substr(2, 8) != "endfacet"){
      messageBox.critical(0,"Error","File is corrupted. Expected an endfacet statement");
      throw std::runtime_error("File is corrupted. Expected an endfacet statement");
    }
    result.faces.push_back(new_face);
  }
  if (!end_reached){
    messageBox.critical(0,"Error","File is corrupted");
    throw std::runtime_error("File is corrupted");
  }

  return result;
}

/**
  * Find the position of the next occurrence of
  * '/' or ' ' characters
  * Input: const std::string - string to check
  * Output: int - position of the next occurrence or -1
  */
int GLWidget::delim(const std::string &str){
  return std::min(str.find(" "), str.find("/"));
}

/**
  * Load a model with .obj extension
  * Input: const QString - path to the file
  * Output: FaceCollection - faces and normals of the model
  */
FaceCollection GLWidget::loadObj(const QString &path){
  FaceCollection result;
  double x, y, z;
  QMessageBox messageBox;
  std::string line, token, value;
  std::vector<QVector3D> v;
  std::vector<QVector3D> vn;
  std::string::size_type sz;
  std::ifstream infile(path.toUtf8().constData());
  if(!infile){
    messageBox.critical(0,"Error","File not found");
    throw std::runtime_error("File not found");

  }

  /* Read input file */
  if(DEBUG){
    qDebug() << "\tReading the file";
  }
  while(!infile.eof() && std::getline(infile, line)){
    if(line.substr(0,2) == "v "){
      // Read a vertex
      value = line.substr(2, strlen(line.c_str()));
      if (!is_digits(value)){
        messageBox.critical(0,"Error","The definition of a vertex contains non-digit characters");
        throw std::runtime_error("The definition of a vertex contains non-digit characters");
      }
      x = std::stod(value,&sz);
      value = value.substr(sz);
      y = std::stod(value, &sz);
      value = value.substr(sz);
      z = std::stod(value, &sz);
      value = value.substr(sz);
      v.push_back(QVector3D(x,y,z));
      if(DEBUG){
        qDebug() << "\t\t" <<  "v " << x << " " << y << " " << z;
      }
    }
    if(line.substr(0,2) == "vn"){
      // Read a normal
      value = line.substr(2, strlen(line.c_str()));
      if (!is_digits(value)){
        messageBox.critical(0,"Error","The definition of a normal contains non-digit characters");
        throw std::runtime_error("The definition of a normal contains non-digit characters");
      }
      x = std::stod(value,&sz);
      value = value.substr(sz);
      y = std::stod(value, &sz);
      value = value.substr(sz);
      z = std::stod(value, &sz);
      value = value.substr(sz);
      vn.push_back(QVector3D(x,y,z));
      if(DEBUG){
        qDebug() << "\t\t" << "vn " << x << " " << y << " " << z;
      }
    }
    if(line.substr(0,1) == "f"){
      Face new_face;
      int a, an;
      new_face.label=0;
      new_face.c = 1;
      line = line.substr(1);
      if(DEBUG){
        qDebug() << "\t\t" << "Reading f";
      }
      value = line.substr(line.find_first_not_of(" "));
      if (!is_digits(value)){
        messageBox.critical(0,"Error","The definition of a face contains non-digit characters");
        throw std::runtime_error("The definition of a face contains non-digit characters");
      }
      while((value.substr(0,1)!="\r") && (strlen(value.c_str())!=0)){
          // Read a vertex
          a = std::stoi(value.substr(0, delim(value)), &sz);
          new_face.vertices.push_back(v[a-1]);
          value = value.substr(sz);
          if(DEBUG){
            qDebug() << "\t\t" << "Vertex" << a << value.c_str();
          }
          // Read texture
          if(value.substr(0,1)=="/"){
            // Skip texture
            if(DEBUG){
              qDebug() << "\t\t" << "Skipping /";
            } 
            value = value.substr(1);
            value = value.substr(value.find("/"));
            if(DEBUG){
              qDebug() << "\t\t" << value.c_str();
            }
          }
          // Read normal
          if(value.substr(0,1)=="/"){
            value = value.substr(1);
            an = std::stoi(value.substr(0, delim(value)), &sz);
            new_face.normal.push_back(vn[an-1]);
            new_face.normals = true;
            if(DEBUG){
              qDebug() << "\t\t" << "Normal";
            }
            value = value.substr(sz);
            if(DEBUG){
              qDebug() << "\t\t" << an << " " << value.c_str();
            }
          }
          if(strlen(value.c_str())>0 && (int)value.find_first_not_of(" ")!=-1){
            value = value.substr(value.find_first_not_of(" "));
          }
          if(DEBUG){
            qDebug() << "\t\t Vertex info read\n\n";
          }
          if(infile.eof()){
            break;
          }
        }
        result.faces.push_back(new_face);
      }      
  }
  if(DEBUG){
    qDebug() << "END OF FILE";
  }
  return result;
}

/**
  * Load a model from file and render it
  * Calls loadJson(path), loadStl(path) or loadObj(path)
  * depending on the file's extension
  * Input: const QString - path to the file
  * Output: void
  */
void GLWidget::loadFaces(const QString &path) {
  QString extension = path.mid(path.lastIndexOf(QString("."))+1, path.length()-1);
  if(extension == "json"){
    if(DEBUG==true){
      qDebug() << "Reading .json";
    }
    face_collection = loadJson(path);
  }
  if(extension == "stl"){
    if(DEBUG==true){
      qDebug() << "Reading .stl";
    }
    face_collection = loadStl(path);
  }
  if(extension == "obj"){
    if(DEBUG==true){
      qDebug() << "Reading .obj";
    }
    face_collection = loadObj(path);
  }
  n_vertices = face_collection.faces.size() * 4;
  vertices = new QVector3D[n_vertices];
  double min_z, max_z;
  min_z = face_collection.faces[0].vertices[0][2];
  max_z = face_collection.faces[0].vertices[0][2];
  for (int i = 0; i < (int)face_collection.faces.size(); i++)
  {
    vertices[i*4] = face_collection.faces[i].vertices[0];
    vertices[i*4+1] = face_collection.faces[i].vertices[1];
    vertices[i*4+2] = face_collection.faces[i].vertices[2];
    vertices[i*4+3] = face_collection.faces[i].vertices[3];
    if(vertices[i*4][2] < min_z){
      min_z = vertices[i*4][2];
    }
    if(vertices[i*4][2] > max_z){
      max_z = vertices[i*4][2];
    }
  }
  scale = 1/std::max(abs(min_z), abs(max_z));
  if(colorization==true){
    colorize(face_collection);
  }
  update();
}

/**
  * Shift point of view along the x axis and update the scene
  * Input: double - shift distance
  * Output: void
  */
void GLWidget::setXTranslation(double d)
{
  x_translation = d;
  update();
}

/**
  * Shift point of view along the y axis and update the scene
  * Input: double - shift distance
  * Output: void
  */
void GLWidget::setYTranslation(double d)
{
  y_translation = d;
  update();
}

/**
  * Initialize the scene
  */
void GLWidget::initializeGL() {
  glClearColor(0.2f, 0.25f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING );
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
}

/**
  * Resize the view window
  * Input: int, int - new width and height of the window
  * Output: void
  */
void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-2.0 + scale, 2.0 - scale, -2.0 + scale, 2.0 - scale, -scale, scale);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  * Paint the scene
  */
void GLWidget::paintGL() {
 
  // Clear color and depth buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the modelview matrix
  QMatrix4x4 matrix;
  matrix.translate(x_translation, y_translation, z_translation);
  matrix.rotate(rotation);
  glLoadMatrixf(matrix.constData());

  // Set the projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glScalef(scale, scale, scale);
  glMatrixMode(GL_MODELVIEW);

  // Draw axes
  if(show_axes){
    drawAxes();
  }

  // Perform z-sorting
  std::vector<std::pair<float, int>> order;
  for (int i=0;i<(int)face_collection.faces.size();i++)
  {
      QVector3D centre = ((vertices[i*4] + vertices[i*4+2])/2);
      double z = centre.x()*matrix(2,0) + centre.y()*matrix(2,1) + centre.z()*matrix(2,2) + matrix(2,3);
      double h = centre.x()*matrix(3,0) + centre.y()*matrix(3,1) + centre.z()*matrix(3,2) + matrix(3,3);
      order.push_back(std::make_pair(abs(z/h),i));
  }
  std::sort(order.begin(), order.end());
  std::reverse(order.begin(),order.end());

  if(zsorting){
    std::sort(order.begin(), order.end());
  }

  // Draw faces
  for (int i = 0; i < n_vertices/4; i++)
  {
    Face face = face_collection.faces[order[i].second];
    if(face_collection.faces[order[i].second].normals==true){
        face.normal[0] = face.normal[0] * matrix;
        face.normals=true;
    }

    if(zsorting){
        if(isFacingCamera(face)){
          drawFace(face_collection.faces[order[i].second]);
        }
    }
    else{
      drawFace(face_collection.faces[i]);
    }
  }
}

/**
  * Draw a face
  * Input: Face - a face to draw
  * Output: void
  */
void GLWidget::drawFace(Face face) {
    double triangle_color = face.c;
    // If colorization is on, select a color for the face
    if(colorization == true){
      int label = face.label;
      label = label % NUM_COLOLORS;
      glColor4f(cmap[label][0], cmap[label][1], cmap[label][2], alpha);
    }
    else{
      glColor4f(triangle_color, triangle_color, triangle_color, alpha); 
    }
    // Draw a face
    glBegin(GL_POLYGON);
    for(int i=0; i<(int)face.vertices.size(); i++){
      glVertex3f(face.vertices[i][0], face.vertices[i][1], face.vertices[i][2]);
      if(face.normals==true && i<(int)face.normal.size()){
        glBegin(GL_POLYGON);
        glNormal3f(face.normal[i][0], face.normal[i][1], face.normal[i][2]);  
      }
    }
    glEnd();
    // If draw_edges is true, display them
    if(draw_edges==true){
        QVector3D last_vertex = face.vertices[0];
        QVector3D current_vertex;
        glColor4f(0.0f, 0.0f, 0.0f, alpha);
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(10.0f);
        glBegin(GL_LINES); 
        for(int i=1; i<(int)face.vertices.size(); i++){
          current_vertex = face.vertices[i];
          glVertex3f(last_vertex[0], last_vertex[1], last_vertex[2]);
          glVertex3f(current_vertex[0], current_vertex[1], current_vertex[2]);
          last_vertex = face.vertices[i];
        }
        glEnd();
    }
}


/**
  * Colorize closed surfaces
  * Input: FaceCollection - a collection of faces
  * Output: void
  */
void GLWidget::colorize(FaceCollection &faces){
  bool all_checked = false;
  // for each face
  int label =1;
  std::vector<int> to_check;
  while(!all_checked){
          all_checked = true;
          // Find a face without a label, assign it a label
          // And add to the queue
          for(int i=0; i<(int)faces.faces.size(); i++){
            if(faces.faces[i].label==0){
              faces.faces[i].label = label;
              to_check.push_back(i);
              break;
            }
          }

          while(to_check.size() > 0){
              // Take a face from the queue
              int i = to_check[to_check.size()-1];
              to_check.pop_back();

              // make all possible pairs for the face with this one
              for(int j=0; j<(int)faces.faces.size(); j++){
              
                // check that the pair of the face has also not been assigned a label
                if(faces.faces[j].label==0){
                  all_checked=false;

                  // for all edges of the two faces
                  for(int k=0; k<(int)faces.faces[i].vertices.size(); k++){
                    for(int m=0; m<(int)faces.faces[j].vertices.size(); m++){

                      // check if the same edge belongs to both faces
                      if(faces.faces[i].vertices[k]==faces.faces[j].vertices[m] &&
                          faces.faces[i].vertices[(k+1)%(int)faces.faces[i].vertices.size()]==
                          faces.faces[i].vertices[(m+1)%(int)faces.faces[j].vertices.size()]){
                        // assign the second face the same label
                        faces.faces[j].label = faces.faces[i].label;
                        // add the face to the queue to check its neighbours
                        to_check.push_back(j);
                        // Skip the rest of edges in the face
                        goto face2_loop_end;
                      }
                    }
                  }
                }
                face2_loop_end:;
              }
            }  
            label++;
  }
}


/**
  * Draw x, y and z axes
  * Input: void
  * Output: void
  */
void GLWidget::drawAxes(){
    glColor3f(1.0,0.0,0.0); // red x
    glBegin(GL_LINES);
    // x aix
 
    glVertex3f(-10000.0f, 0.0f, 0.0f);
    glVertex3f(20.0f, 0.0f, 0.0f);
 
    glVertex3f(4.0, 0.0f, 0.0f);
    glVertex3f(3.0, 1.0f, 0.0f);
 
    glVertex3f(4.0, 0.0f, 0.0f);
    glVertex3f(3.0, -1.0f, 0.0f);
    glEnd();
 
    // y 
    glColor3f(0.0,1.0,0.0); // green y
    glBegin(GL_LINES);
    glVertex3f(0.0f, -10000.0f, 0.0f);
    glVertex3f(0.0f, 20.0f, 0.0f);
 
    glVertex3f(0.0, 4.0f, 0.0f);
    glVertex3f(1.0, 3.0f, 0.0f);
 
    glVertex3f(0.0, 4.0f, 0.0f);
    glVertex3f(-1.0, 3.0f, 0.0f);
    glEnd();
 
    // z 
    glColor3f(0.0,0.0,1.0); // blue z
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f ,-10000.0f);
    glVertex3f(0.0, 0.0f ,20.0f);
 
 
    glVertex3f(0.0, 0.0f ,4.0f );
    glVertex3f(0.0, 1.0f ,3.0f );
 
    glVertex3f(0.0, 0.0f ,4.0f );
    glVertex3f(0.0, -1.0f ,3.0f );
    glEnd();
}


/**
  * Check if the normal vector of a face is in a direction 
  * of the camera
  * Input: Face - a face whose normal is being checked
  * Output: bool - true if the normal is facing the camera
  */
bool GLWidget::isFacingCamera(Face face){
  if (face.normals==false){
    return true;
  }
  else{
    QVector3D camera = QVector3D(0,0,-1);
    double dot = QVector3D::dotProduct(camera, face.normal[0]);
    double angle = acos(dot) * 180 / doublePi;
    if (angle <= 90)
      return true;
    else
      return false;
  }
}

/**
  * Enable/disable sorting based on the state of the
  * according checkbox
  * Input: bool - new state
  * Output: void
  */
void GLWidget::enableSorting(bool state){
  zsorting = state;
  update();
}

/**
  * Enable/disable drawing edges based on the state of the
  * according checkbox
  * Input: bool - new state
  * Output: void
  */
void GLWidget::enableDrawingEdges(bool state){
  draw_edges = state;
  update();
}

/**
  * Show/remove axes based on the state of the
  * according checkbox
  * Input: bool - new state
  * Output: void
  */
void GLWidget::showAxes(bool state){
  show_axes = state;
  update();
}

/**
  * Enable/disable colorization based on the state of the
  * according checkbox
  * Input: bool - new state
  * Output: void
  */
void GLWidget::enableColorization(bool state){
  colorization = state;
  if(colorization == true){
    colorize(face_collection);
  }
  update();
}

/**
  * Mouse event handler - implements translation and rotation
  * Input: QMouseEvent - a mouse event
  * Output: void
  */
void GLWidget::mouseMoveEvent(QMouseEvent *event) {
  QVector2D diff = QVector2D(event->localPos()) - mousePressPosition;
  if (event->buttons() & Qt::LeftButton) {
      // Rotation axis is perpendicular to the mouse position difference
      // vector
      QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();
      // Calculate new rotation axis as weighted sum
      rotationAxis = (angularSpeed * n).normalized();
      // Increase angular speed
      angularSpeed = 3 + accelerated*acc_factor;
      // Update rotation
      rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;
      // Request an update
      update();
  } else if (event->buttons() & Qt::RightButton) {
      setXTranslation(x_translation + (diff.x() + diff.x() * accelerated * acc_factor)/50.0);
      setYTranslation(y_translation - (diff.y() + diff.y() * accelerated * acc_factor)/50.0);
  }
  mousePressPosition = QVector2D(event->localPos());
}

/**
  * Mouse wheel event handler - implements zooming
  * Input: QWheelEvent - a mouse wheel event
  * Output: void
  */
void GLWidget::wheelEvent(QWheelEvent *event) {
  event->delta() > 0 ? scale *= 1.1f + accelerated * acc_factor : scale *= 0.9f + accelerated * acc_factor;
  update();
}

/**
  * Mouse press event handler 
  * Input: QMouseEvent - a mouse event
  * Output: void
  */
void GLWidget::mousePressEvent(QMouseEvent *event){
  mousePressPosition = QVector2D(event->localPos());
}

/**
  * Key press event handler - implements acceleration of translation and rotation
  * Input: QKeyEvent - a mouse event
  * Output: void
  */
void GLWidget::keyPressEvent(QKeyEvent *event){
  if (event->key() == Qt::Key::Key_Shift){
    accelerated = true;
  }
}

/**
  * Key release event handler - implements acceleration of translation and rotation
  * Input: QKeyEvent - a mouse event
  * Output: void
  */
void GLWidget::keyReleaseEvent(QKeyEvent *event){
  if (event->key() == Qt::Key::Key_Shift){
    accelerated = false;
  }
}

/**
  * Update transparency of the model based on the slider's state
  * Input: double - new_alpha
  * Output: void
  */
void GLWidget::updateAlpha(double new_alpha){
  alpha = new_alpha;
  update();
}