/*! \file meactl-widget.cc
	} else if (param == 
 *
 * Implementation of main controlling widget.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "meactl-widget.h"

#include "source-settings-window.h"

MeactlWidget::MeactlWidget(QWidget* parent) :
	QWidget(parent)
{
	setupLayout();
	initSignals();

	/* Create timer for status heartbeats during a recording. */
	recordingStatusTimer = new QTimer(this);
	recordingStatusTimer->setInterval(RecordingPositionUpdateInterval);
}

MeactlWidget::~MeactlWidget()
{
}

void MeactlWidget::setupLayout()
{
	/* Create overall layout for widget. */
	mainLayout = new QGridLayout(this);

	/* Group of widgets related to the BLDS. */
	serverGroup = new QGroupBox("Server", this);
	serverLayout = new QGridLayout(serverGroup);
	serverHostLabel = new QLabel("Hostname:", serverGroup);
	serverHostLabel->setAlignment(Qt::AlignRight);
	serverHostLine = new QLineEdit("localhost", serverGroup);
	serverHostLine->setToolTip("Hostname or IP address of the BLDS");
	connectToServerButton = new QPushButton("Connect", serverGroup);
	connectToServerButton->setToolTip("Connect to the BLDS");
	serverLayout->addWidget(serverHostLabel, 0, 0);
	serverLayout->addWidget(serverHostLine, 0, 1, 1, 2);
	serverLayout->addWidget(connectToServerButton, 0, 3);

	/* Widgets related to manipulating the data source. */
	sourceGroup = new QGroupBox("Data source", this);
	sourceLayout = new QGridLayout(sourceGroup);
	sourceTypeLabel = new QLabel("Type:", sourceGroup);
	sourceTypeLabel->setAlignment(Qt::AlignRight);
	sourceTypeBox = new QComboBox(sourceGroup);
	sourceTypeBox->addItems({"file", "hidens", "mcs"});
	sourceTypeBox->setToolTip("Type of data source to create");
	sourceLocationLabel = new QLabel("Location:", sourceGroup);
	sourceLocationLabel->setAlignment(Qt::AlignRight);
	sourceLocationLine = new QLineEdit("", sourceGroup);
	sourceLocationLine->setToolTip("Location of data source (file, IP address, etc.)");
	createSourceButton = new QPushButton("Create", sourceGroup);
	createSourceButton->setToolTip("Create a data source of the selected type");
	createSourceButton->setEnabled(false);
	showSettingsButton = new QPushButton("Settings", sourceGroup);
	showSettingsButton->setEnabled(false);
	sourceLayout->addWidget(sourceTypeLabel, 0, 0);
	sourceLayout->addWidget(sourceTypeBox, 0, 1);
	sourceLayout->addWidget(createSourceButton, 0, 2);
	sourceLayout->addWidget(showSettingsButton, 0, 3);
	sourceLayout->addWidget(sourceLocationLabel, 1, 0);
	sourceLayout->addWidget(sourceLocationLine, 1, 1, 1, 3);

	/* Widgets related to the actual recording. */
	recordingGroup = new QGroupBox("Recording", this);
	recordingLayout = new QGridLayout(recordingGroup);
	recordingLengthLabel = new QLabel("Length:", recordingGroup);
	recordingLengthLabel->setAlignment(Qt::AlignRight);
	recordingLengthLine = new QLineEdit("1000", recordingGroup);
	recordingLengthLine->setToolTip("Total length of recording");
	recordingLengthLine->setAlignment(Qt::AlignRight);
	auto validator = new QIntValidator(1, 50000, recordingLengthLine);
	recordingLengthLine->setValidator(validator);
	recordingPositionLabel = new QLabel("Position:", recordingGroup);
	recordingPositionLabel->setAlignment(Qt::AlignRight);
	recordingPositionLine = new QLineEdit("0", recordingGroup);
	recordingPositionLine->setAlignment(Qt::AlignRight);
	recordingPositionLine->setReadOnly(true);
	recordingPositionLine->setToolTip("Current time in recording");
	recordingFileLabel = new QLabel("Filename:", recordingGroup);
	recordingFileLabel->setAlignment(Qt::AlignRight);
	recordingFileLine = new QLineEdit("", recordingGroup);
	recordingFileLine->setToolTip("Filename at which to save data");
	recordingPathButton = new QPushButton("Path", recordingGroup);
	recordingPathButton->setEnabled(false);
	recordingPathButton->setToolTip("Choose directory in which data is saved");
	startRecordingButton = new QPushButton("Start", recordingGroup);
	startRecordingButton->setToolTip("Start the recording");
	startRecordingButton->setEnabled(false);
	recordingLayout->addWidget(recordingPositionLabel, 0, 0);
	recordingLayout->addWidget(recordingPositionLine, 0, 1);
	recordingLayout->addWidget(recordingLengthLabel, 0, 2);
	recordingLayout->addWidget(recordingLengthLine, 0, 3);
	recordingLayout->addWidget(recordingFileLabel, 1, 0);
	recordingLayout->addWidget(recordingFileLine, 1, 1);
	recordingLayout->addWidget(recordingPathButton, 1, 2);
	recordingLayout->addWidget(startRecordingButton, 1, 3);

	/* Place all widgets in main layout. */
	mainLayout->addWidget(serverGroup, 0, 0);
	mainLayout->addWidget(sourceGroup, 1, 0);
	mainLayout->addWidget(recordingGroup, 2, 0);
}

