/** Copyright 2014 Rian Shelley */
#ifndef OVERLAYITEM_H_
#define OVERLAYITEM_H_

#include <QColor>
#include <QString>
#include <QVariant>

class QPainter;

class OverlayItem {
 public:
  virtual ~OverlayItem() {}
  struct Point {
    explicit Point(double x = 0, double y = 0, double z = 0): x(x), y(y), z(z) {}
    double x, y, z;
  };

  virtual bool intersects(const Point& min, const Point& max) const = 0;
  virtual void draw(double offsetX, double offsetZ, double scale,
                    QPainter *canvas) const = 0;
  virtual Point midpoint() const = 0;
  const QString& type() const {return itemType;}
  const QString& display() const { return itemDescription;}
  const QVariant& properties() const { return itemProperties;}
  const QColor& color() const { return itemColor; }
  const QString& dimension() const { return itemDimension; }

 protected:
  void setProperties(const QVariant& props) {itemProperties = props;}
  void setColor(const QColor& c) {itemColor = c;}
  void setDimension(const QString& d) {itemDimension = d;}
  void setDisplay(const QString& d) {itemDescription = d;}
  void setType(const QString& t) {itemType = t;}

 private:
  QVariant itemProperties;
  QColor itemColor;
  QString itemDimension;
  QString itemType;
  QString itemDescription;
};

#endif  // OVERLAYITEM_H_
