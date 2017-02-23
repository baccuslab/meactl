/*! \file meactl-window.cc
 *
 * Implementation of MeactlWindow class.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "meactl-window.h"

MeactlWindow::MeactlWindow(QWidget* parent) :
	QMainWindow(parent)
{
	controller = new MeactlWidget(this);
	QObject::connect(controller, &MeactlWidget::connectedToServer,
			this, &MeactlWindow::handleServerConnection);
	QObject::connect(controller, &MeactlWidget::disconnectedFromServer,
			this, &MeactlWindow::handleServerDisconnection);
	QObject::connect(controller, &MeactlWidget::sourceCreated,
			this, &MeactlWindow::handleSourceCreated);
	QObject::connect(controller, &MeactlWidget::sourceDeleted,
			this, &MeactlWindow::handleSourceDeleted);
	QObject::connect(controller, &MeactlWidget::serverConnectionCanceled,
			this, &MeactlWindow::handleServerConnectionCanceled);
	QObject::connect(controller, &MeactlWidget::recordingStarted,
			this, &MeactlWindow::handleRecordingStarted);
	QObject::connect(controller, &MeactlWidget::recordingStopped,
			this, &MeactlWindow::handleRecordingStopped);
	QObject::connect(controller, &MeactlWidget::recordingDirectoryChanged,
			this, &MeactlWindow::handleSaveDirectoryChanged);
	QObject::connect(controller, &MeactlWidget::recordingFilenameChanged,
			this, &MeactlWindow::handleSaveFilenameChanged);

	setCentralWidget(controller);
	statusBar()->showMessage("Ready", StatusMessageTimeout);
}

MeactlWindow::~MeactlWindow()
{
}

void MeactlWindow::handleServerConnection(bool made)
{
	QString msg = made ? "Connected to BLDS" : "Could not connect to BLDS";
	statusBar()->showMessage(msg, StatusMessageTimeout);
}

void MeactlWindow::handleServerDisconnection()
{
	statusBar()->showMessage("Disconnected from BLDS", StatusMessageTimeout);
}

void MeactlWindow::handleSourceCreated(bool success, const QString& msg)
{
	QString status = success ? "Data source created" : "Could not create source";
	statusBar()->showMessage(status, StatusMessageTimeout);
	if (!success) {
		QMessageBox::critical(this, "Could not create source", msg);
	}
}

void MeactlWindow::handleSourceDeleted(bool success, const QString& msg)
{
	QString status = success ? "Data source deleted" : "Could not delete source";
	statusBar()->showMessage(status, StatusMessageTimeout);
	if (!success) {
		QMessageBox::critical(this, "Could not delete data source", msg);
	}
}

void MeactlWindow::handleRecordingStarted(bool success, const QString& msg)
{
	QString status = success ? "Recording started" : "Could not start recording";
	statusBar()->showMessage(status, StatusMessageTimeout);
	if (!success) {
		QMessageBox::critical(this, "Could not start recording", msg);
	}
}

void MeactlWindow::handleRecordingStopped(bool success, const QString& msg)
{
	QString status = success ? "Recording stopped" : "Could not stop recording";
	statusBar()->showMessage(status, StatusMessageTimeout);
	if (!success) {
		QMessageBox::critical(this, "Could not stop recording", msg);
	}
}

void MeactlWindow::handleSaveDirectoryChanged(const QString& dir)
{
	statusBar()->showMessage(QString("Save directory set to %1").arg(dir), 
			StatusMessageTimeout);
}

void MeactlWindow::handleServerConnectionCanceled()
{
	statusBar()->showMessage("Pending connection to BLDS canceled", 
			StatusMessageTimeout);
}

void MeactlWindow::handleSaveFilenameChanged(const QString& name)
{
	statusBar()->showMessage("Recording filename set to " + name,
			StatusMessageTimeout);
}


