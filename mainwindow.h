#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//*** Default QT Libraries ***//
//
#include <QMainWindow>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QWidget>
#include <QMessageBox>
#include <QTimer>
#include <QStandardItemModel>
#include <QTcpSocket>
#include <QFile>
#include <QTextStream>
#include <QtSql>
#include <QSqlQuery>
#include <QTextCursor>
#include <QKeyEvent>
#include <QAction>

//
//***------- user Libraries ----***//
//
#include "plottingwindow.h"
#include "qcustomplot.h"
//
//***-------------------------***//

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class hanleCommand;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /*
     * Startup Setup Functions
     */
    void setTCPConnection();
    void setup();

    /*
     * Custom Data Table Setup Functions
     */
    // void setupPlot(QStandardItemModel *targetModel);

    /*
     * External Acces Methods
     */
    void displayMessageBox(QString message, QString color);     // Displaying message at QT Message box
    void displayMessageConsole(QString message, QString color); // Displaing message at Console
    //
    /*
     * Custom Data Table Setup Functions
     */

    void setupDataFolder(); //-> creates folder for storing data
    // Data table
    void setupData_tableView();                //-> function to setup dataTable View
    void addData_tableView(QStringList datas); //-> function to add item to existing table View
    // Properties table
    void setupProperties_tableView();                              // -> setup for column headers in properties table view
    void addProperties_tableView(QString property, QString value); // -> adds properties to the tableView
    bool checkPropertyExists_tableView(QString item);              // -> returns if property exists in the table already

    //

    //
    /*
     * Pulic Variable Definition
     * Used to acces ui elements from antoher cpp file
     */
    Ui::MainWindow *ui;

private slots:


    
    void connectTCP(QString &host, QString &Port); // connects to the given port and host ip
    void disconnectTCP();                          // Disconnects from the host
    void onReadyRead();                            // Reading incoming bytes form TCP Port
    void sendCommand(QString command);             // Sends given string as command returns if succesfull
    /*
     */

    void on_ConsoleSend_pushButton_clicked();


    void on_TCP_Connect_pushButton_clicked();

    void on_TCP_EnableManualInput_checkBox_stateChanged(int arg1);

    void on_Data_tableView_doubleClicked(const QModelIndex &index);

    void on_DataView_Clear_pushButton_clicked();

    void on_DataView_Export_exportConsole_pushButton_clicked();

    void on_Properties_tableView_doubleClicked(const QModelIndex &index);

    void on_Console_Clear_pushButton_clicked();

    void on_Console_Export_exportConsole_pushButton_clicked();

    void on_ShowFolder_pushButton_clicked();

    void on_Properties_tableView_clicked(const QModelIndex &index);

    void on_PropertySet_pushButton_clicked();

private:
    //  *** Private object definitions  *** //
    QTimer *serialTimer;                                    //-> for timing applications
    QTcpSocket socket;                                      //-> for tcp connection
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); //-> for database access

    //*** Pointer Conteiner to the Widgets ***// -> used to store open widget
    QList<PlottingWindow *> temperaturePlots; // stores open temperature plots pointers ->will be used when updating plots

    //*** Private status Toggle ***//
    bool isConnected = 0;

    //*** Private TCP Variables ***//
    QString targetHostAdress;
    QString targetPortAdress;

    // -> Standard Model Items for the widgets in the ui
    // also acts as a data storage structure at ui
    QStandardItemModel *mainItemModel;                  // Model to store all commands
    QStandardItemModel *Data_tablewView_ItemModel;      // Model to store all incoming respond from server
    QStandardItemModel *Properties_tableView_ItemModel; // Model to store all properties received from server

    void setupDatabase();                                                                                     // creating database on local repo
    void addElementToDatabase(QString date, QString sequence, QString note, QString property, QString value); // adding element to the database

    //  ***  Key event  *** //
    void keyPressEvent(QKeyEvent *event) override; // function to handle keypresses
    QString prevCommands[20];                      // Array to store prev arrays
    int prevIndex = 0;                             // default index to store command cycle

    void insertElementToBuffer(QString item);      // insert element to the queue
    bool checkElementExistsOnBuffer(QString item); // check if command already exists in the buffer
    void checkCommandExists(QString item);         // check command already exists in the stack
};

#endif // MAINWINDOW_H
