#include "plottingwindow.h"
#include "ui_plottingwindow.h"

PlottingWindow::PlottingWindow(QWidget *parent) : QWidget(parent),
                                                  ui(new Ui::PlottingWindow)
{
  ui->setupUi(this);

  // setupPlot();
  setup(); // example plot
}

PlottingWindow::PlottingWindow(QStandardItemModel *target, QStandardItemModel *mainProperties, QString name, QWidget *parent) : QWidget(parent), ui(new Ui::PlottingWindow)
{
  // Setup Plotting settings

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

  ui->setupUi(this);

  setupProperties_ListView();
  setupPlot();
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

  //
  for (int i = 0; i < array.size(); i++)
  {
    for (int j = 0; j < targetModel->rowCount(); j++)
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
    ui->widgetCustomPlot->xAxis->setRange(0, array[i]->y.length() - 1);

    ui->widgetCustomPlot->replot();
  }
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
  // n^2 on large property set !
  for (int i = 0; i < array.size(); i++)
  {
    int prevSize = array[i]->y.size();
    for (int j = 0; j < targetModel->rowCount(); j++)
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
}

void PlottingWindow::startPlot()
{

  /* -> OLD EXAMPLE CODE
 QString serialBuffer = "";
 // generate some data:

 QByteArray SerialInput = serial->readLine();
 serialBuffer += QString::fromStdString(SerialInput.toStdString());
 QStringList input = serialBuffer.split(",");
 qDebug() << input.length() << endl;

 QVector<double> x(input.length()), y(input.length()); // initialize with entries 0..100
 for (int i = 0; i < input.length(); ++i)
 {
   x[i] = i;                                   // x goes from -1 to 1
   y[i] = input[i].toDouble() + verticalShift; // let's plot a quadratic function
 }
 // create graph and assign data to it:
 ui->customPlot->addGraph();
 ui->customPlot->graph(0)->setData(x, y);
 // give the axes some labels:
 ui->customPlot->xAxis->setLabel("x");
 ui->customPlot->yAxis->setLabel("y");
 // set axes ranges, so we see all data:
 ui->customPlot->xAxis->setRange(-1, y.length());
 ui->customPlot->yAxis->setRange(verticalMin, verticalMax);
 ui->customPlot->replot();
 */
}

void PlottingWindow::setup()
{
  ui->widgetCustomPlot->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
  ui->widgetCustomPlot->legend->setVisible(true);
  QFont legendFont = font();  // start out with MainWindow's font..
  legendFont.setPointSize(9); // and make a bit smaller for legend
  ui->widgetCustomPlot->legend->setFont(legendFont);
  ui->widgetCustomPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));
  // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
  ui->widgetCustomPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom | Qt::AlignRight);

  // setup for graph 0: key axis left, value axis bottom
  // will contain left maxwell-like function
  ui->widgetCustomPlot->addGraph(ui->widgetCustomPlot->yAxis, ui->widgetCustomPlot->xAxis);
  ui->widgetCustomPlot->graph(0)->setPen(QPen(QColor(255, 100, 0)));
  ui->widgetCustomPlot->graph(0)->setBrush(QBrush(QPixmap("./balboa.jpg"))); // fill with texture of specified image
  ui->widgetCustomPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
  ui->widgetCustomPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
  ui->widgetCustomPlot->graph(0)->setName("Left maxwell function");

  // setup for graph 1: key axis bottom, value axis left (those are the default axes)
  // will contain bottom maxwell-like function with error bars
  ui->widgetCustomPlot->addGraph();
  ui->widgetCustomPlot->graph(1)->setPen(QPen(Qt::red));
  ui->widgetCustomPlot->graph(1)->setBrush(QBrush(QPixmap("./balboa.jpg"))); // same fill as we used for graph 0
  ui->widgetCustomPlot->graph(1)->setLineStyle(QCPGraph::lsStepCenter);
  ui->widgetCustomPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, Qt::white, 7));
  ui->widgetCustomPlot->graph(1)->setName("Bottom maxwell function");
  QCPErrorBars *errorBars = new QCPErrorBars(ui->widgetCustomPlot->xAxis, ui->widgetCustomPlot->yAxis);
  errorBars->removeFromLegend();
  errorBars->setDataPlottable(ui->widgetCustomPlot->graph(1));

  // setup for graph 2: key axis top, value axis right
  // will contain high frequency sine with low frequency beating:
  ui->widgetCustomPlot->addGraph(ui->widgetCustomPlot->xAxis2, ui->widgetCustomPlot->yAxis2);
  ui->widgetCustomPlot->graph(2)->setPen(QPen(Qt::blue));
  ui->widgetCustomPlot->graph(2)->setName("High frequency sine");

  // setup for graph 3: same axes as graph 2
  // will contain low frequency beating envelope of graph 2
  ui->widgetCustomPlot->addGraph(ui->widgetCustomPlot->xAxis2, ui->widgetCustomPlot->yAxis2);
  QPen blueDotPen;
  blueDotPen.setColor(QColor(30, 40, 255, 150));
  blueDotPen.setStyle(Qt::DotLine);
  blueDotPen.setWidthF(4);
  ui->widgetCustomPlot->graph(3)->setPen(blueDotPen);
  ui->widgetCustomPlot->graph(3)->setName("Sine envelope");

  // setup for graph 4: key axis right, value axis top
  // will contain parabolically distributed data points with some random perturbance
  ui->widgetCustomPlot->addGraph(ui->widgetCustomPlot->yAxis2, ui->widgetCustomPlot->xAxis2);
  ui->widgetCustomPlot->graph(4)->setPen(QColor(50, 50, 50, 255));
  ui->widgetCustomPlot->graph(4)->setLineStyle(QCPGraph::lsNone);
  ui->widgetCustomPlot->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
  ui->widgetCustomPlot->graph(4)->setName("Some random data around\na quadratic function");

  // generate data, just playing with numbers, not much to learn here:
  QVector<double> x0(25), y0(25);
  QVector<double> x1(15), y1(15), y1err(15);
  QVector<double> x2(250), y2(250);
  QVector<double> x3(250), y3(250);
  QVector<double> x4(250), y4(250);
  for (int i = 0; i < 25; ++i) // data for graph 0
  {
    x0[i] = 3 * i / 25.0;
    y0[i] = qExp(-x0[i] * x0[i] * 0.8) * (x0[i] * x0[i] + x0[i]);
  }
  for (int i = 0; i < 15; ++i) // data for graph 1
  {
    x1[i] = 3 * i / 15.0;
    ;
    y1[i] = qExp(-x1[i] * x1[i]) * (x1[i] * x1[i]) * 2.6;
    y1err[i] = y1[i] * 0.25;
  }
  for (int i = 0; i < 250; ++i) // data for graphs 2, 3 and 4
  {
    x2[i] = i / 250.0 * 3 * M_PI;
    x3[i] = x2[i];
    x4[i] = i / 250.0 * 100 - 50;
    y2[i] = qSin(x2[i] * 12) * qCos(x2[i]) * 10;
    y3[i] = qCos(x3[i]) * 10;
    y4[i] = 0.01 * x4[i] * x4[i] + 1.5 * (rand() / (double)RAND_MAX - 0.5) + 1.5 * M_PI;
  }

  // pass data points to graphs:
  ui->widgetCustomPlot->graph(0)->setData(x0, y0);
  ui->widgetCustomPlot->graph(1)->setData(x1, y1);
  errorBars->setData(y1err);
  ui->widgetCustomPlot->graph(2)->setData(x2, y2);
  ui->widgetCustomPlot->graph(3)->setData(x3, y3);
  ui->widgetCustomPlot->graph(4)->setData(x4, y4);
  // activate top and right axes, which are invisible by default:
  ui->widgetCustomPlot->xAxis2->setVisible(true);
  ui->widgetCustomPlot->yAxis2->setVisible(true);
  // set ranges appropriate to show data:
  ui->widgetCustomPlot->xAxis->setRange(0, 2.7);
  ui->widgetCustomPlot->yAxis->setRange(0, 2.6);
  ui->widgetCustomPlot->xAxis2->setRange(0, 3.0 * M_PI);
  ui->widgetCustomPlot->yAxis2->setRange(-70, 35);
  // set pi ticks on top axis:
  ui->widgetCustomPlot->xAxis2->setTicker(QSharedPointer<QCPAxisTickerPi>(new QCPAxisTickerPi));
  // add title layout element:
  ui->widgetCustomPlot->plotLayout()->insertRow(0);
  ui->widgetCustomPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ui->widgetCustomPlot, "Way too many graphs in one plot", QFont("sans", 12, QFont::Bold)));
  // set labels:
  ui->widgetCustomPlot->xAxis->setLabel("Bottom axis with outward ticks");
  ui->widgetCustomPlot->yAxis->setLabel("Left axis label");
  ui->widgetCustomPlot->xAxis2->setLabel("Top axis label");
  ui->widgetCustomPlot->yAxis2->setLabel("Right axis label");
  // make ticks on bottom axis go outward:
  ui->widgetCustomPlot->xAxis->setTickLength(0, 5);
  ui->widgetCustomPlot->xAxis->setSubTickLength(0, 3);
  // make ticks on right axis go inward and outward:
  ui->widgetCustomPlot->yAxis2->setTickLength(3, 3);
  ui->widgetCustomPlot->yAxis2->setSubTickLength(1, 1);
}

void PlottingWindow::on_pushButton_clicked()
{
  QString saveFilePath = QFileDialog::getSaveFileName(this, "Select file path so save graph.");
  ui->widgetCustomPlot->saveJpg(saveFilePath + ".jpg");
}

void PlottingWindow::on_addProperty_pushButton_clicked()
{
}

void PlottingWindow::on_properties_listView_clicked(const QModelIndex &index)
{
}
