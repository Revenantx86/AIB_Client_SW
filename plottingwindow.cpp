#include "plottingwindow.h"
#include "ui_plottingwindow.h"

#define DateFormat "yyyy-MMM-dd-hh:mm:ss" ///< Time Date format to sort timestamp

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

  // time plotting
  startTime = QDateTime::currentDateTime().toTime_t();
  //
  targetModelProperties = mainProperties;
  //
  targetModel = target;
  targets.append(name);
  //
  targetName = name;

  dataStruct *temp = new dataStruct(name);
  array.append(temp);

  setupProperties_ListView();
  setupPlot();

  // setting up the range

  // QString rere = QDateTime::fromTime_t(array[0]->timeData[0].key).toString();
  // QString dede = QDateTime::fromTime_t(array[0]->timeData[array[0]->timeData.size() - 1].key).toString();

  //  *** Setup Interactions  *** //
  ui->widgetCustomPlot->legend->setVisible(true);

  QFont legendFont = font();
  legendFont.setPointSize(10);
  ui->widgetCustomPlot->legend->setSelectedFont(legendFont);
  ui->widgetCustomPlot->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

  ui->widgetCustomPlot->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->widgetCustomPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));

  QObject::connect(ui->widgetCustomPlot, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(mouseMove(QMouseEvent *)));
}

void PlottingWindow::mouseMove(QMouseEvent *event)
{
  int x = ui->widgetCustomPlot->xAxis->pixelToCoord(event->pos().x());
  int y = ui->widgetCustomPlot->yAxis->pixelToCoord(event->pos().y());

  // array[0]->dates[x]+

  QToolTip::showText(event->globalPos(), QDateTime::fromTime_t(x).toString() + " \n Value: " + QString::number(y));

  // setToolTip(QString("%1 , %2").arg(x).arg(y));
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

void PlottingWindow::setupPlot()
{
  // clear data
  ui->widgetCustomPlot->clearGraphs();
  ui->widgetCustomPlot->replot();
  if (array.size() > 0)
  {

    // min and max value encountered
    int minValue = 0;
    int maxvalue = 0;
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

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat(DateFormat);
    ui->widgetCustomPlot->xAxis->setTicker(dateTicker);

    // adding interaction
    ui->widgetCustomPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables);

    for (int i = 0; i < array.size(); i++) // for each element to be plotted
    {

      array[i]->x.clear(); // reset data on x axis
      array[i]->y.clear(); // reset data on y axis

      ui->widgetCustomPlot->addGraph();

      for (int j = 1; j < targetModel->rowCount() - 1; j++) // for each element in database -> search element match in database
      {

        if ((targetModel->item(j, 4)->text().size() > 0) && (array[i]->name == targetModel->item(j, 4)->text()))
        {
          // QCPGraphData * tempData = new QCPGraphData();
          QString dateTemp = targetModel->item(j, 0)->text() + "-" + targetModel->item(j, 1)->text();
          QDateTime Date = QDateTime::fromString(dateTemp, DateFormat);
          double dateDouble = Date.toTime_t();
          // QDateTime Date = QDate::fromString("20/12/2015","dd/MM/yyyy");
          // tempData->key = targetModel->item(j, 0)->text() + targetModel->item(j, 1)->text();
          QStringList temp = targetModel->item(j, 5)->text().split("/");
          QCPGraphData tempTimeData;
          tempTimeData.key = dateDouble;
          tempTimeData.value = temp[0].toDouble();
          array[i]->timeData.push_back(tempTimeData);
          // array[i]->y.append(temp[0].toDouble()); // Append Y value
          // array[i]->x.append(dateDouble);
          // array[i]->dates.append(targetModel->item(j, 0)->text() + targetModel->item(j, 1)->text());
        }
      }
      ui->widgetCustomPlot->graph(i)->data()->set(array[i]->timeData);

      ui->widgetCustomPlot->graph(i)->setName(array[i]->name);
      ui->widgetCustomPlot->graph()->setScatterStyle(QCPScatterStyle(shapes[i], 5));

      ui->widgetCustomPlot->replot();
    }

    // -> saving the previous index
    prevIndex = targetModel->rowCount();
    //
    // Style Options
    QColor color(20 + 200 / 4.0, 70 * (1.6 / 4.0), 150, 150);
    ui->widgetCustomPlot->graph()->setLineStyle(QCPGraph::lsLine);
    ui->widgetCustomPlot->graph()->setPen(QPen(color));
    // ui->widgetCustomPlot->graph()->setBrush(QBrush(color));

    ui->widgetCustomPlot->yAxis->setLabel("Value");
    ui->widgetCustomPlot->xAxis->setLabel("Time");

    ui->widgetCustomPlot->replot();

    on_FitScreen_pushButton_clicked();
  }
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
        // QCPGraphData * tempData = new QCPGraphData();
        QString dateTemp = targetModel->item(j, 0)->text() + "-" + targetModel->item(j, 1)->text();
        QDateTime Date = QDateTime::fromString(dateTemp, DateFormat);
        double dateDouble = Date.toTime_t();
        // QDateTime Date = QDate::fromString("20/12/2015","dd/MM/yyyy");
        // tempData->key = targetModel->item(j, 0)->text() + targetModel->item(j, 1)->text();
        QStringList temp = targetModel->item(j, 5)->text().split("/");
        QCPGraphData tempTimeData;
        tempTimeData.key = dateDouble;
        tempTimeData.value = temp[0].toDouble();
        array[i]->timeData.push_back(tempTimeData);
      }
    }

    ui->widgetCustomPlot->graph(i)->data()->set(array[i]->timeData);

    // ui->widgetCustomPlot->xAxis->setRange(array[i]->timeData[0].key, QDateTime::currentDateTime().toTime_t());

    ui->widgetCustomPlot->replot();
  }
  prevIndex = targetModel->rowCount();
}

