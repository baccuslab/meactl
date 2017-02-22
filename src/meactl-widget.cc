/*! \file meactl-widget.cc
	} else if (param == 
 *
 * Implementation of main controlling widget.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "meactl-widget.h"

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
	sourceLayout->addWidget(sourceLocationLine, 1, 1, 1, 4);

	/* Widgets related to the actual recording. */
	recordingGroup = new QGroupBox("Recording", this);
	recordingLayout = new QGridLayout(recordingGroup);
	recordingLengthLabel = new QLabel("Length:", recordingGroup);
	recordingLengthLabel->setAlignment(Qt::AlignRight);
	recordingLengthLine = new QLineEdit("1000", recordingGroup);
	recordingLengthLine->setAlignment(Qt::AlignRight);
	auto validator = new QIntValidator(1, 50000, recordingLengthLine);
	recordingLengthLine->setValidator(validator);
	recordingPositionLabel = new QLabel("Position:", recordingGroup);
	recordingPositionLabel->setAlignment(Qt::AlignRight);
	recordingPositionLine = new QLineEdit("0", recordingGroup);
	recordingPositionLine->setReadOnly(true);
	recordingFileLabel = new QLabel("Filename:", recordingGroup);
	recordingFileLabel->setAlignment(Qt::AlignRight);
	recordingFile = new QLineEdit("", recordingGroup);
	recordingFile->setToolTip("Filename at which to save data");
	recordingPathButton = new QPushButton("Path", recordingGroup);
	startRecordingButton = new QPushButton("Start", recordingGroup);
	startRecordingButton->setToolTip("Start the recording");
	startRecordingButton->setEnabled(false);
	recordingLayout->addWidget(recordingPositionLabel, 0, 0);
	recordingLayout->addWidget(recordingPositionLine, 0, 1);
	recordingLayout->addWidget(recordingLengthLabel, 0, 2);
	recordingLayout->addWidget(recordingLengthLine, 0, 3);
	recordingLayout->addWidget(recordingFileLabel, 1, 0);
	recordingLayout->addWidget(recordingFile, 1, 1);
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
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::disconnectFromServer);
	QObject::connect(client, &BldsClient::disconnected,
			this, [this]() -> void {
				createSourceButton->setEnabled(false);
				showSettingsButton->setEnabled(false);
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

	/* Setup choosing the recording path. */
	QObject::connect(recordingPathButton, &QPushButton::clicked,
			this, &MeactlWidget::chooseRecordingDirectory);

	/* Connect the handler for the initial status reply from the
	 * BLDS, and request that status.
	 */
	QObject::connect(client, &BldsClient::serverStatus,
			this, &MeactlWidget::handleInitialStatusReply);
	client->requestServerStatus();
}

void MeactlWidget::onServerError(const QString& err)
{
	client->deleteLater();
	emit serverError(err);
}

void MeactlWidget::disconnectFromServer()
{

	/* Disconnect pretty much all signals/slots. */
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::disconnectFromServer);
	QObject::disconnect(recordingPathButton, &QPushButton::clicked,
			this, &MeactlWidget::chooseRecordingDirectory);
	QObject::disconnect(createSourceButton, &QPushButton::clicked, 0, 0);
	QObject::disconnect(recordingLengthLine, &QLineEdit::editingFinished, 0, 0);

	/* Re-connect slot to connect to the server. */
	QObject::connect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::connectToServer);

	/* Reset most of the UI. */
	createSourceButton->setText("Create");
	createSourceButton->setToolTip("Create data source with the given type");
	connectToServerButton->setText("Connect");
	sourceLocationLine->setReadOnly(false);
	recordingPositionLine->setText("0");
	sourceTypeBox->setEnabled(true);
	
	/* Disconnect and delete the client. */
	client->disconnect();
	client->deleteLater();

	/* Notify. */
	emit disconnectedFromServer();
}

void MeactlWidget::handleServerDisconnection()
{
}

void MeactlWidget::createDataSource()
{
	/* Disconnect this slot, and request creation of the source. */
	QObject::disconnect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::createDataSource);
	client->createSource(sourceTypeBox->currentText(), sourceLocationLine->text());
}

void MeactlWidget::onSourceCreated(bool success, const QString& msg)
{
	if (success) {
		handleSourceCreated();
	} else {
		/* Reconnect this slot. */
		QObject::connect(createSourceButton, &QPushButton::clicked,
				this, &MeactlWidget::createDataSource);
	}
	/* Notify. */
	emit sourceCreated(success, msg);
}

void MeactlWidget::handleSourceCreated()
{
	/* Connect slot for deleting the data source, and change the
	 * "create" button to a "delete" button.
	 */
	QObject::connect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::deleteDataSource);
	createSourceButton->setText("Delete");
	createSourceButton->setToolTip("Delete current data source");

	/* Enable starting the recording. */
	startRecordingButton->setEnabled(true);
	QObject::connect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);

	sourceLocationLine->setReadOnly(true);
	sourceTypeBox->setEnabled(false);
}

void MeactlWidget::deleteDataSource()
{
	/* Disconnect this slot and delete the source. */
	QObject::disconnect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::deleteDataSource);
	client->deleteSource();
}

void MeactlWidget::onSourceDeleted(bool success, const QString& msg)
{
	if (success) {
		handleSourceDeleted();
	} else {
		/* Re-connect this slot. */
		QObject::connect(createSourceButton, &QPushButton::clicked,
				this, &MeactlWidget::deleteDataSource);
	}
	/* Notify */
	emit sourceDeleted(success, msg);
}

