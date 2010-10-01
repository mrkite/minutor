/*
Copyright (c) 2010, Sean Kasun
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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "MinutorMap/MinutorMap.h"
#include "minutor.xpm"

static double curX,curZ;
static int curDepth=127;
static double curScale=1.0;
static const char *world;
static unsigned char *bits;
static int curWidth,curHeight;

static gboolean mouseUp(GtkWidget *widget,GdkEventButton *event);

static void destroy()
{
	gtk_main_quit();
}


static gboolean drawMap(GtkWidget *widget)
{
	int w=widget->allocation.width;
	int h=widget->allocation.height;
	if (w!=curWidth || h!=curHeight)
	{
		curWidth=w;
		curHeight=h;
		bits=g_realloc(bits,curWidth*curHeight*4);
	}
	DrawMap(world,curX,curZ,curDepth,curWidth,curHeight,curScale,bits);
	gdk_draw_rgb_32_image(
		widget->window,
		widget->style->white_gc,
		0,0,curWidth,curHeight,
		GDK_RGB_DITHER_NONE,
		bits,
		curWidth*4);
	return TRUE;
}

static gchar *getSliderText(GtkScale *scale,gdouble value)
{
	return g_strdup_printf("%d",
		127-(int)value);
}
static void adjustMap(GtkRange *range,gpointer user_data)
{
	curDepth=127-(int)gtk_range_get_value(range);
	gdk_window_invalidate_rect(GTK_WIDGET(user_data)->window,NULL,FALSE);
}
static GtkWidget *menu=NULL;
static gboolean mousePopup(gdouble x,gdouble y)
{
	int ofs=y*curWidth*4+x*4;
	unsigned int color;
	const char *name;
	if (y>curHeight || x>curWidth)
		return FALSE;

	color=(bits[ofs]<<16)|(bits[ofs+1]<<8)|bits[ofs+2];
	name=IDBlock(color);

	menu=gtk_menu_new();
	GtkWidget *item=gtk_menu_item_new_with_label(name);
	gtk_menu_attach(GTK_MENU(menu),item,0,1,0,1);
	gtk_widget_set_sensitive(item,FALSE);
	gtk_widget_show(item);
	gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,3,
		gtk_get_current_event_time());
	g_signal_connect(G_OBJECT(menu),"button-release-event",
		G_CALLBACK(mouseUp),NULL);
	return TRUE;
}
static gboolean tracking=FALSE;
static double oldX,oldY;
static gboolean mouseDown(GtkWidget *widget,GdkEventButton *event)
{
	if (event->button==3)
		return mousePopup(event->x,event->y);
	gtk_widget_grab_focus(widget);
	oldX=event->x;
	oldY=event->y;
	tracking=TRUE;
	return TRUE;
}
static gboolean mouseUp(GtkWidget *widget,GdkEventButton *event)
{
	if (event->button==3)
	{
		if (menu!=NULL)
		{
			gtk_menu_popdown(GTK_MENU(menu));
			gtk_widget_destroy(menu);
		}
		menu=NULL;
		return TRUE;
	}
	tracking=FALSE;
	return TRUE;
}
static gboolean mouseMove(GtkWidget *widget,GdkEventMotion *event)
{
	if (tracking==FALSE) return FALSE;
	curX+=(oldX-event->x)/curScale;
	curZ+=(oldY-event->y)/curScale;
	oldX=event->x;
	oldY=event->y;
	gdk_window_invalidate_rect(widget->window,NULL,FALSE);
	return TRUE;
}
static gboolean mouseWheel(GtkWidget *widget,GdkEventScroll *event)
{
	if (event->direction==GDK_SCROLL_DOWN)
	{
		curScale-=0.2;
		if (curScale<1.0) curScale=1.0;
		gdk_window_invalidate_rect(widget->window,NULL,FALSE);
	}
	if (event->direction==GDK_SCROLL_UP)
	{
		curScale+=0.2;
		if (curScale>5.0) curScale=5.0;
		gdk_window_invalidate_rect(widget->window,NULL,FALSE);
	}
	return TRUE;
}
static gboolean keyDown(GtkWidget *widget,GdkEventKey *event)
{
	gboolean changed=FALSE;
	switch (event->keyval)
	{
		case GDK_Up:
			curZ-=10.0/curScale;
			changed=TRUE;
			break;
		case GDK_Down:
			curZ+=10.0/curScale;
			changed=TRUE;
			break;
		case GDK_Left:
			curX-=10.0/curScale;
			changed=TRUE;
			break;
		case GDK_Right:
			curX+=10.0/curScale;
			changed=TRUE;
			break;
		case GDK_Page_Up:
			curScale+=1.0;
			if (curScale>5.0)
				curScale=5.0;
			changed=TRUE;
			break;
		case GDK_Page_Down:
			curScale-=1.0;
			if (curScale<1.0)
				curScale=1.0;
			changed=TRUE;
			break;
	}
	if (changed)
	{
		gdk_window_invalidate_rect(widget->window,NULL,FALSE);
		return TRUE;
	}
	return FALSE;
}


void createMapViewer(const gchar *path)
{
	world=path;
	//map window
	GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GFile *file=g_file_new_for_path(path);
	char *title=g_file_get_basename(file);
	gtk_window_set_title(GTK_WINDOW(win),title);
	gtk_window_set_icon(GTK_WINDOW(win),gdk_pixbuf_new_from_xpm_data(icon));
	g_signal_connect(G_OBJECT(win),"destroy",
		G_CALLBACK(destroy),NULL);
	g_free(title);
	g_object_unref(file);

	//main vbox
	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(win),vbox);

	//control hbox
	GtkWidget *hbox=gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	//slider
	GtkWidget *slider=gtk_hscale_new_with_range(0.0,127.0,1.0);
	gtk_box_pack_start(GTK_BOX(hbox),slider,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(slider),"format-value",
		G_CALLBACK(getSliderText),NULL);

	//icon toggle
//	GtkWidget *icons=gtk_toggle_button_new_with_label("Icons");
//	gtk_box_pack_end(GTK_BOX(hbox),icons,FALSE,TRUE,0);

	//map
	GtkWidget *da=gtk_drawing_area_new();
	curWidth=496;
	curHeight=400;
	gtk_drawing_area_size(GTK_DRAWING_AREA(da),curWidth,curHeight);
	gtk_box_pack_start(GTK_BOX(vbox),da,TRUE,TRUE,0);
	gtk_widget_add_events(da,GDK_SCROLL_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(da),"expose-event",
		G_CALLBACK(drawMap),NULL);
	g_signal_connect(G_OBJECT(da),"button-press-event",
		G_CALLBACK(mouseDown),NULL);
	g_signal_connect(G_OBJECT(da),"button-release-event",
		G_CALLBACK(mouseUp),NULL);
	g_signal_connect(G_OBJECT(da),"motion-notify-event",
		G_CALLBACK(mouseMove),NULL);
	g_signal_connect(G_OBJECT(da),"scroll-event",
		G_CALLBACK(mouseWheel),NULL);
	g_signal_connect(G_OBJECT(da),"key-press-event",
		G_CALLBACK(keyDown),NULL);
	gtk_widget_set_can_focus(da,TRUE);
	gtk_widget_grab_focus(da);

	g_signal_connect(G_OBJECT(slider),"value-changed",
		G_CALLBACK(adjustMap),G_OBJECT(da));

	int spawnX,spawnY,spawnZ;
	GetSpawn(path,&spawnX,&spawnY,&spawnZ);
	curX=spawnX;
	curZ=spawnZ;

	bits=g_malloc(curWidth*curHeight*4);

	//and show it
	gtk_widget_show_all(win);
}