void PlottingWindow::on_pushButton_clicked()
{
  QString saveFilePath = QFileDialog::getSaveFileName(this, "Select file path so save graph.");
  ui->widgetCustomPlot->saveJpg(saveFilePath + ".jpg");
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
    /*
    menu->addAction("Add random graph", this, SLOT(addRandomGraph()));
    if (ui->widgetCustomPlot->selectedGraphs().size() > 0)
      menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
    if (ui->widgetCustomPlot->graphCount() > 0)
      menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
      */
    setupContexMenu(menu);
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

int PlottingWindow::indexOfPropertyOnArray(QString property)
{
  for (int i = 0; i < array.size(); i++)
  {
    if (array[i]->name == property)
    {
      return i;
    }
  }
  return -1;
}


//  -----------      ----------------                Context Menu Actions                     ----------------              ---------------- //
//

/**
 * @brief Context menu setup
 *
 * Called when user right click on plotting window
 * @code {.c++}
 * PlottingWindow::setupContexMenu(QMenu *menu)
 * @endcode
 */
void PlottingWindow::setupContexMenu(QMenu *menu)
{
  //Create context menu section for scatter style
  QMenu *scatterStyleSubmenu = menu->addMenu("Scatter Style");
  QAction *actionScatterStyle_setup = scatterStyleSubmenu->addAction("Cross", this, SLOT(changeScatterStyle()));
  QAction *action1ScatterStyle_setup = scatterStyleSubmenu->addAction("Plus", this, SLOT(changeScatterStyle()));
  QAction *action2ScatterStyle_setup = scatterStyleSubmenu->addAction("Circle", this, SLOT(changeScatterStyle()));
  QAction *action3ScatterStyle_setup = scatterStyleSubmenu->addAction("Disc", this, SLOT(changeScatterStyle()));
  QAction *action4ScatterStyle_setup = scatterStyleSubmenu->addAction("Square", this, SLOT(changeScatterStyle()));

  //Create context menu section for line color
  QMenu *colorStyleSubmenu = menu->addMenu("Line Color");
  QAction *actionStyleMenu = colorStyleSubmenu->addAction("red", this, SLOT(changeColor()));
  QAction *action1StyleMenu = colorStyleSubmenu->addAction("green", this, SLOT(changeColor()));
  QAction *action2StyleMenu = colorStyleSubmenu->addAction("blue", this, SLOT(changeColor()));
  QAction *action3StyleMenu = colorStyleSubmenu->addAction("cyan", this, SLOT(changeColor()));
  QAction *action4StyleMenu = colorStyleSubmenu->addAction("magenta", this, SLOT(changeColor()));

  //if graph selected -> also add fit graph selection
  if (ui->widgetCustomPlot->selectedGraphs().size() > 0)
    menu->addAction("Fit Graph", this, SLOT(on_FitScreen_pushButton_clicked()));
}

/**
 * @brief Changes the plotting color for graph
 *
 * Called from context menu action.
 * @code {.c++}
 * PlottingWindow::changeColor()
 * @endcode
 */
void PlottingWindow::changeColor()
{
  if (ui->widgetCustomPlot->selectedGraphs().size() > 0)
  {
    if (QAction *contextAction = qobject_cast<QAction *>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
      ui->widgetCustomPlot->selectedGraphs().first()->setPen(QPen(contextAction->iconText()));
    }
  }
}

/**
 * @brief Changes the scatter style for plot
 *
 * Called from context menu action.
 * @code {.c++}
 * PlottingWindow::changeScatterStyle()
 * @endcode
 */
void PlottingWindow::changeScatterStyle()
{
  if (ui->widgetCustomPlot->selectedGraphs().size() > 0) // if selected graph
  {
    if (QAction *contextAction = qobject_cast<QAction *>(sender())) // make sure this slot is really called by a context menu action, so it carries the data we need
    {
      if (contextAction->iconText() == "Cross")
        ui->widgetCustomPlot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(shapes[0], 5));
      else if (contextAction->iconText() == "Plus")
        ui->widgetCustomPlot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(shapes[1], 5));
      else if (contextAction->iconText() == "Circle")
        ui->widgetCustomPlot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(shapes[2], 5));
      else if (contextAction->iconText() == "Disc")
        ui->widgetCustomPlot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(shapes[3], 5));
      else if (contextAction->iconText() == "Square")
        ui->widgetCustomPlot->selectedGraphs().first()->setScatterStyle(QCPScatterStyle(shapes[4], 5));
    }
  }
}

