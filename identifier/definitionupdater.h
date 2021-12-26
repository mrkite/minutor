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
  DefinitionUpdater(QString filename, QString url, QString version);
  void update();
  static int versionCompare( QString const &version1, QString const &version2 );
 signals:
  void updated(DefinitionUpdater *, QString filename, QString version);
 private slots:
  void checkReply();
  void checkReadyRead();
  void finishUpdate();
 private:
  QString parseVersion(const QByteArray & data);
  QString filename;
  QUrl url;
  QString version;
  QNetworkAccessManager qnam;
  QNetworkReply *reply;
  QFile *save;
};

#endif  // DEFINITIONUPDATER_H_
