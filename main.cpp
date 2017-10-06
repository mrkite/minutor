/** Copyright (c) 2013, Sean Kasun */
#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLocale>

#include "./minutor.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QString locale = QLocale::system().name();

  QTranslator translator;
  translator.load(QString("minutor_")+locale);
  app.installTranslator(&translator);

  app.setApplicationName("Minutor");
  app.setApplicationVersion("2.1.3");
  app.setOrganizationName("seancode");

  Minutor minutor;

  // Process the cmdline arguments:
  QStringList args = app.arguments();
  int numArgs = args.size();
  int ex_top    = 0;
  int ex_left   = 0;
  int ex_bottom = 0;
  int ex_right  = 0;
  bool regionChecker = false;
  bool chunkChecker = false;
  for (int i = 0; i < numArgs; i++) {
    if (args[i].length() > 2) {
      // convert long variants to lower case
      args[i] = args[i].toLower();
    }
    if ((args[i] == "-w" || args[i] == "--world") && i + 1 < numArgs) {
      minutor.loadWorld(args[i + 1]);
      i += 1;
      continue;
    }
    if (args[i] == "--regionchecker") {
      regionChecker = true;
      continue;
    }
    if (args[i] == "--chunkchecker") {
      chunkChecker = true;
      continue;
    }
    if ((args[i] == "-r" || args[i] == "--exportrange") && i + 4 < numArgs) {
      ex_top    = args[i + 1].toInt();
      ex_left   = args[i + 2].toInt();
      ex_bottom = args[i + 3].toInt();
      ex_right  = args[i + 4].toInt();
      i += 4;
      continue;
    }
    if ((args[i] == "-s" || args[i] == "--savepng") && i + 1 < numArgs) {
      minutor.savePNG(args[i + 1], true, regionChecker, chunkChecker,
                      ex_top, ex_left, ex_bottom, ex_right);
      i += 1;
      continue;
    }
    if ((args[i] == "-j" || args[i] == "--jump") && i + 2 < numArgs) {
      minutor.jumpToXZ(args[i + 1].toInt(), args[i + 2].toInt());
      i += 2;
      continue;
    }
    if ((args[i] == "-y" || args[i] == "--depth") && i + 1 < numArgs) {
      minutor.setDepth(args[i + 1].toInt());
      i += 1;
      continue;
    }

    // menu View->
    if (args[i] == "-L" || args[i] == "--lighting") {
      minutor.setViewLighting(true);
      continue;
    }
    if (args[i] == "-M" || args[i] == "--mobspawning") {
      minutor.setViewMobspawning(true);
      continue;
    }
    if (args[i] == "-D" || args[i] == "--depthshading") {
      minutor.setViewDepthshading(true);
      continue;
    }
    if (args[i] == "-B" || args[i] == "--biomecolors") {
      minutor.setViewBiomeColors(true);
      continue;
    }
  }

  minutor.show();
  return app.exec();
}