void MeactlWidget::initSignals()
{
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::connectToServer);
	QObject::connect(showSettingsButton, &QPushButton::clicked,
			this, &MeactlWidget::showSettingsWindow);
}

void MeactlWidget::connectToServer()
{
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::connectToServer);

	/* Change UI to allow canceling a pending connection request. */
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::cancelPendingServerConnection);
	connectToServerButton->setText("Cancel");
	connectToServerButton->setToolTip("Cancel pending connection to the BLDS");
	serverHostLine->setReadOnly(true);

	/* Connect handler for the result of a connection attempt. */
	client = new BldsClient(serverHostLine->text());
	QObject::connect(client, &BldsClient::connected,
			this, &MeactlWidget::onServerConnection);
	client->connect();
}

void MeactlWidget::onServerConnection(bool made)
{
	/* Disconnect this handler */
	QObject::disconnect(client, &BldsClient::connected, 0, 0);

	if (made) {
		handleServerConnection();
	} else {
		serverHostLine->setReadOnly(false);
		QObject::connect(connectToServerButton, &QPushButton::clicked,
				this, &MeactlWidget::connectToServer);
		client->deleteLater();
	}

	/* Notify. */
	emit connectedToServer(made);
}

void MeactlWidget::handleServerConnection()
{
	/* Enable disconnecting from the server */
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::cancelPendingServerConnection);
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::disconnectFromServer);
	QObject::connect(client, &BldsClient::disconnected,
			[this]() -> void {
				handleServerDisconnection();
				client->deleteLater();
				emit disconnectedFromServer();
			});
	connectToServerButton->setText("Disconnect");

	/* Connect signals of the client. */
	QObject::connect(client, &BldsClient::sourceCreated,
			this, &MeactlWidget::onSourceCreated);
	QObject::connect(client, &BldsClient::sourceDeleted,
			this, &MeactlWidget::onSourceDeleted);
	QObject::connect(client, &BldsClient::recordingStarted,
			this, &MeactlWidget::onRecordingStarted);
	QObject::connect(client, &BldsClient::recordingStopped,
			this, &MeactlWidget::onRecordingStopped);
	QObject::connect(client, &BldsClient::error,
			this, &MeactlWidget::onServerError);

	/* Setup choosing the recording path. */
	QObject::connect(recordingPathButton, &QPushButton::clicked,
			this, &MeactlWidget::chooseRecordingDirectory);

	/* Connect functor for sending a new recording filename. */
	QObject::connect(recordingFileLine, &QLineEdit::returnPressed,
			[this]() -> void {
				client->set("save-file", recordingFileLine->text());
			});
	recordingPathButton->setEnabled(true);

	/* Connect functor for handling a response to a request
	 * to change the length of a recording.
	 */
	recordingLengthLine->setEnabled(true);
	connections.insert("recording-length-response",
			QObject::connect(client, &BldsClient::setResponse,
			[this](const QString& param, bool success, const QString& msg) -> void {
				if (param != "recording-length") {
					return;
				}
				if (success) {
					emit recordingLengthChanged(recordingLengthLine->text());
				} else {
					QMessageBox::warning(parentWidget(), 
							"Could not change recording length",
							"An error occurred changing the recording length: " + msg);
				}
			})
	);

	/* Connect functor for handling a response to a request
	 * to change the save filename.
	 */
	recordingFileLine->setEnabled(true);
	connections.insert("recording-filename-response",
			QObject::connect(client, &BldsClient::setResponse,
			[this](const QString& param, bool success, const QString& msg) -> void {
				if (param != "save-file") {
					return;
				}
				if (success) {
					emit recordingFilenameChanged(recordingFileLine->text());
				} else {
					QMessageBox::warning(parentWidget(), "Could not set filename",
							"The recording filename could not be set. " + msg);
				}
			})
	);

	/* Connect the handler for the initial status reply from the
	 * BLDS, and request that status.
	 */
	QObject::connect(client, &BldsClient::serverStatus,
			this, &MeactlWidget::handleInitialStatusReply);
	client->requestServerStatus();
}

