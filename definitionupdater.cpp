/*
   Copyright (c) 2013, Sean Kasun
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "definitionupdater.h"
#include <QDebug>
#include <QFile>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

DefinitionUpdater::DefinitionUpdater(QString filename,
									 QString url,
									 QDateTime timestamp) : filename(filename), url(url), timestamp(timestamp)
{
	save=NULL;
}

void DefinitionUpdater::update()
{
	reply=qnam.head(QNetworkRequest(url));
	connect(reply,SIGNAL(finished()),
			this,SLOT(checkTime()));
}

void DefinitionUpdater::checkTime()
{
	QVariant redir=reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (!redir.isNull())
	{
		reply->deleteLater();
		url=redir.toUrl();
		update();
		return;
	}
	QVariant lmod=reply->header(QNetworkRequest::LastModifiedHeader);
	if (lmod.isValid() && lmod.toDateTime()>timestamp)
	{
		timestamp=lmod.toDateTime();
		qDebug()<<"timestamp:"<<timestamp;
		reply->deleteLater();
		reply=qnam.get(QNetworkRequest(url));
		save=new QFile(filename);
		save->open(QIODevice::WriteOnly);
		connect(reply,SIGNAL(finished()),
				this,SLOT(didUpdate()));
		connect(reply,SIGNAL(readyRead()),
				this,SLOT(saveFile()));
		return;
	}
	reply->deleteLater();
	// no need to update
	emit updated(this,filename,timestamp);
}
void DefinitionUpdater::didUpdate()
{
	save->flush();
	save->close();
	delete save;
	save=NULL;
	emit updated(this,filename,timestamp);
}
void DefinitionUpdater::saveFile()
{
	if (save)
		save->write(reply->readAll());
}
