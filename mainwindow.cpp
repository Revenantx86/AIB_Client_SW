#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "treeviewcommands.cpp"
#include <QHostAddress>

#include <QtWidgets/QFileDialog>

#include "handleCommand.cpp"

//  -----------      ----------------                Ui Initalization Functions                     ----------------              ---------------- //
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
//
/*
 * Setup layout
 */
// setup layout
void MainWindow::setup()
{
    // Console Text edit Setup
    ui->Console_textEdit->setTextInteractionFlags(Qt::NoTextInteraction);

    //----- Ui Function Initalization  ------//
    //
    setCommandTree();      // -> set up command tree with builtin text file
    setTCPConnection();    // -> set up TCP Default connection parameters
    setupCustomPlot(ui);   // -> setup customPlot
    setupData_tableView(); // - > setup data table view paramters
    setupProperties_tableView();

    // console command cycle setup demo
    insertElementToBuffer("sub de1.temp");
    insertElementToBuffer("sub coil0.vol");
    insertElementToBuffer("sub coil0.cur");
    insertElementToBuffer("get de1.temp");
    insertElementToBuffer("get de2.temp");

    // demo graphs
}

/*
 * Layout Initalization functions
 */
//
// ***  Setting the TCP connection Boxes default parameters *** //
void MainWindow::setTCPConnection()
{
    // adding default adresses to the combo boxs
    ui->TCP_Select_IP_comboBox->addItem("10.58.0.81");
    ui->TCP_Select_Port_comboBox->addItem("1234");
    ui->TCP_Select_Port_comboBox->addItem("1233");

    //
    ui->TCP_Select_IP_comboBox->addItem("127.0.0.1");
    ui->TCP_Select_Port_comboBox->addItem("8081");

    // disabling the manual entry
    ui->TCP_ManualInput_IP_lineEdit->setEnabled(0);
    ui->TCP_ManualInput_IP_label->setEnabled(0);
    ui->TCP_ManualInput_Port_lineEdit->setEnabled(0);
    ui->TCP_ManualInput_Port_label->setEnabled(0);
}
//
// *** set up command tree with initial parameters  *** //
void MainWindow::setCommandTree()
{
    // container to hold sub items
    QList<QStandardItem *> items;
    // Setup Header
    QStandardItem *header0 = new QStandardItem("Command");
    QStandardItem *header1 = new QStandardItem("Effect");
    // create main model
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(2);
    // append header
    model->setHorizontalHeaderItem(0, header0);
    model->setHorizontalHeaderItem(1, header1);
    // sample Properties at start
    items.append(new QStandardItem("get"));
    items.append(new QStandardItem("Get the current value of Property"));
    //
    QStandardItem *item1 = new QStandardItem("Commands");
    item1->appendRow(items);
    //
    model->appendRow(item1);
    // Forward final model to the tree view
    ui->Commands_treeView->setModel(model);
}
//
// *** Setup for data Table View  *** //
void MainWindow::setupData_tableView()
{
    //
    QList<QStandardItem *> headerItem; // list of standard Item  -> append each column
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

    //
    Data_tablewView_ItemModel = new QStandardItemModel(this);
    Data_tablewView_ItemModel->appendRow(headerItem);
    //
    ui->Data_tableView->setModel(Data_tablewView_ItemModel);                            // append final model to data table
    ui->Data_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // adjust column width
    //
}
//
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
//  -----------      ----------------                  Custom Functions                     ----------------              ---------------- //
/*
 * Internal Methods
 */
//
//*** Internal Method connecting from TCP server ***//
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
//
// *** Internal Method disconnecting from TCP server *** //
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
//
// *** Method for reading incoming bytes from TCP Socket */
void MainWindow::onReadyRead() // triggers when byte received
{
    /*
     * Qt incoming data structre for reqular client - server comm
     *      QBytreArray = <Time Stamp> + <Time Stamp> + <Sequence Number> + <Keyword>
     *
     * Qt incoming data structure for change in property
     *      QBtreArray = <Time Stamp> + <Time Stamp> + <sequence number> + note +  <property> + <value>
     */
    QByteArray rawData = socket.readAll();
    QStringList datas = QString(rawData).split(" ");
    //
    // -> Displaying the raw input on debug console text edit
    //
    displayMessageConsole(rawData, "blue");
    //
    //
    if (datas.size() > 4 && rawData != "Succesfully Connected to Host \r\n") // Check if
    {
        handleCommand(rawData, ui);
        addData_tableView(datas);
        addProperties_tableView(datas[4], datas[5]); // pass property and its value
        //
        for (int i = 0; i < temperaturePlots.size(); i++) // for each open widget update existing plots
        {
            temperaturePlots[i]->updatePlot();
        }
    }

    /*
    if (datas.size() <= 3 || rawData == "Succesfully Connected to Host \r\n") // if answer is regular
    {
    }
    else // else if answer for changed event
    {

    }
    */
}
//
// *** Method for sending bytes to TCP Socket ***//
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
//
// *** Adding element to the data table view ***//
void MainWindow::addData_tableView(QStringList datas)
{
    QList<QStandardItem *> element;
    for (int i = 0; i < datas.size(); i++)
    {
        element.append(new QStandardItem(datas[i]));
    }
    Data_tablewView_ItemModel->appendRow(element);
}
//
//  *** Adding element to the properties table view ***// -> Case Sensitive !
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
//
// *** Checking if element exists in the properties table view  *** //
bool MainWindow::checkPropertyExists_tableView(QString item)
{
    for (int i = 1; i < Properties_tableView_ItemModel->rowCount(); i++)
    {
        if (Properties_tableView_ItemModel->item(i, 0)->text() == item)
        {
            return true;
        }
    }
    return false;
}
//
//  *** previous command cycle with arrow keys  *** //
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up)
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
    else if (event->key() == Qt::Key_Down)
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

    else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (ui->Console_lineEdit->text().size() > 0)
        {
            on_ConsoleSend_pushButton_clicked();
        }
    }
}
//
//  *** check if command already exists in the buffer   ***//
bool MainWindow::checkElementExistsOnBuffer(QString item)
{
    for (int i = 0; i < 20; i++) // iterate over commands
    {
        if (prevCommands[i] == item)
            return true;
    }
    return false;
}
//  *** inserts element to the previous comamnd array   *** //
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
//
/*
 * External Methods
 */