void MeactlWidget::onServerError(const QString& err)
{
	handleServerDisconnection();
	client->deleteLater();
	QMessageBox::critical(parentWidget(), "Server error", 
			"An error occurred communicating with the BLDS:\n\n" + err);
	emit serverError(err);
}

void MeactlWidget::disconnectFromServer()
{
	handleServerDisconnection();

	/* Disconnect and delete the client. */
	client->disconnect();
	client->deleteLater();

	/* Notify. */
	emit disconnectedFromServer();
}

void MeactlWidget::handleServerDisconnection()
{
	/* Disconnect pretty much all signals/slots. */
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::disconnectFromServer);
	QObject::disconnect(recordingPathButton, &QPushButton::clicked,
			this, &MeactlWidget::chooseRecordingDirectory);
	QObject::disconnect(createSourceButton, &QPushButton::clicked, 0, 0);
	QObject::disconnect(recordingLengthLine, &QLineEdit::returnPressed, 0, 0);
	QObject::disconnect(startRecordingButton, &QPushButton::clicked, 0, 0);
	QObject::disconnect(recordingFileLine, &QLineEdit::returnPressed, 0, 0);
	QObject::disconnect(showSettingsButton, &QPushButton::clicked, 0, 0);

	/* Remove any residual other functors. */
	for (auto& each : connections)
		QObject::disconnect(each);
	connections.clear();

	/* Re-connect slot to connect to the server. */
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::connectToServer);

	/* Disconnect functor for manipulating the recording filename. */
	QObject::disconnect(recordingFileLine, &QLineEdit::returnPressed, 0, 0);
	if (connections.contains("recording-filename-response"))
		QObject::disconnect(connections.take("recording-filename-response"));

	/* Reset most of the UI. */
	connectToServerButton->setText("Connect");
	serverHostLine->setReadOnly(false);

	createSourceButton->setText("Create");
	createSourceButton->setToolTip("Create data source with the given type");
	sourceLocationLine->setReadOnly(false);
	createSourceButton->setEnabled(false);
	showSettingsButton->setEnabled(false);
	startRecordingButton->setEnabled(false);
	recordingPathButton->setEnabled(false);

	recordingPositionLine->setText("0");
	sourceTypeBox->setEnabled(true);
}

void MeactlWidget::createDataSource()
{
	auto type = sourceTypeBox->currentText();
	auto location = (type == "mcs") ? "" : sourceLocationLine->text();
	client->createSource(type, location);
}

void MeactlWidget::onSourceCreated(bool success, const QString& msg)
{
	if (success) {
		handleSourceCreated();
	}
	/* Notify. */
	emit sourceCreated(success, msg);
}

