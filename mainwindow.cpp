#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "treeviewcommands.cpp"
#include <QHostAddress>

#include <QtWidgets/QFileDialog>

//  -----------      ----------------                Ui Initalization Functions                     ----------------              ---------------- //
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height()); // disable maximizing
    //
    serialTimer = new QTimer(this);
    mainItemModel = new QStandardItemModel(this);
    //
    setup();
}

MainWindow::~MainWindow()
{
    // Checking if its still connected
    if (isConnected)
    {
        disconnectTCP();
    }

    delete ui;
}

/**
 * @brief main setup function caller.
 *
 *  This function calls the all setup functions for the elements in the ui.
 *  also sets up command cycling buffer with predefined commands
 * @code {.c++}
 * MainWindow::setup()
 * @endcode
 *
 */
void MainWindow::setup()
{
    // Check&create data folder
    setupDataFolder();

    // Console Text edit Setup
    ui->Console_textEdit->setTextInteractionFlags(Qt::NoTextInteraction);

    //----- Ui Function Initalization  ------//
    //
    setTCPConnection();
    setupData_tableView();
    setupProperties_tableView();
    setupDatabase();

    // Insert Predefined commands to the command buffer
    insertElementToBuffer("sub de1.temp");
    insertElementToBuffer("sub coil0.vol");
    insertElementToBuffer("sub coil0.cur");
    insertElementToBuffer("get de1.temp");
    insertElementToBuffer("get test");
}


/**
 * @brief Setting up TCP connection GroupBox
 *
 * Sets up the TCP selection group box elements. Inserts the predefined IP and Port
 * Adresses. Starts with ComboBox selection mode as default.
 * @code {.c++}
 * MainWindow::setTCPConnection()
 * @endcode
 *
 */
void MainWindow::setTCPConnection()
{
    // Adding predefined IP & Port adresses to the ComboBoxes.
    ui->TCP_Select_IP_comboBox->addItem("10.58.0.81");
    ui->TCP_Select_Port_comboBox->addItem("1234");
    //
    ui->TCP_Select_IP_comboBox->addItem("127.0.0.1");
    ui->TCP_Select_Port_comboBox->addItem("8081");

    // Disabling the manual entry lineEdits
    ui->TCP_ManualInput_IP_lineEdit->setEnabled(0);
    ui->TCP_ManualInput_IP_label->setEnabled(0);
    ui->TCP_ManualInput_Port_lineEdit->setEnabled(0);
    ui->TCP_ManualInput_Port_label->setEnabled(0);
}

/**
 * @brief Setup function for table view
 *
 * Setup for tablew view. Sets up header name, stretch format and other initial properties.
 *
 * @code {.c++}
 * MainWindow::setupData_tableView()
 * @endcode
 *
 */
void MainWindow::setupData_tableView()
{
    //
    QList<QStandardItem *> headerItem; // list of standard Item  -> append each column -> align at center
    headerItem.append(new QStandardItem("<Timestamp>"));
    headerItem[0]->setTextAlignment(Qt::AlignCenter);
    headerItem.append(new QStandardItem("<Timestamp>"));
    headerItem[1]->setTextAlignment(Qt::AlignCenter);
    headerItem.append(new QStandardItem("<Sequence Number>"));
    headerItem[2]->setTextAlignment(Qt::AlignCenter);
    headerItem.append(new QStandardItem("note"));
    headerItem[3]->setTextAlignment(Qt::AlignCenter);
    headerItem.append(new QStandardItem("<Property>"));
    headerItem[4]->setTextAlignment(Qt::AlignCenter);
    headerItem.append(new QStandardItem("<Message>"));
    headerItem[5]->setTextAlignment(Qt::AlignCenter);

    // Append header
    Data_tablewView_ItemModel = new QStandardItemModel(this);
    Data_tablewView_ItemModel->appendRow(headerItem);
    //
    ui->Data_tableView->setModel(Data_tablewView_ItemModel);                            // append final model to data table
    ui->Data_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // adjust column width
    //
}

/**
 * @brief Setup properties table view
 *
 * setup functin for table view. Creates header and sets default parameters for view.
 *
 * @code {.c++}
 * MainWindow::setupProperties_tableView()
 * @endcode
 *
 */
