/** Copyright (c) 2013, Sean Kasun */

#include <QDebug>
#include <QFile>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "./definitionupdater.h"

DefinitionUpdater::DefinitionUpdater(QString filename, QString url,
                                     QDateTime timestamp) :
  filename(filename), url(url), timestamp(timestamp) {
  save = NULL;
}

void DefinitionUpdater::update() {
  reply = qnam.head(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()),
          this, SLOT(checkTime()));
}

void DefinitionUpdater::checkTime() {
  QVariant redir =
      reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if (!redir.isNull()) {
    reply->deleteLater();
    url = redir.toUrl();
    update();
    return;
  }
  QVariant lmod = reply->header(QNetworkRequest::LastModifiedHeader);
  if (lmod.isValid() && lmod.toDateTime() > timestamp) {
    timestamp = lmod.toDateTime();
    reply->deleteLater();
    reply = qnam.get(QNetworkRequest(url));
    save = new QFile(filename);
    save->open(QIODevice::WriteOnly | QIODevice::Truncate);
    connect(reply, SIGNAL(finished()),
            this, SLOT(didUpdate()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(saveFile()));
    return;
  }
  reply->deleteLater();
  // no need to update
  emit updated(this, filename, timestamp);
}
void DefinitionUpdater::didUpdate() {
  save->flush();
  save->close();
  delete save;
  save = NULL;
  emit updated(this, filename, timestamp);
}
void DefinitionUpdater::saveFile() {
  if (save)
    save->write(reply->readAll());
}
