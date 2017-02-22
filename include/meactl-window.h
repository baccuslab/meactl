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

class MeactlWindow : public QMainWindow {
	Q_OBJECT

		const int StatusMessageTimeout = 5000;

	public:
		MeactlWindow(QWidget* parent = nullptr);
		~MeactlWindow();

		MeactlWindow(const MeactlWindow&) = delete;
		MeactlWindow(MeactlWindow&&) = delete;
		MeactlWindow& operator=(const MeactlWindow&) = delete;

	public slots:
		void handleServerConnection(bool made);
		void handleServerDisconnection();
		void handleSourceCreated(bool success, const QString& msg);
		void handleSourceDeleted(bool success, const QString& msg);
		void handleRecordingStarted(bool success, const QString& msg);
		void handleRecordingStopped(bool success, const QString& msg);
		//void handleSaveFilenameChanged(bool success, const QString& name);
		void handleSaveDirectoryChanged(const QString& name);
		//void handleReadIntervalChanged(bool success, const QString& msg);

	private:
		MeactlWidget* controller;

};

#endif

