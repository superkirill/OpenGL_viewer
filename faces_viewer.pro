QT_VERSION = 5
QMAKE_CXXFLAGS += -std=c++11

HEADERS = glwidget.h face.h viewer_widget.h
SOURCES = faces_viewer.cpp glwidget.cpp face.cpp viewer_widget.cpp
QT     += opengl widgets
