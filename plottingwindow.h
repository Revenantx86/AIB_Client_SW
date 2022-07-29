#ifndef TEMPERATUREPLOT_H
#define TEMPERATUREPLOT_H

#include <QWidget>
#include <QStandardItemModel>
#include <QDebug>
#include <QString>
#include <QColor>
#include "qcustomplot.h"
#include <QInputDialog>

// ->  data structure for the plot
struct dataStruct
{

public:
    dataStruct() {}
    dataStruct(QString nm) : name(nm) {}

    QString name;
    QVector<QString> dates;
    QVector<double> x;
    QVector<double> y;
};
//
//
namespace Ui
{
    class PlottingWindow;
}

class PlottingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlottingWindow(QWidget *parent = nullptr);
    PlottingWindow(QStandardItemModel *target, QStandardItemModel *mainProperties, QString name, QWidget *parent = nullptr);
    ~PlottingWindow();

    void setup();
    void setupPlot();
    void setupSettings();

    // ***  Properties list View Functions  *** //
    void setupProperties_ListView();
    bool checkPropertyExistOnListView(QString property); // check property already exists on the list view
    // Continious data adding
    void updatePlot();

private slots:
    void on_pushButton_clicked();

    void on_addProperty_pushButton_clicked();

    void on_properties_listView_clicked(const QModelIndex &index);

    //  *** On Graph Control Functions  ***   //
    void contextMenuRequest(QPoint pos); // menu
    void moveLegend();                   // moving legend

    void on_properties_listView_activated(const QModelIndex &index);

private:
    Ui::PlottingWindow *ui;

    QStandardItemModel *propertiesModel; // properties model for the list view for all available properties
    QStandardItemModel *targetModel;     //  main item model
    QStandardItemModel *mainPropertiesModel;

    QVector<dataStruct *> array;

    QStringList targets;
    QString targetName;
    int verticalMax = 300;
    int verticalMin = 100;

    // Storage for plotting settings
    QVector<QCPScatterStyle::ScatterShape> shapes;
    QVector<QCPGraph::LineStyle> lines;
};

#endif // TEMPERATUREPLOT_H