void MeactlWidget::handleSourceCreated()
{
	/* Connect slot for deleting the data source, and change the
	 * "create" button to a "delete" button.
	 */
	QObject::disconnect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::createDataSource);
	QObject::connect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::deleteDataSource);
	createSourceButton->setText("Delete");
	createSourceButton->setToolTip("Delete current data source");

	/* Enable starting the recording. */
	startRecordingButton->setEnabled(true);
	QObject::connect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);

	showSettingsButton->setEnabled(true);
	sourceLocationLine->setReadOnly(true);
	sourceTypeBox->setEnabled(false);
}

void MeactlWidget::deleteDataSource()
{
	client->deleteSource();
}

void MeactlWidget::onSourceDeleted(bool success, const QString& msg)
{
	if (success) {
		handleSourceDeleted();
	}
	/* Notify */
	emit sourceDeleted(success, msg);
}

void MeactlWidget::handleSourceDeleted()
{
	/* Disconnect functor for checking if the source has been deleted
	 * in the situation when the recording is stopped by a client different
	 * from ourselves.
	 */
	if (connections.contains("source-exists-connection"))
		QObject::disconnect(connections.take("source-exists-connection"));

	/* Disconnect this slot, and change the "delete" button 
	 * back to a "create" button.
	 */
	QObject::disconnect(createSourceButton, &QPushButton::clicked, 
			this, &MeactlWidget::deleteDataSource);
	QObject::connect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::createDataSource);
	createSourceButton->setText("Create");
	createSourceButton->setToolTip("Create a data source of the selected type");
	showSettingsButton->setEnabled(false);

	/* Disable starting the recording. */
	startRecordingButton->setEnabled(false);
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);
	sourceLocationLine->setReadOnly(false);
	sourceTypeBox->setEnabled(true);
}

void MeactlWidget::startRecording()
{
	client->startRecording();
}

void MeactlWidget::onRecordingStarted(bool success, const QString& msg)
{
	if (success) {
		handleRecordingStarted();
	}
	/* Notify */
	emit recordingStarted(success, msg);
}

void MeactlWidget::handleRecordingStarted()
{
	/* Disable starting the recording, and enable stopping it. */
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);
	recordingLengthLine->setReadOnly(true);
	recordingFileLine->setReadOnly(true);
	QObject::connect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::stopRecording);
	startRecordingButton->setText("Stop");
	startRecordingButton->setToolTip("Stop current recording");

	/* Get the current recording filename, because other clients
	 * may have changed it and it changes each time a recording
	 * is started/stopped unless the client explicitly sets it.
	 */
	if (connections.contains("recording-filename-response"))
		QObject::disconnect(connections.take("recording-filename-response"));
	connections.insert("recording-filename-response",
			QObject::connect(client, &BldsClient::getResponse,
			[this](const QString& param, bool, const QVariant& data) -> void {
				if (param == "save-file") {
					recordingFileLine->setText(data.toString());
					if (connections.contains("recording-filename-response"))
						QObject::disconnect(connections.take("recording-filename-response"));
				}
			})
	);
	client->get("save-file");

	/* Disable creating a data source. */
	createSourceButton->setEnabled(false);

	setupRecordingStatusHeartbeat();
}

void MeactlWidget::stopRecording()
{
	client->stopRecording();
}

void MeactlWidget::onRecordingStopped(bool success, const QString& msg)
{
	if (success) {
		handleRecordingStopped();
	}
	/* Notify */
	emit recordingStopped(success, msg);
}

void MeactlWidget::handleRecordingStopped()
{
	/* Disconnect functors which periodically check that the recording
	 * exists. These are connected in setupRecordingStatusHearbeat().
	 */
	if (connections.contains("recording-exists-connection"))
		QObject::disconnect(connections.take("recording-exists-connection"));
	if (connections.contains("recording-position-connections"))
		QObject::disconnect(connections.take("recording-position-connections"));
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::stopRecording);

	/* Re-enable setting the length of the recording. The signal has
	 * already been connected, just make it editable.
	 */
	recordingLengthLine->setReadOnly(false);
	recordingPositionLine->setText("0");

	/* Reconnect setting the filename. Here we reconnect the signal
	 * because when starting a recording a different one is connected
	 * in case other clients set the filename before we start.
	 */
	recordingFileLine->setReadOnly(false);
	if (connections.contains("recording-filename-response"))
		QObject::disconnect(connections.take("recording-filename-response"));
	connections.insert("recording-filename-response",
			QObject::connect(client, &BldsClient::setResponse,
			[this](const QString& param, bool success, const QString& msg) -> void {
				if (param != "save-file") {
					return;
				}
				if (success) {
					emit recordingFilenameChanged(recordingFileLine->text());
				} else {
					QMessageBox::warning(parentWidget(), "Could not set filename",
							"The recording filename could not be set. " + msg);
				}
			})
	);


	/* Re-enable starting the recording. */
	QObject::connect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);
	startRecordingButton->setText("Start");
	startRecordingButton->setToolTip("Start a recording");

	/* Re-enable deleting the source. The widgets/UI should be set up
	 * for deleting at this point.
	 */
	createSourceButton->setEnabled(true);

	/* Disable the periodic requests to the server for the status. */
	QObject::disconnect(recordingStatusTimer, &QTimer::timeout, 0, 0);
	recordingStatusTimer->stop();
}

