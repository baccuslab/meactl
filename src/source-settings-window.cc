/*! \file source-settings-window.cc
 *
 * Implementation of SourceSettingsWindow class.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "H5Cpp.h"

#include "source-settings-window.h"

SourceSettingsWindow::SourceSettingsWindow(const QString& hostname,
				QWidget* parent) :
	QWidget(parent, Qt::Window)
{
	/* Create new client, request status after successfully connecting. */
	client = new BldsClient(hostname);
	QObject::connect(client, &BldsClient::sourceStatus,
			this, &SourceSettingsWindow::handleSourceStatus);
	QObject::connect(client, &BldsClient::connected,
			[&](bool made) -> void { 
				if (made) 
					client->requestSourceStatus();
				else { 
					QMessageBox::warning(this, "Could not connect",
							"The BLDS could not be reached. Verify "
							"that it is running and the IP address is correct.");
					close();
				}
			});
	client->connect();

	/* Setup UI. */
	setupLayout();
	setWindowTitle("Source settings");
	setAttribute(Qt::WA_DeleteOnClose);

	/* Move just below parent widget. */
	auto upperLeft = parentWidget()->pos();
	auto rect = parentWidget()->frameGeometry();
	move(upperLeft.x(), upperLeft.y() + rect.height());
}

SourceSettingsWindow::~SourceSettingsWindow()
{
	client->disconnect();
	client->deleteLater();
}

void SourceSettingsWindow::setupLayout()
{
	layout = new QGridLayout(this);

	adcRangeLabel = new QLabel("ADC range:", this);
	adcRangeLabel->setAlignment(Qt::AlignRight);
	adcRangeBox = new QDoubleSpinBox(this);
	adcRangeBox->setToolTip("Voltage range of ADC");
	adcRangeBox->setRange(0.001, 10.0);
	adcRangeBox->setValue(0);
	adcRangeBox->setSuffix(" V");

	triggerLabel = new QLabel("Trigger:", this);
	triggerLabel->setAlignment(Qt::AlignRight);
	triggerBox = new QComboBox(this);
	triggerBox->addItems( {"none", "photodiode"} );
	triggerBox->setToolTip("Start trigger for recordings");

	plugLabel = new QLabel("Plug:", this);
	plugLabel->setAlignment(Qt::AlignRight);
	plugBox = new QComboBox(this);
	for (auto i = 0; i < 5; i++) {
		plugBox->addItem(QString::number(i));
	}
	plugBox->setToolTip("Choose Neurolizer plug number");

	configurationLabel = new QLabel("Configuration:", this);
	configurationLabel->setAlignment(Qt::AlignRight);
	configurationLine = new QLineEdit("", this);
	configurationLine->setToolTip("Configuration of Hidens chip");

	chooseConfigurationButton = new QPushButton("Select", this);
	chooseConfigurationButton->setToolTip("Select configuration from file");

	analogOutputLabel = new QLabel("Analog output:", this);
	analogOutputLabel->setAlignment(Qt::AlignRight);

	analogOutputLine = new QLineEdit("", this);
	analogOutputLine->setReadOnly(true);

	selectAnalogOutputButton = new QPushButton("Select", this);
	selectAnalogOutputButton->setToolTip("Select analog output from file");

	clearAnalogOutputButton = new QPushButton("Clear", this);
	clearAnalogOutputButton->setToolTip("Clear analog output");

	layout->addWidget(adcRangeLabel, 0, 0);
	layout->addWidget(adcRangeBox, 0, 1);
	layout->addWidget(triggerLabel, 0, 2);
	layout->addWidget(triggerBox, 0, 3);
	layout->addWidget(plugLabel, 0, 4);
	layout->addWidget(plugBox, 0, 5);
	layout->addWidget(configurationLabel, 1, 0);
	layout->addWidget(configurationLine, 1, 1, 1, 4);
	layout->addWidget(chooseConfigurationButton, 1, 5);
	layout->addWidget(analogOutputLabel, 2, 0);
	layout->addWidget(analogOutputLine, 2, 1, 1, 3);
	layout->addWidget(selectAnalogOutputButton, 2, 4);
	layout->addWidget(clearAnalogOutputButton, 2, 5);
}

