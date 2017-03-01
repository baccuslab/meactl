/*! \file source-settings-window.h
 *
 * Header declaring class for manipulating data source settings.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEACTL_SOURCE_SETTINGS_WINDOW_H
#define MEACTL_SOURCE_SETTINGS_WINDOW_H

#include <QtCore>
#include <QtWidgets>

#include "blds-client.h"

/*! \class SourceSettingsWindow
 *
 * The SourceSettingsWindow class is used to view and manipulate
 * the settings of a data source managed by the BLDS. Users are
 * given widgets for manipulating the ADC range, triggering 
 * mechanism, analog output, and HiDens chip configuration.
 */
class SourceSettingsWindow : public QWidget {
	Q_OBJECT
	public:

		/*! Construct a SourceSettingsWindow. */
		SourceSettingsWindow(const QString& host, QWidget* parent = nullptr);

		/*! Destruct a SourceSettingsWindow. */
		~SourceSettingsWindow();

		/* Copying ain't allowed. */
		SourceSettingsWindow(const SourceSettingsWindow&) = delete;
		SourceSettingsWindow(SourceSettingsWindow&&) = delete;
		SourceSettingsWindow& operator=(const SourceSettingsWindow&) = delete;

	signals:

		/*! Emitted when the user changes the ADC range.
		 *
		 * \param range The new ADC range.
		 */
		void adcRangeChanged(float range);

		/*! Emitted when the user selects a new configuration file.
		 *
		 * \param file The new file from which the configuration was read.
		 */
		void configurationChanged(const QString& file);

		/*! Emitted when the user selects a new analog output file.
		 *
		 * \param file The file from which the analog output was read.
		 */
		void analogOutputChanged(const QString& file);

		/*! Emitted when the user selects a new trigger.
		 *
		 * \param trigger The new triggering mechanism.
		 */
		void triggerChanged(const QString& trigger);

		void plugChanged(const QString& plug);

	private slots:

		/* Open a dialog box for selecting a configuration, and send 
		 * it to the BLDS.
		 */
		void chooseConfiguration();

		/* Handle the initial request for the status of the data
		 * source, collecting the current values for the parameters
		 * that can be manipulated via the provided widgets. Also
		 * connect slots for responding to user selections.
		 */
		void handleSourceStatus(bool exists, QJsonObject json);

		/* Slot called when the trigger changes, sending the new
		 * value to the BLDS.
		 */
		void onTriggerChanged(const QString& text);

		/* Slot called when the ADC range changes, sending the new
		 * value to the BLDS.
		 */
		void onAdcRangeChanged(double value);

		/* Slot called when the analog output changes, sending the
		 * new value to the BLDS.
		 */
		void onAnalogOutputChanged(const QString& file, const QVector<double>& aout);

		/* Slot called to actually choose a file and read data from it. */
		void chooseAnalogOutput();

		/* Slot called which simply clears analog output. */
		void clearAnalogOutput();

		void onPlugChanged(const QString& plug);

	private:

		/* Method which actually reads data from the given file. */
		QVector<double> readAnalogOutputFromFile(const QString& file);

		/* Initialize the widget layout. */
		void setupLayout();

		QJsonObject status;

		/*! Main layout manager */
		QGridLayout* layout;

		/*! Labels the ADC range spin box. */
		QLabel* adcRangeLabel;

		/*! Shows the actual ADC range. */
		QDoubleSpinBox* adcRangeBox;

		/*! Labels the selected trigger. */
		QLabel* triggerLabel;

		/*! List of allowed triggers, and shows the current one. */
		QComboBox* triggerBox;

		QLabel* plugLabel;
		QComboBox* plugBox;

		/*! Labels the configuration line. */
		QLabel* configurationLabel;

		/*! Shows the actual configuration. */
		QLineEdit* configurationLine;


		/*! Button to bring up dialog for selecting new file. */
		QPushButton* chooseConfigurationButton;

		QLabel* analogOutputLabel;
		QLineEdit* analogOutputLine;
		QPushButton* selectAnalogOutputButton;
		QPushButton* clearAnalogOutputButton;

		/*! The window uses its own client to get and set the values
		 * of the parameters corresponding to the provided widgets.
		 */
		QPointer<BldsClient> client;
};

#endif

