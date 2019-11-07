#include <QApplication>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "viewer_widget.h"

void usage(int argc, char **argv) {
  (void)argc;
  std::cerr << "Usage: " << argv[0] << " <optional: input.json>" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  if (argc > 2) {
    usage(argc, argv);
  }

  ViewerWidget viewer_widget;
  if (argc == 2)
    viewer_widget.gl_widget->loadFaces(argv[1]);
  viewer_widget.show();
  return app.exec();
}
