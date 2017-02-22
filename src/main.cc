/*! \file main.cc
 *
 * Main entry point for the meactl application.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "meactl-window.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MeactlWindow win;
	win.show();
	return app.exec();
}
