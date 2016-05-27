/** Copyright (c) 2013, Sean Kasun */
#ifndef DEFINITIONUPDATER_H_
#define DEFINITIONUPDATER_H_

#include <QObject>
#include <QDateTime>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>

class QFile;
class QNetworkReply;

class DefinitionUpdater : public QObject {
  Q_OBJECT
 public:
  DefinitionUpdater(QString filename, QString url, QDateTime timestamp);
  void update();
 signals:
  void updated(DefinitionUpdater *, QString filename, QDateTime timestamp);
 private slots:
  void checkTime();
  void didUpdate();
  void saveFile();
 private:
  QString filename;
  QUrl url;
  QDateTime timestamp;
  QNetworkAccessManager qnam;
  QNetworkReply *reply;
  QFile *save;
};

#endif  // DEFINITIONUPDATER_H_