void MeactlWidget::handleSourceDeleted()
{
	/* Disconnect this slot, and change the "delete" button 
	 * back to a "create" button.
	 */
	QObject::connect(createSourceButton, &QPushButton::clicked,
			this, &MeactlWidget::createDataSource);
	createSourceButton->setText("Create");
	createSourceButton->setToolTip("Create a data source of the selected type");

	/* Disable starting the recording. */
	startRecordingButton->setEnabled(false);
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);
	sourceLocationLine->setReadOnly(false);
	sourceTypeBox->setEnabled(true);
	showSettingsButton->setEnabled(false);

}

void MeactlWidget::startRecording()
{
	/* Disconnect this slot and start the recording. */
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::startRecording);
	client->startRecording();
}

void MeactlWidget::onRecordingStarted(bool success, const QString& msg)
{
	if (success) {
		handleRecordingStarted();
	} else {
		/* Reconnect this handler. */
		QObject::connect(startRecordingButton, &QPushButton::clicked,
				this, &MeactlWidget::startRecording);
	}
	/* Notify */
	emit recordingStarted(success, msg);
}

void MeactlWidget::handleRecordingStarted()
{
	/* Disable starting the recording, and enable stopping it. */
	QObject::disconnect(recordingLengthLine, &QLineEdit::editingFinished, 0, 0);
	QObject::connect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::stopRecording);
	startRecordingButton->setText("Stop");
	startRecordingButton->setToolTip("Stop current recording");

	/* Disable creating a data source. */
	createSourceButton->setEnabled(false);

	setupRecordingStatusHeartbeat();
}

void MeactlWidget::stopRecording()
{
	/* Disconnect this handler and stop the recording */
	QObject::disconnect(startRecordingButton, &QPushButton::clicked,
			this, &MeactlWidget::stopRecording);
	client->stopRecording();
}

void MeactlWidget::onRecordingStopped(bool success, const QString& msg)
{
	if (success) {
		handleRecordingStopped();
	} else {
		/* Reconnect this handler */
		QObject::connect(startRecordingButton, &QPushButton::clicked,
				this, &MeactlWidget::stopRecording);
	}

	/* Notify */
	emit recordingStopped(success, msg);
}

void MeactlWidget::handleRecordingStopped()
{
	/* Reconnect setting the length of the recording. */
	QObject::connect(recordingLengthLine, &QLineEdit::editingFinished,
			this, [this]() -> void {
				client->set("recording-length", 
						recordingLengthLine->text().toInt());
			});
	recordingPositionLine->setText("0");

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

void MeactlWidget::chooseAnalogOutput()
{
}

void MeactlWidget::handleInitialStatusReply(const QJsonObject& json)
{
	/* Determine basic information about the server and source. */
	auto sourceExists = json["source-exists"].toBool();
	auto recordingExists = json["recording-exists"].toBool();
	auto length = json["recording-length"].toInt();
	auto position = json["recording-position"].toDouble();

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
			recordingFile->setText(json["save-file"].toString());
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
			QObject::connect(recordingLengthLine, &QLineEdit::editingFinished,
					this, [this]() -> void {
						setRecordingLength(recordingLengthLine->text().toInt());
					});

			/* Enable starting the recording. */
			startRecordingButton->setEnabled(true);
			QObject::connect(startRecordingButton, &QPushButton::clicked,
					this, &MeactlWidget::startRecording);
			
			createSourceButton->setEnabled(true);
		}
	} else {
		/* No source, enable creating one. */
		createSourceButton->setEnabled(true);
		QObject::connect(createSourceButton, &QPushButton::clicked,
				this, &MeactlWidget::createDataSource);

		/* Set recording length and enable changing it. */
		recordingLengthLine->setText(QString("%1").arg(length));
		QObject::connect(recordingLengthLine, &QLineEdit::editingFinished,
				this, [this]() -> void {
					setRecordingLength(recordingLengthLine->text().toInt());
				});
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
		QObject::connect(client, &BldsClient::setResponse,
				this, [this](const QString& param, bool success, 
						const QString& msg) -> void {
					if (param == "save-directory") {
						
						/* Disconnect this slot. */
						QObject::disconnect(client, &BldsClient::setResponse, 0, 0);

						/* Notify, just a status bar message if successful, else
						 * a full warning dialog.
						 */
						if (success) {
							emit recordingDirectoryChanged(msg);
						} else {
							QMessageBox::warning(parentWidget(), "Could not set save path",
									QString("Could not set the save directory. %1").arg(msg));
						}
					}
				});

		/* Actually request to set the directory.*/
		client->set("save-directory", dir);
	}
}

void MeactlWidget::setupRecordingStatusHeartbeat()
{
	/* Request the server's status when the timer fires. */
	QObject::connect(recordingStatusTimer, &QTimer::timeout, 
			[this]() -> void {
				if (client) {
					client->requestServerStatus();
				}
			});

	/* Connect the slot for handling those replies. */
	QObject::connect(client, &BldsClient::serverStatus, 
			this, &MeactlWidget::handleRecordingStatusReply);

	recordingStatusTimer->start();
}

void MeactlWidget::handleRecordingStatusReply(const QJsonObject& json)
{
	if (!json["recording-exists"].toBool()) {
		handleRecordingStopped();
	}

	if (!json["source-exists"].toBool()) {
		handleSourceDeleted();
	}

	/* Update position in recording */
	recordingPositionLine->setText(
			QString::number(json["recording-position"].toDouble(), 'f', 1));

}

void MeactlWidget::cancelPendingServerConnection()
{
	QObject::disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MeactlWidget::cancelPendingServerConnection);
	connectToServerButton->setText("Connect");
	connectToServerButton->setToolTip("Connect to BLDS");
	if (client) {
		QObject::disconnect(client, 0, 0, 0);
		client->deleteLater();
	}
}

