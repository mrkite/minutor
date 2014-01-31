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


#include "labelledslider.h"
#include <QVBoxLayout>

LabelledSlider::LabelledSlider(QWidget *parent) : QWidget(parent)
{
	slider = new MSlider(Qt::Vertical);
	slider->setRange(0,255);

	label = new QLabel();
	label->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
	label->setFixedWidth(label->fontMetrics().width("888"));
	label->setNum(255);

	connect(slider, SIGNAL(valueChanged(int)),
	        this,   SLOT(intValueChange(int)));

	QVBoxLayout *myLayout = new QVBoxLayout;
	myLayout->addWidget(label);
	myLayout->addWidget(slider);
	setLayout(myLayout);
	setFocusProxy(slider);
}

int LabelledSlider::value() const
{
	return slider->value();
}

// public slot
void LabelledSlider::setValue(int v)
{
	slider->setValue(v);
}

// public slot
void LabelledSlider::changeValue(int v)
{
	slider->setValue(slider->value()+v);
}

// private slot
void LabelledSlider::intValueChange(int v)
{
	label->setNum(v);
	emit valueChanged(v);
}

void  LabelledSlider::wheelEvent( QWheelEvent* event )
{
	slider->wheelEvent( event );
}

MSlider::MSlider(Qt::Orientation orientation, QWidget* parent)
	: QSlider(orientation, parent)
{}

void  MSlider::wheelEvent( QWheelEvent* event )
{
	int delta = event->delta() / 120; // in order to make wheel intuitive
	this->setValue(this->value()+delta);
}
