#pragma once

#include <QMatrix4x4>
#include <QOpenGLWidget>
#include <QString>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector4D>
#include <QOpenGLBuffer>

#include "face.h"

class GLWidget : public QOpenGLWidget {
public:
  Q_OBJECT
public:
  GLWidget(QWidget *parent = 0);
  ~GLWidget();
  QSize sizeHint() const { return QSize(1200, 1200); }

  void loadFaces(const QString &path);
  void updateAlpha(double new_alpha);
  void enableSorting(bool state);
  void enableDrawingEdges(bool state);
  void enableColorization(bool state);
  void showAxes(bool state);

protected:
  void initializeGL() override;
  FaceCollection loadJson(const QString &path);
  FaceCollection loadStl(const QString &path);
  FaceCollection loadObj(const QString &path);
  int delim(const std::string &str);
  bool is_digits(const std::string &str);
  void paintGL() override;
  void resizeGL(int width, int height) override;
  void drawFace(Face face);
  void wheelEvent(QWheelEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void setXTranslation(double d);
  void setYTranslation(double d);
  void drawAxes();
  bool isFacingCamera(Face face);
  void colorize(FaceCollection &faces);

  FaceCollection face_collection;
  double x_translation;
  double y_translation;
  double z_translation;
  float scale;
  double acc_factor;
  bool accelerated;
  double alpha;
  int n_vertices;

  QVector2D mousePressPosition;
  QVector3D rotationAxis;
  qreal angularSpeed;
  QQuaternion rotation;
  QVector3D * vertices;
  QVector3D * cmap;
  bool zsorting;
  bool draw_edges;
  bool colorization;
  bool show_axes;
  QWidget *parent_widget;
};