/**
 * @brief Fit selected graph to the screen
 *
 * Fits the selected graph to the screen. pointer to the selected graph used to calculate axis
 * @code {.c++}
 * PlottingWindow::on_FitScreen_pushButton_clicked()
 * @endcode
 */
void PlottingWindow::on_FitScreen_pushButton_clicked()
{
  // Getting the pointer to the graph
  if (ui->widgetCustomPlot->selectedGraphs().size() > 0)
  {
    QCPGraph *ptr = nullptr; // ptr to graph
    ptr = ui->widgetCustomPlot->selectedGraphs().first();
    // ptr = ui->widgetCustomPlot->graph(0); //get the first selected graph

    ui->widgetCustomPlot->xAxis->setRange(ptr->data()->at(0)->key - 1, ptr->data()->at(ptr->dataCount() - 1)->key + 1);
    // ui->widgetCustomPlot->xAxis->setRange(array[0]->timeData[0].key - 1, array[0]->timeData[array[0]->timeData.size() - 1].key + 1);
    ui->widgetCustomPlot->replot();
  }
  else
  {
    qDebug() << "-> \"on_FitScreen_pushButton_clicked()\" not in selected graph mode." << endl;
    ui->widgetCustomPlot->xAxis->setRange(array[0]->timeData[0].key - 1, array[0]->timeData[array[0]->timeData.size() - 1].key + 1);
    ui->widgetCustomPlot->replot();
  }
}

//  -----------      ----------------                Properties ListView Functions                     ----------------              ---------------- //
//
/**
 * @brief Setup for properties list view tab
 *
 * Sets up the list view using the target model, goes over every row and adds the properties pointer to the list view if they don't already exists.
 * @code {.c++}
 * PlottingWindow::setupProperties_ListView()
 * @endcode
 */