void MeactlWidget::setRecordingLength(int len)
{
	if (client)
		client->set("recording-length", len);
}

void MeactlWidget::setRecordingFilename(const QString& name)
{
	if (client)
		client->set("save-file", name);
}

void MeactlWidget::handleInitialStatusReply(QJsonObject json)
{
	/* Determine basic information about the server and source. */
	auto sourceExists = json["source-exists"].toBool();
	auto recordingExists = json["recording-exists"].toBool();
	auto length = json["recording-length"].toInt();
	auto position = json["recording-position"].toDouble();

	/* Recording file should always be read from server, regardless
	 * of whether a source or recording exists.
	 */
	recordingFileLine->setText(json["save-file"].toString());

	/* Functor for updating sending an updated recording length. */
	auto onLengthUpdate = [this]() -> void {
		setRecordingLength(recordingLengthLine->text().toInt());
	};

	if (sourceExists) {
		
		/* Show the source type and disable selecting a new one. */
		sourceTypeBox->setCurrentText(json["source-type"].toString());
		sourceTypeBox->setEnabled(false);
		sourceLocationLine->setReadOnly(true);
		sourceLocationLine->setText(json["source-location"].toString());

		/* Switch create source button to delete the source. */
		createSourceButton->setText("Delete");
		createSourceButton->setToolTip("Delete current data source");
		QObject::connect(createSourceButton, &QPushButton::clicked,
				this, &MeactlWidget::deleteDataSource);

		/* Enable showing the source settings. */
		showSettingsButton->setEnabled(true);

		if (recordingExists) {

			/* Show the current position in the recording and disable
			 * setting a new length.
			 */
			recordingLengthLine->setText(QString("%1").arg(length));
			recordingLengthLine->setReadOnly(true);
			recordingFileLine->setReadOnly(true);
			recordingPositionLine->setText(QString::number(position, 'f', 1));

			/* Enable stopping the recording. */
			startRecordingButton->setEnabled(true);
			startRecordingButton->setText("Stop");
			startRecordingButton->setToolTip("Stop the current recording");
			QObject::connect(startRecordingButton, &QPushButton::clicked,
					this, &MeactlWidget::stopRecording);

			/* Disable deleting the source. */
			createSourceButton->setEnabled(false);

			/* Setup periodic status requests to the server. */
			setupRecordingStatusHeartbeat();

		} else {

			/* Show current final length of the recording, enable changing it. */
			recordingLengthLine->setText(QString("%1").arg(length));
			QObject::connect(recordingLengthLine, &QLineEdit::returnPressed,
					onLengthUpdate);

			/* Enable starting the recording. */
			startRecordingButton->setEnabled(true);
			QObject::connect(startRecordingButton, &QPushButton::clicked,
					this, &MeactlWidget::startRecording);
			
			createSourceButton->setEnabled(true);
		}
	} else {
		/* No source, enable creating one. */
		createSourceButton->setEnabled(true);
		showSettingsButton->setEnabled(false);
		QObject::connect(createSourceButton, &QPushButton::clicked,
				this, &MeactlWidget::createDataSource);

		/* Set recording length and enable changing it. */
		recordingLengthLine->setText(QString("%1").arg(length));
		QObject::connect(recordingLengthLine, &QLineEdit::returnPressed,
				onLengthUpdate);
	}
}

