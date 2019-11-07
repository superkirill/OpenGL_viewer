#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QVector3D>

QJsonArray vectorToJson(const QVector3D &vector);
QVector3D vectorFromJson(const QJsonArray &array);

class Face {
public:
  Face(){
    normals = false;
  };
  std::vector<QVector3D> vertices;
  std::vector<QVector3D> normal;
  bool normals;
  float c;
  int label;

  QJsonObject toJson() const;
  void fromJson(const QJsonObject &json);
};

class FaceCollection {
public:
  std::vector<Face> faces;

  void fromJson(const QJsonArray &json);
};
