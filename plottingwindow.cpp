#include "plottingwindow.h"
#include "ui_plottingwindow.h"

PlottingWindow::PlottingWindow(QWidget *parent) : QWidget(parent),
                                                  ui(new Ui::PlottingWindow)
{
  ui->setupUi(this);

  setupPlot();
  // setup(); // example plot
}

PlottingWindow::PlottingWindow(QStandardItemModel *target, QStandardItemModel *mainProperties, QString name, QWidget *parent) : QWidget(parent), ui(new Ui::PlottingWindow)
{
  // Setup Plotting settings
  ui->setupUi(this);

  setupSettings();

  //
  mainPropertiesModel = mainProperties;
  //
  targetModel = target;
  targets.append(name);
  //
  targetName = name;

  dataStruct *temp = new dataStruct(name);
  array.append(temp);

  setupProperties_ListView();
  setupPlot();

  //  *** Setup Interactions  *** //
  ui->widgetCustomPlot->legend->setVisible(true);

  QFont legendFont = font();
  legendFont.setPointSize(10);
  ui->widgetCustomPlot->legend->setSelectedFont(legendFont);
  ui->widgetCustomPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

  ui->widgetCustomPlot->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->widgetCustomPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));
}

PlottingWindow::~PlottingWindow()
{
  delete ui;
}

void PlottingWindow::setupSettings()
{
  // setting up shape styles
  shapes << QCPScatterStyle::ssCross;
  shapes << QCPScatterStyle::ssPlus;
  shapes << QCPScatterStyle::ssCircle;
  shapes << QCPScatterStyle::ssDisc;
  shapes << QCPScatterStyle::ssSquare;
  shapes << QCPScatterStyle::ssDiamond;
  shapes << QCPScatterStyle::ssStar;
  shapes << QCPScatterStyle::ssTriangle;
  shapes << QCPScatterStyle::ssTriangleInverted;
  shapes << QCPScatterStyle::ssCrossSquare;
  shapes << QCPScatterStyle::ssPlusSquare;
  shapes << QCPScatterStyle::ssCrossCircle;
  shapes << QCPScatterStyle::ssPlusCircle;
  shapes << QCPScatterStyle::ssPeace;
  shapes << QCPScatterStyle::ssCustom;

  // settinng
}

void PlottingWindow::setupProperties_ListView()
{
  // creating temp item for first property
  QStandardItem *tempProperty = new QStandardItem(targetName);
  // setup properties
  tempProperty->setCheckable(true);
  tempProperty->setCheckState(Qt::Checked);
  //  -> append first item to the tree
  propertiesModel = new QStandardItemModel(this);
  propertiesModel->appendRow(tempProperty);
  ui->properties_listView->setModel(propertiesModel);

  for (int i = 0; i < mainPropertiesModel->rowCount(); i++)
  {
    if (!checkPropertyExistOnListView(mainPropertiesModel->item(i, 0)->text()))
    {
      QStandardItem *tempProperty = new QStandardItem(mainPropertiesModel->item(i, 0)->text());
      // setup properties
      tempProperty->setCheckable(true);
      tempProperty->setCheckState(Qt::Unchecked);
      propertiesModel->appendRow(tempProperty);
    }
  }
}

bool PlottingWindow::checkPropertyExistOnListView(QString property)
{
  for (int i = 0; i < propertiesModel->rowCount(); i++)
  {
    if ((propertiesModel->item(i, 0)->text() == property) || (property == "<Property Name>"))
      return true;
  }
  return false;
}

