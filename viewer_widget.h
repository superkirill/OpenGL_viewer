#pragma once

#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QLabel>
#include <QString>

#include "glwidget.h"

class ViewerWidget : public QWidget {
public:
  Q_OBJECT
public:
  ViewerWidget();
  void resizeEvent(QResizeEvent *event) override;
  void updateParams(QString text);
  QGridLayout *layout;
  QPushButton *load_file_button;
  GLWidget *gl_widget;
  QSlider *alpha_slider;
  QCheckBox *enable_sorting_checkbox, *enable_drawing_edges, *enable_colorization, *show_axes;
public slots:
  void loadFile();
  void updateAlpha();
  void enableDrawingEdges();
  void enableSorting();
  void enableColorization();
  void showAxes();
private:
  double _aspectRatio;
  double _min_size;
};