void MeactlWidget::chooseRecordingDirectory()
{
	/* Open dialog to choose directory */
	auto dir = QFileDialog::getExistingDirectory(this, "Choose save directory",
			QDir::homePath());
	if (dir.isNull() || dir.size() == 0)
		return;

	if (client) {
		/* Connect handler to the response to set the directory. */
		connections.insert("save-directory-connection", 
				QObject::connect(client, &BldsClient::setResponse,
				[this,dir](const QString& param, bool success, 
						const QString& msg) -> void {
					if (param == "save-directory") {
						
						/* Disconnect this slot. */
						QObject::disconnect(client, &BldsClient::setResponse, 0, 0);

						/* Notify, just a status bar message if successful, else
						 * a full warning dialog.
						 */
						if (success) {
							emit recordingDirectoryChanged(dir);
						} else {
							QMessageBox::warning(parentWidget(), "Could not set save path",
									QString("Could not set the save directory. %1").arg(msg));
						}
						connections.remove("save-directory-connection");
					}
				})
			);

		/* Actually request to set the directory.*/
		client->set("save-directory", dir);
	}
}

void MeactlWidget::setupRecordingStatusHeartbeat()
{
	/* Periodically check that the recording still exists. */
	QObject::connect(recordingStatusTimer, &QTimer::timeout, 
			[this]() -> void {
				if (client) {
					client->get("recording-exists");
				}
			});

	/* Functor for handling replies to the periodic recording-exists request */
	connections.insert("recording-exists-connection", 
			QObject::connect(client, &BldsClient::getResponse, 
			[this](const QString& param, bool, 
					const QVariant& data) -> void {
				if (param != "recording-exists") {
					return;
				}
				handleRecordingExistsReply(data.toBool());
			})
	);

	/* Functor for handling periodic requests for the recording position.
	 * Note that these requests are actually made inside the method
	 * handleRecordingExistsReply().
	 */
	connections.insert("recording-position-connection", 
			QObject::connect(client, &BldsClient::getResponse,
			this, [this](const QString& param, bool, 
					const QVariant& value) -> void {
				if (param != "recording-position") {
					return;
				}
				recordingPositionLine->setText(QString::number(value.toFloat(), 'f', 1));
			})
	);

	/* In the case that the recording has been stopped, that handler
	 * then checks if the source still exists as well. This functor
	 * handles those responses.
	 */
	connections.insert("source-exists-connection", 
			QObject::connect(client, &BldsClient::getResponse,
			[this](const QString& param, bool, const QVariant& data) -> void {
				if (param != "source-exists") {
					return;
				}
				if (!data.toBool()) {
					handleSourceDeleted();
				}
			})
	);

	recordingStatusTimer->start();
}

void MeactlWidget::handleRecordingExistsReply(bool exists)
{
	if (exists) {
		client->get("recording-position");
	} else {
		handleRecordingStopped();
		client->get("source-exists");
	}
}

void MeactlWidget::cancelPendingServerConnection()
{

	/* Re-enable connecting to the server. */
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::cancelPendingServerConnection);
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::connectToServer);
	connectToServerButton->setText("Connect");
	connectToServerButton->setToolTip("Connect to BLDS");
	serverHostLine->setReadOnly(false);
	emit serverConnectionCanceled();

	/* Delete the client. */
	if (client) {
		QObject::disconnect(client, 0, 0, 0);
		client->deleteLater();
	}
}

void MeactlWidget::showSettingsWindow()
{
	if (!client)
		return;
	auto win = new SourceSettingsWindow(client->hostname(), this);
	QObject::connect(win, &SourceSettingsWindow::adcRangeChanged,
			this, &MeactlWidget::adcRangeChanged);
	QObject::connect(win, &SourceSettingsWindow::configurationChanged,
			this, &MeactlWidget::configurationChanged);
	QObject::connect(win, &SourceSettingsWindow::analogOutputChanged,
			this, &MeactlWidget::analogOutputChanged);
	QObject::connect(win, &SourceSettingsWindow::triggerChanged,
			this, &MeactlWidget::triggerChanged);
	win->show();
}