void PlottingWindow::setupPlot()
{

  // clear data
  ui->widgetCustomPlot->clearGraphs();
  ui->widgetCustomPlot->replot();
  // seting up plot
  //-> x Axis setup
  ui->widgetCustomPlot->xAxis->label();
  ui->widgetCustomPlot->xAxis->setTickLabels(false);
  ui->widgetCustomPlot->xAxis->setLabel("Time");
  ui->widgetCustomPlot->xAxis2->setVisible(true);
  ui->widgetCustomPlot->xAxis2->setTickLabels(false);
  ui->widgetCustomPlot->yAxis2->setVisible(true);
  ui->widgetCustomPlot->yAxis2->setTickLabels(false);
  ui->widgetCustomPlot->yAxis->setRange(0, verticalMax);
  ui->widgetCustomPlot->yAxis->ticker()->setTickCount(10);

  // adding interaction
  ui->widgetCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);

  for (int i = 0; i < array.size(); i++) // for each element to be plotted
  {

    array[i]->x.clear();                              // reset data on x axis
    array[i]->y.clear();                              // reset data on y axis
    for (int j = 0; j < targetModel->rowCount(); j++) // for each element in database -> search element match in database
    {

      if ((targetModel->item(j, 4)->text().size() > 0) && (array[i]->name == targetModel->item(j, 4)->text()))
      {
        QStringList temp = targetModel->item(j, 5)->text().split("/");
        array[i]->y.append(temp[0].toDouble());
        array[i]->dates.append(targetModel->item(j, 0)->text() + targetModel->item(j, 1)->text());
      }
    }

    for (int n = 0; n < array[i]->y.size(); n++)
    {
      array[i]->x.append(n);
    }

    ui->widgetCustomPlot->addGraph()->setData(array[i]->x, array[i]->y);
    ui->widgetCustomPlot->graph(i)->setName(array[i]->name);
    ui->widgetCustomPlot->xAxis->setRange(0, array[i]->y.length() - 1);

    ui->widgetCustomPlot->replot();
  }
  // -> saving the previous index
  prevIndex = targetModel->rowCount();
  //
  // Style Options
  QColor color(20 + 200 / 4.0, 70 * (1.6 / 4.0), 150, 150);
  ui->widgetCustomPlot->graph()->setLineStyle(QCPGraph::lsLine);
  ui->widgetCustomPlot->graph()->setPen(QPen(color));
  ui->widgetCustomPlot->graph()->setScatterStyle(QCPScatterStyle(shapes[4], 5));
  // ui->widgetCustomPlot->graph()->setBrush(QBrush(color));

  ui->widgetCustomPlot->yAxis->setLabel("Temperature");
  ui->widgetCustomPlot->xAxis->setLabel("Time");
  ui->widgetCustomPlot->replot();
}

void PlottingWindow::updatePlot()
{
  // O(n^2) on large property set !
  for (int i = 0; i < array.size(); i++)
  {
    int prevSize = array[i]->y.size();
    for (int j = prevIndex; j < targetModel->rowCount(); j++)
    {

      if ((targetModel->item(j, 4)->text().size() > 0) && (targetModel->item(j, 4)->text() == array[i]->name))
      {
        QStringList temp = targetModel->item(j, 5)->text().split("/");
        array[i]->y.append(temp[0].toDouble());
        array[i]->dates.append(targetModel->item(j, 0)->text() + targetModel->item(j, 1)->text());
      }
    }

    for (int n = prevSize; n < array[i]->y.size(); n++)
    {
      array[i]->x.append(n);
    }

    ui->widgetCustomPlot->graph(i)->setData(array[i]->x, array[i]->y);
    ui->widgetCustomPlot->xAxis->setRange(0, array[i]->y.length() - 1);

    ui->widgetCustomPlot->replot();
  }
  prevIndex = targetModel->rowCount();
}

void PlottingWindow::on_pushButton_clicked()
{
  QString saveFilePath = QFileDialog::getSaveFileName(this, "Select file path so save graph.");
  ui->widgetCustomPlot->saveJpg(saveFilePath + ".jpg");
}

void PlottingWindow::on_addProperty_pushButton_clicked()
{
}

void PlottingWindow::contextMenuRequest(QPoint pos)
{
  QMenu *menu = new QMenu(this);
  menu->setAttribute(Qt::WA_DeleteOnClose);

  if (ui->widgetCustomPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
  {
    menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
    menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
    menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
    menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
    menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
  }
  else // general context menu on graphs requested
  {
    menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
    if (ui->widgetCustomPlot->selectedGraphs().size() > 0)
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->widgetCustomPlot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
  }

  menu->popup(ui->widgetCustomPlot->mapToGlobal(pos));
}

void PlottingWindow::moveLegend()
{
  if (QAction *contextAction = qobject_cast<QAction *>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
  {
    bool ok;
    int dataInt = contextAction->data().toInt(&ok);
    if (ok)
    {
      ui->widgetCustomPlot->axisRect()->insetLayout()->setInsetAlignment(0, (Qt::Alignment)dataInt);
      ui->widgetCustomPlot->replot();
    }
  }
}

void PlottingWindow::on_properties_listView_activated(const QModelIndex &index)
{
  qDebug() << " a " << endl;
}

void PlottingWindow::on_properties_listView_clicked(const QModelIndex &index)
{
  bool changed = false;
  for (int i = 0; i < propertiesModel->rowCount(); i++)
  {
    if (propertiesModel->item(i, 0)->checkState() == Qt::Checked && !checkPropertyExistOnArray(propertiesModel->item(i, 0)->text())) // adding graph
    {
      array.push_back(new dataStruct(propertiesModel->item(i, 0)->text()));
      changed = true;
    }
    else if (propertiesModel->item(i,0)->checkState() != Qt::Checked && checkPropertyExistOnArray(propertiesModel->item(i,0)->text()) )
    {
      changed = true;
      array.removeAt(i);
    }
  }
  if (changed)   // if new elementd added
    setupPlot(); // setup again
}

bool PlottingWindow::checkPropertyExistOnArray(QString property)
{
  for (int i = 0; i < array.size(); i++)
  {
    if (property == array[i]->name)
    {
      return true;
    }
  }
  return false;
}