void MainWindow::setupProperties_tableView()
{
    QList<QStandardItem *> tempHeader; // Create list of temp headers
    tempHeader.append(new QStandardItem("<Property Name>"));
    tempHeader.append(new QStandardItem("<Value>"));

    tempHeader[0]->setTextAlignment(Qt::AlignCenter);

    Properties_tableView_ItemModel = new QStandardItemModel(this);
    Properties_tableView_ItemModel->appendRow(tempHeader);

    ui->Properties_tableView->setModel(Properties_tableView_ItemModel);
    ui->Properties_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

/**
 *  @brief Setup function for database
 *
 *  Function for setting up database functionality.
 *  Creates new database folder according to the startup time of the program.
 *  Creates database items based on string.
 * @code {.c++}
 * MainWindow::setupDatabase()
 * @endcode
 *
 */
void MainWindow::setupDatabase()
{

    // create default path for fb
    QString path = QDir::currentPath() + "/data/" + QDateTime::currentDateTime().toString("MM-dd-HH:mm:ss");
    path = path + ".db";
    //qDebug() << path << endl;
    db.setDatabaseName(path);

    // open database
    if (!db.open())
    {
        displayMessageBox("An Error occured while setting up database ! ", "black");
        //qDebug() << "An Error occured while creating database ! " << endl;
    }
    // create script for table struct
    QString setupScript = "CREATE TABLE database ("
                          "Timestamp VARCHAR(20),"
                          "SequenceNumber VARCHAR(20),"
                          "Note VARCHAR(20),"
                          "Property VARCHAR(20),"
                          "Value VARCHAR(20) );";
    QSqlQuery query;
    // create table
    if (!query.exec(setupScript))
    {
        displayMessageBox("An Error occured while setting up database ! ", "black");
        //qDebug() << "An Error occured while creating database ! " << endl;
    }
}


//  -----------      ----------------                  TCP Functions                     ----------------              ----------------  //

/**
 * @brief Connect to TCP target
 *
 * Function for connecting tcp target. host and port names are get from corresponding
 * combo box sections.
 *
 * @code {.c++}
 * MainWindow::setupProperties_tableView()
 * @endcode
 *
 */
void MainWindow::connectTCP(QString &host, QString &port)
{
    if (!isConnected) // if not already connected
    {
        socket.connectToHost(QHostAddress(host), port.toInt()); // connect to specified host
        socket.waitForConnected(100);                           // wait for connection
        if (socket.state() == QTcpSocket::ConnectedState)       // if succesfull
        {
            isConnected = 1;
            ui->TCP_EnableManualInput_checkBox->setEnabled(0);
            ui->TCP_Connect_pushButton->setText("Disconnect");
            connect(&socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
        }
        else // not succesfull -> close port
        {
            displayMessageBox("Could not find the host !", "black");
            socket.disconnect();
            socket.close();
        }
    }
}


/**
 * @brief Disconnect from TCP target
 *
 * Function for disconnecting from tcp target.
 *
 * @code {.c++}
 * MainWindow::disconnectTCP()
 * @endcode
 *
 */
void MainWindow::disconnectTCP()
{
    if (isConnected)
    {
        socket.disconnect();
        if (socket.state() != QTcpSocket::ConnectedState)
            ; // checks if properly disconnected
        {
            isConnected = 0;
            ui->TCP_EnableManualInput_checkBox->setEnabled(1);
            ui->TCP_Connect_pushButton->setText("Connect");
            socket.close();
        }
    }
}

/**
 * @brief Function for reading data from TCP
 *
 * Function emitted when ui recieves bytes over TCP connection.
 * Reads the bytes and converts into data package.
 *
 * @code {.c++}
 * MainWindow::onReadyRead()
 * @endcode
 *
 */
void MainWindow::onReadyRead() // triggers when byte received
{
    /*
     * Qt incoming data structre for reqular client - server comm
     *      QBytreArray = <Time Stamp> + <Time Stamp> + <Sequence Number> + <Keyword>
     *
     * Qt incoming data structure for change in property
     *      QBtreArray = <Time Stamp> + <Time Stamp> + <sequence number> + note +  <property> + <value>
     */

    QByteArray rawData = socket.readLine();             // reads the all available btyes in the socket
    QStringList dataList = QString(rawData).split(" "); // splits it
    //

    displayMessageConsole(rawData, "blue"); // Displaying the raw message on console
    //

    if (dataList.size() == 6) // Check if data package fits for package with property value
    {
        addData_tableView(dataList);                                                                               // add message to the data table view
        addProperties_tableView(dataList[4], dataList[5]);                                                         // pass property and its value
        addElementToDatabase(dataList[0] + "-" + dataList[1], dataList[2], dataList[3], dataList[4], dataList[5]); // add message to the database

        for (int i = 0; i < temperaturePlots.size(); i++) // for each open plotting window, update the plot values
        {
            temperaturePlots[i]->updatePlot();
        }
    }
    else if (dataList.size() == 4) // if package is just ack response
    {
    }
}


/**
 * @brief Send commands over TCP.
 *
 * Sends commands over TCP connection. uses QTCP functions to send data
 * over tcp connection.
 *
 * @code {.c++}
 * MainWindow::onReadyRead()
 * @endcode
 *
 */
void MainWindow::sendCommand(QString command)
{
    // writing on the TCP Server
    if (socket.state() == QTcpSocket::ConnectedState)
    {
        insertElementToBuffer(command); // Adding command to the memmory buffer
        prevIndex = 0;                  // reset command cycling index

        // sending command to tcp socket
        command += " \n";
        socket.write(command.toUtf8());
        socket.flush();
        displayMessageConsole("Sending ->", "darkMagenta");
        displayMessageConsole(command, "black"); // displaying on console text box
    }
    else
    {
        displayMessageBox("Not connected to any Host !", "Black");
    }
}




//  -----------      ----------------                  Internal Functions                     ----------------              ----------------  //

/**
 * @brief Function for adding item to database
 *
 *  This function called whenever new message received to system. Saves every message received on database.
 * @code {.c++}
 * MainWindow::addElementToDatabase(QString date, QString sequence, QString note, QString property, QString value) // adding element to the database
 * @endcode
 *
 */
void MainWindow::addElementToDatabase(QString date, QString sequence, QString note, QString property, QString value) // adding element to the database
{

    QSqlQuery query;

    query.prepare("INSERT INTO database ("
                  "Timestamp, "
                  "SequenceNumber, "
                  "Note, "
                  "Property, "
                  "Value) "
                  "VALUES (?,?,?,?,?);");

    query.addBindValue(date);
    query.addBindValue(sequence);
    query.addBindValue(note);
    query.addBindValue(property);
    query.addBindValue(value);

    if (!query.exec())
    {
        displayMessageBox("An Error occured while adding value to database ! ", "black");
    }
}

/**
 * @brief Adding data to TableView
 *
 * Function for adding data to table view. Gets input message as string list
 * and adds corresponding field to the table view as a new row.
 *
 * @code {.c++}
 * MainWindow::addData_tableView(QStringList datas)
 * @endcode
 *
 */
void MainWindow::addData_tableView(QStringList datas)
{
    QList<QStandardItem *> element;         // create new row
    for (int i = 0; i < datas.size(); i++)  // for each elements in datas
    {
        element.append(new QStandardItem(datas[i])); // add a new column to row
    }
    Data_tablewView_ItemModel->appendRow(element); // append the new row
}

/**
 * @brief adding properties to the table view.
 *
 * Function for adding properties to the properties table view.
 * gets the property name and value to add table view.
 *
 * If property already exists, it updates the last value to the current new value.
 *
 * @code {.c++}
 * MainWindow::addProperties_tableView(QString property, QString value)
 * @endcode
 *
 */
void MainWindow::addProperties_tableView(QString property, QString value)
{
    if (!checkPropertyExists_tableView(property)) // if property does not exists in the table
    {
        QList<QStandardItem *> element;
        element.append(new QStandardItem(property));
        element.append(new QStandardItem(value));
        Properties_tableView_ItemModel->appendRow(element);
    }
    else // if property already exists
    {
        for (int i = 0; i < Properties_tableView_ItemModel->rowCount(); i++)
        {
            if (Properties_tableView_ItemModel->item(i, 0)->text() == property)
            {
                Properties_tableView_ItemModel->item(i, 1)->setText(value);
            }
        }
    }
}

/**
 * @brief Function for checking if proprety exists on table view
 *
 * Internal iterative method for checking if propoerty exists
 * at properties table view.
 *
 * return true if exists.
 *
 * @code {.c++}
 * MainWindow::checkPropertyExists_tableView(QString item)
 * @endcode
 *
 */
bool MainWindow::checkPropertyExists_tableView(QString item)
{
    for (int i = 1; i < Properties_tableView_ItemModel->rowCount(); i++) //each element in properties table view
    {
        if (Properties_tableView_ItemModel->item(i, 0)->text() == item)
        {
            return true;
        }
    }
    return false;
}

/**
 * @brief Function for hadnling key press events.
 *
 * This function emitted whenever keypress happens.
 * Checks if the pressed button functionality exits.
 *
 * @code {.c++}
 * MainWindow::keyPressEvent(QKeyEvent *event)
 * @endcode
 *
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) //pressing key up
    {
        if (prevCommands[prevIndex].length() > 1)
        {
            ui->Console_lineEdit->setText(prevCommands[prevIndex]);
            if (prevCommands[prevIndex + 1].length() > 1)
            {
                prevIndex++;
                prevIndex = prevIndex % 20;
            }
        }
    }
    else if (event->key() == Qt::Key_Down) //pressing key down
    {
        if ((prevIndex) > 0)
        {
            prevIndex--;
            ui->Console_lineEdit->setText(prevCommands[prevIndex]);
        }
        else
        {
            ui->Console_lineEdit->setText(prevCommands[prevIndex]);
        }
    }

    else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) //pressing enter or return key
    {
        if (ui->Console_lineEdit->text().size() > 0)
        {
            on_ConsoleSend_pushButton_clicked();
        }
    }
}

/**
 * @brief Checking if command exists on command buffer
 *
 *  Internal method of checking if elements exists in
 *  cycling command buffer.
 *
 * @code {.c++}
 * MainWindow::checkElementExistsOnBuffer(QString item)
 * @endcode
 *
 */
bool MainWindow::checkElementExistsOnBuffer(QString item)
{
    for (int i = 0; i < 20; i++) // iterate over commands
    {
        if (prevCommands[i] == item)
            return true;
    }
    return false;
}

/**
 * @brief Inserting item to cycling command buffer
 *
 *  Internal method of inserting command to the
 *  cyclinc command buffer.
 *
 * @code {.c++}
 * MainWindow::insertElementToBuffer(QString item)
 * @endcode
 *
 */
void MainWindow::insertElementToBuffer(QString item)
{
    if (!checkElementExistsOnBuffer(item))
    {
        for (int i = 8; i >= 0; i--)
        {
            prevCommands[i + 1] = prevCommands[i];
        }
        prevCommands[0] = item;
    }
}


/**
 * @brief Message box function
 *
 *  Function to display given string at external message box.
 *
 * @code {.c++}
 * MainWindow::displayMessageBox(QString message, QString color)
 * @endcode
 *
 */
void MainWindow::displayMessageBox(QString message, QString color)
{
    QMessageBox::about(this, "Warning !", message);
}

/**
 * @brief Console Display function
 *
 *  Function to ddisplay given message on console
 *
 * @code {.c++}
 * MainWindow::displayMessageConsole(QString message, QString color)
 * @endcode
 *
 */
void MainWindow::displayMessageConsole(QString message, QString color)
{
    ui->Console_textEdit->setTextColor(QColor(color));
    ui->Console_textEdit->insertPlainText(message);
    ui->Console_textEdit->setTextColor(QColor("white"));
    ui->Console_textEdit->verticalScrollBar()->setValue(ui->Console_textEdit->verticalScrollBar()->maximum()); // scrolling to bottom automatically
    // ui->Console_textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse); // -> enables mouse intereaction
}

/**
 * @brief Setup data folder.
 *
 *  Function for setup data folder. checks if data fodler exists and creates new one if dont.
 * @code {.c++}
 * MainWindow::setupDataFolder()
 * @endcode
 *
 */
void MainWindow::setupDataFolder()
{
    if (!QDir("data").exists())
    {
        QDir().mkdir("data");
    }
}

//  -----------      ----------------                Ui Button Functions                     ----------------              ---------------- //


/**
 * @brief Enabling manual TCP input
 *
 * Checkbox function to enable manual input for TCP connection parameters.
 * When checked, it enables manual line edit section and disables the combo box section.
 *
 * @code {.c++}
 * MainWindow::on_TCP_EnableManualInput_checkBox_stateChanged(int arg1)
 * @endcode
 *
 */
void MainWindow::on_TCP_EnableManualInput_checkBox_stateChanged(int arg1)
{
    if (!arg1)
    {
        // Disable manual input boxes
        ui->TCP_ManualInput_IP_lineEdit->setEnabled(0);
        ui->TCP_ManualInput_IP_label->setEnabled(0);
        ui->TCP_ManualInput_Port_lineEdit->setEnabled(0);
        ui->TCP_ManualInput_Port_label->setEnabled(0);
        // Enable combobox selextion
        ui->TCP_Select_IP_comboBox->setEnabled(1);
        ui->TCP_Select_IP_label->setEnabled(1);
        ui->TCP_Select_Port_comboBox->setEnabled(1);
        ui->TCP_Select_Port_label->setEnabled(1);
    }
    else // if Manual input is enabled
    {
        // enable manual input boxes
        ui->TCP_ManualInput_IP_lineEdit->setEnabled(1);
        ui->TCP_ManualInput_IP_label->setEnabled(1);
        ui->TCP_ManualInput_Port_lineEdit->setEnabled(1);
        ui->TCP_ManualInput_Port_label->setEnabled(1);
        // disable combobox selextion
        ui->TCP_Select_IP_comboBox->setEnabled(0);
        ui->TCP_Select_IP_label->setEnabled(0);
        ui->TCP_Select_Port_comboBox->setEnabled(0);
        ui->TCP_Select_Port_label->setEnabled(0);
    }
}

/**
 * @brief Connect button
 *
 *  TCP connect button. Checks current state and gathers requierd port and
 *  host adrres from correspoding boxes. Then calls the internal method for connecting
 *
 * @code {.c++}
 *  MainWindow::on_TCP_Connect_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_TCP_Connect_pushButton_clicked()
{
    if (!isConnected) // if Not connected
    {
        if (ui->TCP_EnableManualInput_checkBox->isChecked())
        {
            targetHostAdress = ui->TCP_ManualInput_IP_lineEdit->text();
            targetPortAdress = ui->TCP_ManualInput_Port_lineEdit->text();
        }
        else
        {
            targetHostAdress = ui->TCP_Select_IP_comboBox->currentText();
            targetPortAdress = ui->TCP_Select_Port_comboBox->currentText();
        }
        connectTCP(targetHostAdress, targetPortAdress);
    }
    else // if Connected
    {
        disconnectTCP();
    }
}

/**
 * @brief Console command send button
 *
 * Button function for sending commands written on console text edit.
 * It calls internal method for sending data over TCP.
 *
 * @code {.c++}
 * MainWindow::on_ConsoleSend_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_ConsoleSend_pushButton_clicked()
{
    if (ui->Console_lineEdit->text().size() > 0) // if there is actually input
    {
        sendCommand(ui->Console_lineEdit->text());
        ui->Console_lineEdit->clear(); // clear
    }
}

/**
 * @brief Clear Console
 *
 * Function for cleaning all lines at console text edit.
 *
 * @code {.c++}
 * MainWindow::on_Console_Clear_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_Console_Clear_pushButton_clicked()
{
    ui->Console_textEdit->clear();
}


/**
 * @brief Console Export button
 *
 *  Function for exporting all text written on console text edit.
 *  Exports as .txt file format.
 *
 * @code {.c++}
 * MainWindow::on_Console_Export_exportConsole_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_Console_Export_exportConsole_pushButton_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Select Text File "), QDir::rootPath(), "Text File (*.txt)");
    if (path.length() > 0) // if text selected
    {
        QFile file(path);
        if (file.open(QIODevice::ReadWrite))
        {
            QTextStream text(&file);
            text << ui->Console_textEdit->toPlainText();
            text.flush();
        }
        file.close();
    }
}

/**
 * @brief Opening plotting window by double clicking.
 *
 *  This function opens up plotting screen if user clicked on property name.
 * @code {.c++}
 * MainWindow::on_Data_tableView_doubleClicked(const QModelIndex &index)
 * @endcode
 *
 */
void MainWindow::on_Data_tableView_doubleClicked(const QModelIndex &index)
{
    if (index.column() == 4) // Check if double clicked on property name only
    {
        QString targetName = Data_tablewView_ItemModel->itemFromIndex(index)->text();                                                   // Get the name of the double clicked property
        PlottingWindow *newWidget = new PlottingWindow(Data_tablewView_ItemModel, Properties_tableView_ItemModel, targetName, nullptr); // create new plotting window using clicked property
        newWidget->show();

        temperaturePlots.append(newWidget);
    }
    else // display error message
    {
        displayMessageBox("Please select Property name only.", "black");
    }
}

/**
 * @brief Console Export button
 *
 *  Function for exporting all text written on console text edit.
 *  Exports as .txt file format.
 *
 * @code {.c++}
 * MainWindow::on_Console_Export_exportConsole_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_Properties_tableView_doubleClicked(const QModelIndex &index)
{
    QString targetName = Properties_tableView_ItemModel->itemFromIndex(index)->text(); // getting the name for table

    PlottingWindow *newWidget = new PlottingWindow(Data_tablewView_ItemModel, Properties_tableView_ItemModel, targetName, nullptr);

    newWidget->show();

    temperaturePlots.append(newWidget);
}

/**
 * @brief Clear Table View Function
 *
 *  Function for cleaning all data on table view. It does not affect database.
 *
 * @code {.c++}
 * MainWindow::on_DataView_Clear_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_DataView_Clear_pushButton_clicked()
{
    Data_tablewView_ItemModel->clear();
}

/**
 * @brief Create csv file at selected location by user.
 *
 *  This function creates csv file from data table.
 * @code {.c++}
 * MainWindow::on_DataView_Export_exportConsole_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_DataView_Export_exportConsole_pushButton_clicked()
{

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Text File"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    QFile file(filename + ".csv");
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);

        stream << "<TimeStamp>"
               << ","
               << "<Sequence Number>"
               << ","
               << "<Keyword>"
               << ","
               << "<Property>"
               << ","
               << "<Value>"
               << "\n";
        for (int i = 1; i < Data_tablewView_ItemModel->rowCount(); i++)
        {
            stream << Data_tablewView_ItemModel->item(i, 0)->text() + " " + Data_tablewView_ItemModel->item(i, 1)->text() << "," << Data_tablewView_ItemModel->item(i, 2)->text() << "," << Data_tablewView_ItemModel->item(i, 3)->text() << "," << Data_tablewView_ItemModel->item(i, 4)->text() << "," << Data_tablewView_ItemModel->item(i, 5)->text();
        }
        stream.flush();
    }
    file.close();
}

/**
 * @brief Reveal folder location button.
 *
 * Function for revealing folder location.
 * @code {.c++}
 * MainWindow::on_ShowFolder_pushButton_clicked()
 * @endcode
 *
 */
void MainWindow::on_ShowFolder_pushButton_clicked()
{
    QString path = QDir::currentPath() + "/data";
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}


/**
 * @brief Function for properties click
 *
 * function for handling input when user clicked the properties table.
 * @code {.c++}
 * MainWindow::on_Properties_tableView_clicked(const QModelIndex &index)
 * @endcode
 *
 */
void MainWindow::on_Properties_tableView_clicked(const QModelIndex &index)
{
    if (index.column() == 0 && index.row() > 0) // -> if selected item is in range of property column and its property name
    {
        ui->PropertyName_lineEdit->setText(Properties_tableView_ItemModel->itemFromIndex(index)->text()); // ->set property line edit selected property name
    }
}


/**
 * @brief Button for property set
 *
 * Called when user wants to set value for clicked property value.
 * Get the value text information and sends over send command function.
 *
 * @code {.c++}
 * MainWindow::on_Properties_tableView_clicked(const QModelIndex &index)
 * @endcode
 *
 */
void MainWindow::on_PropertySet_pushButton_clicked()
{
    if (ui->PropertyValue_lineEdit->text().length() > 0)
    {
        sendCommand("set " + ui->PropertyName_lineEdit->text() + " " + ui->PropertyValue_lineEdit->text() + " \n");
    }
}