void PlottingWindow::setupProperties_ListView()
{
  // creating temp item for first property
  QStandardItem *tempProperty = new QStandardItem(targetName);

  // setup properties of the list view table
  tempProperty->setCheckable(true);         // adding check boxes property
  tempProperty->setCheckState(Qt::Checked); // its checked for the first item

  // Append & set the first item to the tree since it will be the main property to begin with
  propertiesListModel = new QStandardItemModel(this);
  propertiesListModel->appendRow(tempProperty);
  ui->properties_listView->setModel(propertiesListModel);

  for (int i = 0; i < targetModelProperties->rowCount(); i++) // for each element in the target models properties list
  {
    if (!checkPropertyExistOnListView(targetModelProperties->item(i, 0)->text())) // check if its exists
    {
      QStandardItem *tempProperty = new QStandardItem(targetModelProperties->item(i, 0)->text()); // create new item for property

      // setup properties the created item & append to the list view model
      tempProperty->setCheckable(true);
      tempProperty->setCheckState(Qt::Unchecked);
      propertiesListModel->appendRow(tempProperty);
    }
  }
}

/**
 * @brief Refresh properties ui button
 *
 *  Plotting ui button for refreshing available properties. Calls the updating subroutine.
 * @code {.c++}
 * PlottingWindow::on_refreshProperties_pushButton_clicked()
 * @endcode
 */
void PlottingWindow::on_refreshProperties_pushButton_clicked()
{
  updateProperties_ListView(); // reinitalize list view
}

/**
 * @brief update function for properties list view
 *
 * Updated the existing properties list view for new data in target model. Iterates from where setup function left and
 * adds the new properties if they don't already exists.
 * @code {.c++}
 * PlottingWindow::updateProperties_ListView()
 * @endcode
 */
void PlottingWindow::updateProperties_ListView()
{
  for (int i = 0; i < targetModelProperties->rowCount(); i++) // iterate over target models properties list
  {
    if (!checkPropertyExistOnListView(targetModelProperties->item(i, 0)->text())) // check if its exists
    {
      QStandardItem *tempProperty = new QStandardItem(targetModelProperties->item(i, 0)->text()); // create new item for property

      // setup properties the created item & append to the list view model
      tempProperty->setCheckable(true);
      tempProperty->setCheckState(Qt::Unchecked);
      propertiesListModel->appendRow(tempProperty);
    }
  }
}

/**
 * @brief Fit selected graph to the screen
 *
 * Fits the selected graph to the screen. pointer to the selected graph used to calculate axis
 * @code {.c++}
 * PlottingWindow::on_FitScreen_pushButton_clicked()
 * @endcode
 */
bool PlottingWindow::checkPropertyExistOnListView(QString property)
{
  for (int i = 0; i < propertiesListModel->rowCount(); i++) // Iterate over properties
  {
    if ((propertiesListModel->item(i, 0)->text() == property) || (property == "<Property Name>")) // If matches
      return true;
  }
  return false;
}

/**
 * @brief Function emitted when you click the properties list view
 *
 * Checks which properties you clicked. Adds or removes the properties you clicked.
 * Tobe plotted array includes the name of the elements that you want it to be plotted.
 * Function changes this array in order to manipulate plotted arrays.
 * @code {.c++}
 * PlottingWindow::on_properties_listView_clicked(const QModelIndex &index)
 * @endcode
 */
void PlottingWindow::on_properties_listView_clicked(const QModelIndex &index)
{
  bool changed = false;                                     // state false at beginning
  for (int i = 0; i < propertiesListModel->rowCount(); i++) // Iterate over properties list
  {
    if (propertiesListModel->item(i, 0)->checkState() == Qt::Checked && !checkPropertyExistOnArray(propertiesListModel->item(i, 0)->text())) // If item Checked
    {
      array.push_back(new dataStruct(propertiesListModel->item(i, 0)->text())); // add properties to be plotted array
      changed = true;                                                           // mark that state changed
    }
    else if (propertiesListModel->item(i, 0)->checkState() != Qt::Checked && checkPropertyExistOnArray(propertiesListModel->item(i, 0)->text())) // If item Unchecked
    {
      changed = true;                                                                  // mark state changed
      array.removeAt(indexOfPropertyOnArray(propertiesListModel->item(i, 0)->text())); // remove selected item from to be plotted array
    }
  }
  if (changed)   // if new elementd added
    setupPlot(); // setup again
}
