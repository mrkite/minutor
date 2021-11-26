/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <QFile>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "./definitionupdater.h"

DefinitionUpdater::DefinitionUpdater(QString filename,
                                     QString url,
                                     QString version) :
  filename(filename), url(url), version(version), save(NULL) {}

void DefinitionUpdater::update() {
  reply = qnam.head(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()),
          this, SLOT(checkReply()));
}

void DefinitionUpdater::checkReply() {
  // check for server side redirection
  QVariant redir = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (!redir.isNull()) {
    reply->deleteLater();
    // start update again with redirected URL
    url = redir.toUrl();
    update();
    return;
  }
  // final URL found -> get data
  reply->deleteLater();
  reply = qnam.get(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()),
          this, SLOT(finishUpdate()));
  connect(reply, SIGNAL(readyRead()),
          this, SLOT(checkReadyRead()));
}

void DefinitionUpdater::checkReadyRead() {
  // get all data received at the moment (more may follow)
  const QByteArray data(reply->readAll());

  if (save) {
    // consecutive data received
    // -> append it to current file
    save->write(data);
  } else {
    // no file open -> first data for our request
    // -> parse reply data for version information
    QString remoteversion = parseVersion(data);

    // check for newer version
    if (versionCompare(remoteversion,version)>0) {
      version = remoteversion;
      save = new QFile(filename);
      save->open(QIODevice::WriteOnly | QIODevice::Truncate);
      if (save) {
        save->write(data);
      }
    }
  }
}

QString DefinitionUpdater::parseVersion(const QByteArray & data) {
  int vtag   = data.indexOf("\"version\"");
  int vstart = data.indexOf("\"", vtag+9) + 1;    // "version" is length 9
  int vlen   = data.indexOf("\"", vstart) - vstart;
  QString remoteversion = "0.0.0";
  if ((vtag!=-1)&&(vstart!=-1)&&(vlen>0))
    remoteversion = data.mid(vstart, vlen);
  return remoteversion;
}

void DefinitionUpdater::finishUpdate() {
  // close open files
  if (save) {
    save->flush();
    save->close();
    delete save;
    save = NULL;
  }
  reply->deleteLater();
  emit updated(this, filename, version);
}

// static
int DefinitionUpdater::versionCompare(QString const &version1, QString const &version2)
{
  // split in major and minor version sections
  QStringList sec1 = version1.split('.');
  QStringList sec2 = version2.split('.');

  // iterate over all main and minor version parts
  int mm = 0;
  while (mm<sec1.length() && mm<sec2.length()) {
    int c = 0;
    bool ok1, ok2;
    int i1 = sec1[mm].toInt(&ok1,10);
    int i2 = sec2[mm].toInt(&ok2,10);

    if (ok1 && ok2) {   // both are integer numbers
      c = i1 - i2;
    }
    if (!ok1 && !ok2) { // both are strings
      c = QString::compare(sec1[mm], sec2[mm]);
    }
    if (ok1 && !ok2) { // i1 is an int i2 is a string
      return 1; // int wins
    }
    if (!ok1 && ok2) { // i1 is a string i2 an int
      return -1; // int wins
    }

    // finished when any difference is found
    if (c != 0) return c;
    // else proceed with next section
    mm++;
  }
  // longer one wins :-)
  return sec1.length() - sec2.length();
}
