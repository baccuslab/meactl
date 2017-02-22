/*! \file meactl-widget.h
 *
 * Implementation of main widget in the meactl application.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEACTL_WIDGET_H
#define MEACTL_WIDGET_H

#include "libblds-client/include/blds-client.h"

#include <QtCore>
#include <QtWidgets>

class MeactlWidget : public QWidget {
	Q_OBJECT

		const int RecordingPositionUpdateInterval = 1000;

	public:
		MeactlWidget(QWidget* parent = nullptr);
		~MeactlWidget();

		MeactlWidget(const MeactlWidget&) = delete;
		MeactlWidget(MeactlWidget&&) = delete;
		MeactlWidget& operator=(const MeactlWidget&) = delete;

	private slots:

		/*! Connect to the Baccus Lab Data Server. */
		void connectToServer();

		/*! Disconnect from the BLDS. */
		void disconnectFromServer();

		/*! Create a data source.
		 *
		 * This uses the current source type selected from the combo box
		 * and the current location entered in the location line to request
		 * that the BLDS create a data source.
		 */
		void createDataSource();

		/*! Delete a data source. */
		void deleteDataSource();

		/*! Request that the BLDS start recording data. */
		void startRecording();

		/*! Request that the BLDS stop recording data. */
		void stopRecording();

		/*! Send the current recording length to the BLDS. */
		void setRecordingLength(int length);

		/*! Send the given filename to the BLDS, requesting that it save
		 * future data to that location.
		 *
		 * \param file The name of the file to which the BLDS should save data.
		 */
		void setRecordingFilename(const QString& file);

		/*! Open a dialog to select an analog output file, and send the encoded
		 * signal to the BLDS.
		 */
		void chooseAnalogOutput();

		/*! Slot called when a connection to the server completes or fails.
		 * 
		 * \param made True if the connection succeeded, else false.
		 */
		void onServerConnection(bool made);

		/*! Slot called when an error is received from the server.
		 *
		 * \param msg The received error message.
		 */
		void onServerError(const QString& msg);

		/*! Slot called when a response to create a source is received.
		 *
		 * \param success True if the source was created, false otherwise.
		 * \param msg If the source could not be created, this contains an error
		 * 	message from the BLDS.
		 */
		void onSourceCreated(bool success, const QString& msg);

		/*! Slot called when a response to delete a source is received.
		 *
		 * \param success True if the source was deleted, false otherwise.
		 * \param msg If the source could not be deleted, this contains an error
		 * 	message from the BLDS.
		 */
		void onSourceDeleted(bool success, const QString& msg);

		/*! Slot called when a response to start the recording is received.
		 *
		 * \param success True if the recording started, false otherwise.
		 * \param msg If the request failed, this contains an error message
		 * 	from the BLDS.
		 */
		void onRecordingStarted(bool success, const QString& msg);

		/*! Slot called when a response to stop the recording is received.
		 *
		 * \param success True if the recording stopped, false otherwise.
		 * \param msg If the request failed, this contains an error message
		 * 	from the BLDS.
		 */
		void onRecordingStopped(bool success, const QString& msg);

		/*! Slot called when receiving the initial status reply from the server
		 * just after connecting.
		 *
		 * \param json The response data containing the server's status.
		 */
		void handleInitialStatusReply(const QJsonObject& json);

		/*! Choose a directory in which to save a recording. */
		void chooseRecordingDirectory();

		/*! Slot called to handle the heartbeat requests for the server's status.
		 *
		 * \param json The JSON object containing the server's status.
		 */
		void handleRecordingStatusReply(const QJsonObject& json);

		/*! Slot called to cancel a pending connection to the BLDS. */
		void cancelPendingServerConnection();

	signals:

		/*! Emitted after an attempt to connect to the BLDS.
		 *
		 * \param made True if the connected was made, false if it failed.
		 */
		void connectedToServer(bool made);

		/*! Emitted upon receipt of an error message from the BLDS.
		 *
		 * \param msg The error message.
		 */
		void serverError(const QString& msg);

		/*! Emitted after disconnecting from the BLDS. */
		void disconnectedFromServer();

		/*! Emitted upon receipt of a response to a request to create a data source.
		 *
		 * \param success True if the request succeeded, else false.
		 * \param If the request failed, this contains an error message.
		 */
		void sourceCreated(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to delete a data source.
		 *
		 * \param success True if the request succeeded, else false.
		 * \param If the request failed, this contains an error message.
		 */
		void sourceDeleted(bool success, const QString& msg);

		/*! Emitted when the recording directory is successfully changed.
		 *
		 * \param dir The new recording directory.
		 */
		void recordingDirectoryChanged(const QString& dir);

		/*! Emitted upon receipt of a response to a request to start the recording.
		 *
		 * \param success True if the request succeeded, else false.
		 * \param msg If the request failed, this contains an error message.
		 */
		void recordingStarted(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to stop the recording.
		 *
		 * \param success True if the request succeeded, else false.
		 * \param msg If the request failed, this contains an error message.
		 */
		void recordingStopped(bool success, const QString& msg);

	private:

		/* Initialize the user interface. */
		void setupLayout();

		/* Connect initial signals. */
		void initSignals();

		/* Get the status of the server and any data source after initially connecting. */
		void getInitialStatus();

		/* Setup a heartbeat HTTP request to the server to get the status
		 * of the recording, and connect slots to handle the response.
		 */
		void setupRecordingStatusHeartbeat();

		/* Handler which implements changes to the control widget in response
		 * to a successful connection to the server.
		 */
		void handleServerConnection();

		/* Handler which implements changes to the control widget in response
		 * to a disconnection from the server.
		 */
		void handleServerDisconnection();

		/* Handler which implements changes to the control widget in response
		 * to a successful creation of a data source.
		 */
		void handleSourceCreated();

		/* Handler which implements changes to the control widget in response
		 * to a successful deletion of a data source.
		 */
		void handleSourceDeleted();

		/* Handler which implements changes to the control widget in response
		 * to a successful start to a recording.
		 */
		void handleRecordingStarted();

		/* Handler which implements changes to the control widget in response
		 * to a successful stop to a recording.
		 */
		void handleRecordingStopped();

		/*! Main widget layout. */
		QGridLayout* mainLayout;

		/*! Group containing widgets related to the server. */
		QGroupBox* serverGroup;

		/*! Layout manager for the server group. */
		QGridLayout* serverLayout;

		/*! Labels the hostname line. */
		QLabel* serverHostLabel;

		/*! Line containing the hostname/IP of the BLDS. */
		QLineEdit* serverHostLine;

		/*! Button for connecting to / disconnecting from the BLDS. */
		QPushButton* connectToServerButton;

		/*! Group of widgets relating to the status of the data source. */
		QGroupBox* sourceGroup;

		/*! Layout manager for the source group. */
		QGridLayout* sourceLayout;

		/*! Labels the combo box showing source type. */
		QLabel* sourceTypeLabel;

		/*! Combo box for selecting the type of data source to create. */
		QComboBox* sourceTypeBox;

		/*! Labels the line giving location of the source. */
		QLabel* sourceLocationLabel;

		/*! Gives the location of the source. */
		QLineEdit* sourceLocationLine;

		/*! Button to create/delete data source. */
		QPushButton* createSourceButton;

		/*! Button for showing the source's settings. */
		QPushButton* showSettingsButton;

		/*! Sub-window which can be used to show/manipulate the data
		 * source settings.
		 */
		//QPointer<SourceSettingsWindow> settingsWindow;

		/*! Group of widgets related to a recording. */
		QGroupBox* recordingGroup;

		/*! Layout manager for recording group. */
		QGridLayout* recordingLayout;

		/*! Labels the line showing the full length of the recording. */
		QLabel* recordingLengthLabel;

		/*! Shows the full length of the recording. */
		QLineEdit* recordingLengthLine;

		/*! Labels line showing the current position in the recording. */
		QLabel* recordingPositionLabel;

		/*! Shows current position in recording. */
		QLineEdit* recordingPositionLine;

		/*! Labels the line showing current file to which data is saved. */
		QLabel* recordingFileLabel;

		/*! Shows current file to which data is saved. */
		QLineEdit* recordingFile;

		/*! Button for setting the path to the recording file. */
		QPushButton* recordingPathButton;

		/*! Button for starting/stopping the recording. */
		QPushButton* startRecordingButton;

		/*! Client for communication with BLDS. */
		QPointer<BldsClient> client;

		/*! Timer for making periodic requests about the status of the
		 * server/source
		 */
		QTimer* recordingStatusTimer;
};

#endif

