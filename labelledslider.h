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


#ifndef __LABELLEDSLIDER_H__
#define __LABELLEDSLIDER_H__

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QWheelEvent>
#include <QKeyEvent>

class MSlider : public QSlider
{
	Q_OBJECT

public:
	MSlider(Qt::Orientation orientation, QWidget * parent = 0);

public slots:
	void wheelEvent( QWheelEvent* event );
};


class LabelledSlider : public QWidget
{
	Q_OBJECT
public:
	explicit LabelledSlider(QWidget *parent = 0);
	int value() const;

signals:
	void valueChanged(int);

public slots:
	void setValue(int);    // set absolute
	void changeValue(int); // change relative

private slots:
	void intValueChange(int);

protected:
	void wheelEvent( QWheelEvent* event );

private:
	MSlider *slider;
	QLabel *label;
};

#endif
