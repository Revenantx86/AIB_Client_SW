#ifndef TEMPERATUREPLOT_H
#define TEMPERATUREPLOT_H

#include <QWidget>
#include <QStandardItemModel>
#include <QDebug>
#include <QString>
#include <QColor>
#include "qcustomplot.h"
#include <QInputDialog>
#include <QAction>

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
    QVector<QCPGraphData> timeData;
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
    void setupProperties_ListView();                     // checks the target model and adds property to the list view
    void updateProperties_ListView();                    // updated the available properties and adds to the list view
    bool checkPropertyExistOnListView(QString property); // check property already exists on the list view

    // *** Properties array *** //
    // -> array used for LUT table for plotting function
    // -> if item exists in the array plotter will plot it
    bool checkPropertyExistOnArray(QString property); // Checks if property exists in the aray
    int indexOfPropertyOnArray(QString property);     //-> returns the index of element in the array
    // Continious data adding
    void updatePlot();

    void setupContexMenu(QMenu *menu);
private slots:
    void on_pushButton_clicked();

    void on_properties_listView_clicked(const QModelIndex &index);

    //  *** On Graph Control Functions  ***   //
    void contextMenuRequest(QPoint pos); // menu
    void moveLegend();                   // moving legend

    void changeColor();
    void changeScatterStyle();

    void mouseMove(QMouseEvent *event);

    void on_FitScreen_pushButton_clicked();

    void on_refreshProperties_pushButton_clicked();

private:
    Ui::PlottingWindow *ui;

    QStandardItemModel *propertiesListModel; // properties model for the list view for all available properties
    QStandardItemModel *targetModel;         //  main item model -> received at setup
    QStandardItemModel *targetModelProperties;

    QVector<dataStruct *> array;
    int prevIndex = 0; // to store where program left when iterating over the main target model

    QString targetName;
    int verticalMax = 300;
    int verticalMin = 100;

    // Storage for plotting settings
    QVector<QCPScatterStyle::ScatterShape> shapes;
    QVector<QCPGraph::LineStyle> lines;

    QVariant red;
    double startTime;
};

#endif // TEMPERATUREPLOT_H
