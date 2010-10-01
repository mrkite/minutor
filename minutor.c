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
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "mapviewer.h"
#include "minutor.xpm"

static GtkWidget *mainWindow;
static GtkWidget *standardWorld, *customWorld, *worlds;
static char *worldPaths[5];
static int worldIdxs[5];
char *customFile=NULL;

static int dontDestroy=FALSE;
static void destroy()
{
	if (!dontDestroy)
		gtk_main_quit();
}

static void doView()
{
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(standardWorld)))
	{
		char *path=worldPaths[worldIdxs[gtk_combo_box_get_active(GTK_COMBO_BOX(worlds))]];
		if (path!=NULL)
		{
			createMapViewer(worldPaths[worldIdxs[gtk_combo_box_get_active(GTK_COMBO_BOX(worlds))]]);
			dontDestroy=TRUE;
			gtk_widget_destroy(mainWindow);
		}
	}
	else
	{
		createMapViewer(customFile);
		dontDestroy=TRUE;
		gtk_widget_destroy(mainWindow);
	}
}

static void worldChanged()
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(standardWorld),TRUE);
}
static void customToggle()
{
	if (customFile==NULL)
		worldChanged();
}
static void customChanged(GtkFileChooser *chooser)
{
	if (customFile!=NULL)
		g_free(customFile);
	GFile *file=gtk_file_chooser_get_file(chooser);
	GFile *parent=g_file_get_parent(file);
	if (parent!=NULL)
	{
		customFile=g_file_get_path(parent);
		g_object_unref(parent);
	}
	else
		customFile=g_strdup("/");
	g_object_unref(file);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(customWorld),TRUE);
}

int main(int argc,char *argv[])
{
	gtk_init(&argc,&argv);

	//set up world paths
	gchar *root=g_strdup_printf("%s/.minecraft/saves",g_get_home_dir());
	for (int i=0;i<5;i++)
		worldPaths[i]=NULL;
	GDir *dir=g_dir_open(root,0,NULL);
	if (dir!=NULL)
	{
		const gchar *name;
		while ((name=g_dir_read_name(dir))!=NULL)
		{
			int world=-1;
			if (!g_strcmp0(name,"World1")) world=0;
			else if (!g_strcmp0(name,"World2")) world=1;
			else if (!g_strcmp0(name,"World3")) world=2;
			else if (!g_strcmp0(name,"World4")) world=3;
			else if (!g_strcmp0(name,"World5")) world=4;
			if (world!=-1)
				worldPaths[world]=g_strdup_printf("%s/%s",root,name);
		}
		g_dir_close(dir);
	}
	g_free(root);

	//main window
	mainWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(mainWindow),"Choose World");
	gtk_window_set_default_size(GTK_WINDOW(mainWindow),300,150);
	
	gtk_window_set_icon(GTK_WINDOW(mainWindow),gdk_pixbuf_new_from_xpm_data(icon));

	g_signal_connect(G_OBJECT(mainWindow),"destroy",
		G_CALLBACK(destroy),NULL);
	gtk_container_border_width(GTK_CONTAINER(mainWindow),10);

	//main vbox
	GtkWidget *vbox=gtk_vbox_new(FALSE,5);
	gtk_container_add(GTK_CONTAINER(mainWindow),vbox);

	//standard world hbox
	GtkWidget *swbox=gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),swbox,FALSE,TRUE,0);

	//standard world radio button
	standardWorld=gtk_radio_button_new(NULL);
	gtk_box_pack_start(GTK_BOX(swbox),standardWorld,FALSE,TRUE,0);

	//standard world combobox
	worlds=gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(swbox),worlds,TRUE,TRUE,0);

	int curWorld=0;
	for (int i=0;i<5;i++)
	{
		if (worldPaths[i]!=NULL)
		{
			gchar *str=g_strdup_printf("World %d",i+1);
			gtk_combo_box_append_text(GTK_COMBO_BOX(worlds),str);
			worldIdxs[curWorld++]=i;
			g_free(str);
		}
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(worlds),0);
	g_signal_connect(G_OBJECT(worlds),"changed",
		G_CALLBACK(worldChanged),NULL);

	//custom world hbox
	GtkWidget *cwbox=gtk_hbox_new(FALSE,5);
	gtk_box_pack_start(GTK_BOX(vbox),cwbox,FALSE,TRUE,0);

	//custom world radio button
	customWorld=gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(standardWorld));
	gtk_box_pack_start(GTK_BOX(cwbox),customWorld,FALSE,TRUE,0);
	g_signal_connect(G_OBJECT(customWorld),"toggled",
		G_CALLBACK(customToggle),NULL);

	//custom world file selection
	GtkWidget *customButton=gtk_file_chooser_button_new("Select World's level.dat",
		GTK_FILE_CHOOSER_ACTION_OPEN);
	GtkFileFilter *filter=gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter,"level.dat");
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(customButton),filter);
	gtk_box_pack_start(GTK_BOX(cwbox),customButton,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(customButton),"selection-changed",
		G_CALLBACK(customChanged),NULL);


	//view button buttonbox
	GtkWidget *buttons=gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX(buttons),GTK_BUTTONBOX_END);
	gtk_box_pack_end(GTK_BOX(vbox),buttons,FALSE,FALSE,0);

	//view button	
	GtkWidget *viewButton=gtk_button_new_with_label("View");
	g_signal_connect(G_OBJECT(viewButton),"clicked",
		G_CALLBACK(doView),NULL);
	gtk_container_add(GTK_CONTAINER(buttons),viewButton);


	//and show the main window
	gtk_widget_show_all(mainWindow);

	gtk_main();
	return 0;
}
