#include "viewer_widget.h"

#include <QFileDialog>

ViewerWidget::ViewerWidget() {
  layout = new QGridLayout(this);
  load_file_button = new QPushButton("Load file");
  enable_sorting_checkbox = new QCheckBox("Sorting");
  enable_drawing_edges = new QCheckBox("Show edges");
  enable_colorization = new QCheckBox("Colorize");
  show_axes = new QCheckBox("Show axes");
  alpha_slider = new QSlider(Qt::Horizontal);
  gl_widget = new GLWidget();
  layout->addWidget(load_file_button, 0, 0);
  layout->addWidget(gl_widget, 1, 0);
  layout->addWidget(alpha_slider,2,0);
  layout->addWidget(enable_sorting_checkbox, 3,0);
  layout->addWidget(enable_drawing_edges, 4,0);
  layout->addWidget(enable_colorization, 5,0);
  layout->addWidget(show_axes, 6,0);
  connect(load_file_button, SIGNAL(released()), this, SLOT(loadFile()));
  connect(alpha_slider, SIGNAL(valueChanged(int)), this, SLOT(updateAlpha()));
  connect(enable_sorting_checkbox, SIGNAL(stateChanged(int)), this, SLOT(enableSorting()));
  connect(enable_drawing_edges, SIGNAL(stateChanged(int)), this, SLOT(enableDrawingEdges()));
  connect(enable_colorization, SIGNAL(stateChanged(int)), this, SLOT(enableColorization()));
  connect(show_axes, SIGNAL(stateChanged(int)), this, SLOT(showAxes()));
  alpha_slider->setValue(100);
  _aspectRatio = 1;
  _min_size = 400;
}

void ViewerWidget::loadFile() {
  QString file_name;
  file_name = QFileDialog::getOpenFileName(this,
        tr("Open model"), "",
        tr("Model files (*.json *.stl *.obj);;All Files (*)"));
  gl_widget->loadFaces(file_name);
}

void ViewerWidget::updateAlpha(){
  gl_widget->updateAlpha(alpha_slider->value()/100.0);
}

void ViewerWidget::enableSorting(){
  if(enable_sorting_checkbox->checkState() == Qt::Checked)
  {
    gl_widget->enableSorting(true);
  }
  else{
    gl_widget->enableSorting(false); 
  }
}

void ViewerWidget::showAxes(){
  if(show_axes->checkState() == Qt::Checked)
  {
    gl_widget->showAxes(true);
  }
  else{
    gl_widget->showAxes(false); 
  }
}

void ViewerWidget::enableDrawingEdges(){
  if(enable_drawing_edges->checkState() == Qt::Checked)
  {
    gl_widget->enableDrawingEdges(true);
  }
  else{
    gl_widget->enableDrawingEdges(false); 
  }
}

void ViewerWidget::enableColorization(){
  if(enable_colorization->checkState() == Qt::Checked)
  {
    gl_widget->enableColorization(true);
  }
  else{
    gl_widget->enableColorization(false); 
  }
}

void ViewerWidget::resizeEvent(QResizeEvent *event){
    int containerWidth = this->width();
    int containerHeight = this->height();
    if (containerHeight < _min_size){
      containerHeight = _min_size;
    }
    if (containerWidth < _min_size){
      containerWidth = _min_size;
    }
    int contentsHeight = containerHeight;
    int contentsWidth = containerHeight * _aspectRatio;
    if (contentsWidth > containerWidth ) {
        contentsWidth = containerWidth;
        contentsHeight = containerWidth / _aspectRatio;
    }

    resize(contentsWidth, contentsHeight);
}