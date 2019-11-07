#include "face.h"

QJsonArray vectorToJson(const QVector3D &vector) {
  QJsonArray result;
  for (int dim = 0; dim < 3; dim++)
    result.append(QJsonValue(vector[dim]));
  return result;
}

QVector3D vectorFromJson(const QJsonArray &array) {
  if (array.count() != 3) {
    throw std::runtime_error("Invalid size for vector: " +
                             std::to_string(array.count()));
  }
  QVector3D result;
  for (int dim = 0; dim < 3; dim++) {
    if (!array.at(dim).isDouble())
      throw std::runtime_error("Invalid value in vector at idx: " +
                               std::to_string(dim));
    result[dim] = array.at(dim).toDouble();
  }
  return result;
}

QJsonObject Face::toJson() const {
  QJsonArray vertices_json;
  for (const QVector3D &v : vertices)
    vertices_json.append(vectorToJson(v));
  QJsonObject result;
  result["vertices"] = vertices_json;
  result["normal"] = vectorToJson(normal[0]);
  result["normals"] = normals;
  result["color"] = c;
  result["label"] = label;
  return result;
}

void Face::fromJson(const QJsonObject &json) {
  for (const std::string &field : {"vertices", "normal", "color"}) {
    if (!json.contains(QString(field.data())))
      throw std::runtime_error("Missing field '" + field + "' in json file");
  }
  vertices.clear();
  for (const QJsonValue &vertex : json["vertices"].toArray()) {
    vertices.push_back(vectorFromJson(vertex.toArray()));
  }
  std::vector<QVector3D> n;
  n.push_back(vectorFromJson(json["normal"].toArray()));
  normal = n;
  normals = true;
  label = 0;
  c = json["color"].toDouble();
}

void FaceCollection::fromJson(const QJsonArray &json) {
  faces.clear();
  for (const QJsonValue &face : json) {
    Face new_face;
    new_face.fromJson(face.toObject());
    faces.push_back(new_face);
  }
}
