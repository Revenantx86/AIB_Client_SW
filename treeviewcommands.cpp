#include <QStandardItem>
#include <QStandardItemModel>
#include <QFile>
#include <QTextStream>
#include <QDialog>

#include <QMessageBox>

#include <mainwindow.h>
#include <QDateTime>

//-----------       ----------------             Command Tree Functions                ----------------          ----------------//
/*
 * Method for reading commands from text file, returns the ready tree view model
 */
QStandardItemModel *readCommandsFromFile(QString &fileName)
{
    QStandardItemModel *tempModel = new QStandardItemModel;
    QStandardItem *tempMainItem = nullptr;
    QStandardItem *tempSubItem = nullptr;
    QStandardItem *tempSubColumn = nullptr;
    QFile commandText(fileName);

    tempModel->setColumnCount(2);

    int count = 0;

    QTextStream stream(&commandText);
    if (commandText.open(QIODevice::ReadWrite))
    {
        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            QStringList listedLine = line.split("-");
            QList<QStandardItem *> tempItem;
            // commandDataStruct* tempData = new commandDataStruct(listedLine[0],listedLine[1]);
            if (line[0] == '*') // if main function
            {
                tempMainItem = new QStandardItem(listedLine[0].mid(1));
            }
            else if (line[0] == '#')
            {
                tempItem.append(new QStandardItem(listedLine[0].mid(1)));
                tempItem.append(new QStandardItem(listedLine[1]));
                // tempSubItem = new QStandardItem(listedLine[0]);
                tempMainItem->appendRow(tempItem);
            }
            else
            {
                tempModel->appendRow(tempMainItem);
                count++;
            }
        }
    }
    commandText.close();
    return tempModel;
}