void SourceSettingsWindow::chooseConfiguration()
{
	auto fname = QFileDialog::getOpenFileName(this,
			"Choose config file", QDir::homePath(),
			"Config files (*.cmdraw.nrk2 *.cmdraw)");
	if (fname.isNull() || fname.size() == 0)
		return;

	/* Connect functor to handle the response to the request. */
	QObject::connect(client, &BldsClient::setSourceResponse,
			this, [this,fname](const QString& param, bool success, 
					const QString& msg) -> void {
				if (!param.startsWith("configuration"))
					return;
				QObject::disconnect(client, &BldsClient::setSourceResponse, 0, 0);
				if (success) {
					configurationLine->setText(fname);
					emit configurationChanged(configurationLine->text());
				} else {
					QMessageBox::warning(this, "Could not set configuration",
							QString("The configuration could not be set: %1").arg(msg));
				}
			});

	/* Actually make the request */
	client->setSource("configuration-file", fname);
}

void SourceSettingsWindow::handleSourceStatus(bool exists, QJsonObject obj)
{
	if (!exists) {
		QMessageBox::critical(this, "No source!",
				"There doesn't appear to be a data source!");
		close();
	}

	/* Get current values of the parameters from the server. */
	status = obj;
	if (status.contains("adc-range")) {
		adcRangeBox->setValue(status["adc-range"].toDouble());
	}
	if (status.contains("trigger")) {
		triggerBox->setCurrentText(status["trigger"].toString());
	}
	if (status.contains("has-analog-output")) {
		if (status["has-analog-output"].toBool()) {
			analogOutputLine->setText("Unknown analog output file");
			analogOutputLine->setEnabled(false);
		}
	}

	/* Connect slots from UI widgets, now that we're connected. */
	QObject::connect(triggerBox, &QComboBox::currentTextChanged,
			this, &SourceSettingsWindow::onTriggerChanged);
	QObject::connect(chooseConfigurationButton, &QPushButton::clicked,
			this, &SourceSettingsWindow::chooseConfiguration);
	QObject::connect(adcRangeBox, static_cast<void(QDoubleSpinBox::*)(double)>(
				&QDoubleSpinBox::valueChanged),
			this, &SourceSettingsWindow::onAdcRangeChanged);
	QObject::connect(selectAnalogOutputButton, &QPushButton::clicked,
			this, &SourceSettingsWindow::chooseAnalogOutput);
	QObject::connect(clearAnalogOutputButton, &QPushButton::clicked,
			this, &SourceSettingsWindow::clearAnalogOutput);
	QObject::connect(plugBox, &QComboBox::currentTextChanged,
			this, &SourceSettingsWindow::onPlugChanged);
}

void SourceSettingsWindow::chooseAnalogOutput()
{
	auto fname = QFileDialog::getOpenFileName(this,
			"Choose analog output file", QDir::homePath(), 
			"HDF5 files (*.h5 *.hdf5)");
	if (fname.isNull() || fname.size() == 0)
		return;

	QVector<double> vec;
	try {
		vec = readAnalogOutputFromFile(fname);
	} catch (std::invalid_argument& err) {
		QMessageBox::critical(this, "Error reading analog output", err.what());
		return;
	}
	onAnalogOutputChanged(fname, vec);
}

