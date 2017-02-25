/*! \file main.cc
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "meactl-window.h"

/*! \fn * Main entry point for the meactl application.
 *
 * This creates a Qt application and a MeactlWindow object, which
 * handles all the remote interaction with the BLDS.
 */
int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MeactlWindow win;
	win.show();
	return app.exec();
}