//  *** Displays message on external message box    *** //
void MainWindow::displayMessageBox(QString message, QString color)
{
    QMessageBox::about(this, "Warning !", message);
}
//
// *** Display given message on console with custom color   *** //
void MainWindow::displayMessageConsole(QString message, QString color)
{
    ui->Console_textEdit->setTextColor(QColor(color));
    ui->Console_textEdit->insertPlainText(message);
    ui->Console_textEdit->setTextColor(QColor("white"));
    ui->Console_textEdit->verticalScrollBar()->setValue(ui->Console_textEdit->verticalScrollBar()->maximum()); // scrolling to bottom automatically
    // ui->Console_textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse); // -> enables mouse intereaction
}
//
//  -----------      ----------------                Ui Button Functions                     ----------------              ---------------- //
//
//  *** Routine for enabling and disabling between manual TCP input and combo box TCP input ***     //
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
//
//****  TCP Connect Buton ***//
// Gets current host and IP adress and passes to the internal methods
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

//****************** DEBUG CONSOLE BUTTONS ***************//
//
// *** Getting input from command box and writing to tcp and displaying on console   ***   //
void MainWindow::on_ConsoleSend_pushButton_clicked()
{
    if (ui->Console_lineEdit->text().size() > 0) // if there is actually input
    {
        sendCommand(ui->Console_lineEdit->text());
        ui->Console_lineEdit->clear(); // clear
    }
}
//
//  *** Clearing the console text ***   //
void MainWindow::on_Console_Clear_pushButton_clicked()
{
    ui->Console_textEdit->clear();
}
//
//  *** Exporting Console as text file  *** //
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

void MainWindow::on_Commands_treeView_doubleClicked(const QModelIndex &index)
{
    ui->Console_lineEdit->setText(mainItemModel->itemFromIndex(index)->text());
    // QStandardItemModel * temp = mainItemModel->itemFromIndex(index)->takeColumn(1);
    // ui->Console_textEdit->insertPlainText(mainItemModel->itemFromIndex(index)->text());
}

/*
 * Importing the commands text file, selecting text file at qDialog,
 * it will automatically import to the command tree
 */
void MainWindow::on_Command_Import_pushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Text File "), QDir::rootPath(), "Text File (*.txt)");
    if (fileName.length() > 0) // if text selected
    {
        ui->Console_textEdit->setTextColor(QColor("magenta"));
        ui->Console_textEdit->insertPlainText("Opening text file at -> \" ");
        ui->Console_textEdit->setTextColor(QColor("white"));
        ui->Console_textEdit->insertPlainText(fileName);
        ui->Console_textEdit->insertPlainText("\" \n");
        mainItemModel = readCommandsFromFile(fileName);
        ui->Commands_treeView->setModel(mainItemModel);
    }
}

void MainWindow::on_Data_tableView_doubleClicked(const QModelIndex &index)
{
    QString targetName = Data_tablewView_ItemModel->itemFromIndex(index)->text();
    PlottingWindow *newWidget = new PlottingWindow(Data_tablewView_ItemModel, Properties_tableView_ItemModel, targetName, nullptr);
    newWidget->show();

    temperaturePlots.append(newWidget);
}

void MainWindow::on_Properties_tableView_doubleClicked(const QModelIndex &index)
{
    QString targetName = Properties_tableView_ItemModel->itemFromIndex(index)->text();
    PlottingWindow *newWidget = new PlottingWindow(Data_tablewView_ItemModel, Properties_tableView_ItemModel, targetName, nullptr);
    newWidget->show();

    temperaturePlots.append(newWidget);
}

void MainWindow::on_DataView_Clear_pushButton_clicked()
{
    Data_tablewView_ItemModel->clear();
}

void MainWindow::on_DataView_Export_exportConsole_pushButton_clicked()
{

    QString filename = QFileDialog::getSaveFileName(this, tr("Save Text File"), QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), tr("Text Files (*.txt)"));
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite))
    {
        QTextStream stream(&file);

        for (int i = 0; i < Data_tablewView_ItemModel->rowCount(); i++)
        {
            stream << Data_tablewView_ItemModel->item(i, 0)->text() << Data_tablewView_ItemModel->item(i, 1)->text() << Data_tablewView_ItemModel->item(i, 2)->text() << Data_tablewView_ItemModel->item(i, 3)->text() << Data_tablewView_ItemModel->item(i, 4)->text() << endl;
        }
        stream.flush();
    }
    file.close();
}