QVector<double> SourceSettingsWindow::readAnalogOutputFromFile(const QString& fname)
{
	auto name = fname.toStdString();
	if (!H5::H5File::isHdf5(name)) {
		throw std::invalid_argument("The selected file is not in valid HDF5 format.");
	}

	H5::H5File f(fname.toStdString().c_str(), H5F_ACC_RDONLY);
	H5::DataSet dset;
	try {
		dset = f.openDataSet("analog-output");
	} catch (...) {
		f.close();
		throw std::invalid_argument("The file doesn't have a dataset called 'analog-output'");
	}

	auto space = dset.getSpace();
	if (space.getSimpleExtentNdims() != 1) {
		f.close();
		throw std::invalid_argument("The selected file has a dataset with "
				"more than 1 dimension. Analog output signals must be specified with "
				"only a single dimension, and with double-precision data");
	}
	hsize_t dims[1] = { 0 };
	space.getSimpleExtentDims(dims);
	QVector<double> vec(dims[0]);
	dset.read(vec.data(), H5::PredType::IEEE_F64LE);
	return vec;
}

void SourceSettingsWindow::clearAnalogOutput()
{
	onAnalogOutputChanged("", {});
}

void SourceSettingsWindow::onTriggerChanged(const QString& text)
{
	/* Connect functor to handle a response to the request to 
	 * set the trigger.
	 */
	QObject::connect(client, &BldsClient::setSourceResponse,
			this, [this](const QString& param, bool valid, const QString& msg) -> void {
				if (param != "trigger")
					return;
				QObject::disconnect(client, &BldsClient::setSourceResponse, 0, 0);
				if (valid) {
					emit triggerChanged(triggerBox->currentText());
				} else {
					QMessageBox::warning(this, "Could not set trigger",
							QString("The trigger could not be set: %1").arg(msg));
					return;
				}
			});

	/* Actually make the request. */
	client->setSource("trigger", text);
}

void SourceSettingsWindow::onAdcRangeChanged(double range)
{
	/* Connect functor to handle a response to the request to
	 * set the ADC range.
	 */
	QObject::connect(client, &BldsClient::setSourceResponse,
			this, [this](const QString& param, bool valid, const QString& msg) -> void {
				if (param != "adc-range")
					return;
				QObject::disconnect(client, &BldsClient::setSourceResponse, 0, 0);
				if (valid) {
					emit adcRangeChanged(adcRangeBox->value());
				} else {
					QMessageBox::warning(this, "Could not set ADC range",
							QString("The ADC range could not be set: %1").arg(msg));
					return;
				}
			});

	/* Actually make the request. */
	client->setSource("adc-range", range);
}

void SourceSettingsWindow::onAnalogOutputChanged(const QString& file, 
		const QVector<double>& aout)
{
	/* Connect functor to handle a response to the request to
	 * set the analog output.
	 */
	QObject::connect(client, &BldsClient::setSourceResponse,
			this, [this,file,aout](const QString& param, bool valid, 
					const QString& msg) -> void {
				if (param != "analog-output")
					return;
				QObject::disconnect(client, &BldsClient::setSourceResponse, 0, 0);
				if (valid) {
					analogOutputLine->setText(file);
					analogOutputLine->setEnabled(true);
					emit analogOutputChanged(file);
				} else {
					QMessageBox::warning(this, "Could not set analog output",
							QString("The analog output could not be set: %1").arg(msg));
					return;
				}
			});

	/* Actually make the request. */
	client->setSource("analog-output", QVariant::fromValue(aout));
}

void SourceSettingsWindow::onPlugChanged(const QString& plug)
{
	/* Connect functor handling response to request to set
	 * the plug number.
	 */
	QObject::connect(client, &BldsClient::setSourceResponse,
			this, [this,plug](const QString& param, bool valid, 
					const QString& msg) -> void {
				if (param != "plug")
					return;
				QObject::disconnect(client, &BldsClient::setSourceResponse, 0, 0);
				if (valid) {
					emit plugChanged(plug);
				} else {
					QMessageBox::warning(this, "Could not select plug",
							QString("The Neurolizer plug could not "
							"be selected: %1").arg(msg));
				}
			});

	/* Actually make the request. */
	client->setSource("plug", static_cast<quint32>(plug.toInt()));
}

