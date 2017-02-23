/*! \file meactl-window.h
 *
 * Header for the MeactlWindow class.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEACTL_WINDOW_H
#define MEACTL_WINDOW_H

#include "meactl-widget.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

/*! \class MeactlWindow
 *
 * The MeactlWindow is the top-level main window in the `meactl` application.
 * It is little more than a status bar and a container for a
 * MeactlWidget, which does the vast majority of the heavy lifting for
 * the application.
 */
class MeactlWindow : public QMainWindow {
	Q_OBJECT

		/*! Timeout for status bar messages */
		const int StatusMessageTimeout = 5000;

	public:
		
		/*! Construct the window.
		 *
		 * \param parent The parent widget. */
		MeactlWindow(QWidget* parent = nullptr);

		/*! Destroy a window. */
		~MeactlWindow();

		/* Copying is not allowed. */
		MeactlWindow(const MeactlWindow&) = delete;
		MeactlWindow(MeactlWindow&&) = delete;
		MeactlWindow& operator=(const MeactlWindow&) = delete;

	public slots:

		/*! Slot called when the server connection is successfully
		 * made or fails.
		 */
		void handleServerConnection(bool made);

		/*! Slot called when the server disconnects. */
		void handleServerDisconnection();

		/*! Slot called when a data source is created.
		 *
		 * \param success True if the source was created successfully, else false.
		 * \param msg An error message if the request failed.
		 */
		void handleSourceCreated(bool success, const QString& msg);

		/*! Slot called when a data source is deleted.
		 *
		 * \param success True if the source was deleted successfully, else false.
		 * \param msg An error message if the request failed.
		 */
		void handleSourceDeleted(bool success, const QString& msg);

		/*! Slot called when a recording is started.
		 *
		 * \param success True if the source was created successfully, else false.
		 * \param msg An error message if the request failed.
		 */
		void handleRecordingStarted(bool success, const QString& msg);

		/*! Slot called when a recording is stopped.
		 *
		 * \param success True if the source was created successfully, else false.
		 * \param msg An error message if the request failed.
		 */
		void handleRecordingStopped(bool success, const QString& msg);

		/*! Slot called when the file to which data is saved changed.
		 *
		 * \param name The new filename.
		 */
		void handleSaveFilenameChanged(const QString& name);

		/*! Slot called when the directory in which data is saved changes.
		 *
		 * \param name The name of the new directory.
		 */
		void handleSaveDirectoryChanged(const QString& name);

		/*! Slot called when a pending connection to the BLDS is canceled. */
		void handleServerConnectionCanceled();

	private:

		/* The actual controller widget, which does all the work. */
		MeactlWidget* controller;

};

#endif

