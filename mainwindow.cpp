#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{


    ui->setupUi(this);

    this->loadedFileLabel = new QLabel();
    this->ui->statusBar->addPermanentWidget(this->loadedFileLabel);


    std::string cwd;
    char *buffer = new char[256];
    getcwd(buffer,256);
    if(buffer != NULL){
        cwd = std::string(buffer);
        delete[] buffer;
    }else{
        cwd = std::string("");
    }

    this->loadParametersFromFile(cwd + "/parameters.txt");
    this->ui->trajectory->updateParameters(this->parameters);
    this->ui->crosssection->updateParameters(this->parameters);

    this->ui->trajectory->setTestTable(this->ui->crosssection->findChild<TestInputWidget*>("test"));
    this->ui->csserie->setTestTable(this->ui->crosssection->findChild<TestInputWidget*>("test"));

    this->connect(this->ui->actionLoad_parameters, &QAction::triggered, this, &MainWindow::loadParametersAction);
    this->connect(this->ui->actionSave_parameters, &QAction::triggered, this, &MainWindow::saveParametersAction);
    this->connect(this->ui->actionExit, &QAction::triggered, this, &MainWindow::exitAction);
    this->connect(this->ui->actionTrajectory, &QAction::triggered, this, &MainWindow::switchToTrajectoryAction);
    this->connect(this->ui->actionCrossSection, &QAction::triggered, this, &MainWindow::switchToCrossSectionAction);
    this->connect(this->ui->actionCS_Serie, &QAction::triggered, this, &MainWindow::switchToCSSerieAction);
    this->connect(this->ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(aboutAction()));

    this->connect(this->ui->trajectory, SIGNAL(parametersChanged(CircuitParameters*)), this, SLOT(parametersChangedInTrajectory(CircuitParameters*)));
}



void MainWindow::parametersChangedInTrajectory(CircuitParameters* parameters){
    if(this->parameters != parameters){
        delete this->parameters;
        this->parameters = parameters;
    }

    this->ui->crosssection->updateParameters(this->parameters);
    this->ui->csserie->updateParameters(this->parameters);
}

std::string MainWindow::trim(const std::string& str, const std::string& whitespace = " \t"){
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

void MainWindow::loadParametersFromFile(std::string filename){
    double C1, C2, R, L, I;
    double Bp, B0, m0, m1, m2;
    double ro, tmax, h0, uhmax, ihmax;
    double n, u2_0, u1_0, i_0, t_test;
    double nth, pMax;
    VACharacteristic vaChar = VACharacteristic::THREE_SEGMENT;
    double va_h0, va_h1, va_h2, va_h3;
    double u1_from_u1i, u1_to_u1i, u1_points_u1i, u2_u1i, i_from_u1i, i_to_u1i, i_points_u1i;
    double i_from_u2i, u2_from_u2i, u2_to_u2i, u2_points_u2i, u1_u2i, i_to_u2i, i_points_u2i;
    double u1_from_u1u2, u1_to_u1u2, u1_points_u1u2, u2_from_u1u2, u2_to_u1u2, u2_points_u1u2, i_u1u2;

    double i_from_u2i_cs, i_to_u2i_cs, i_points_u2i_cs, u2_from_u2i_cs, u2_to_u2i_cs, u2_points_u2i_cs, u1_from_u2i_cs, u1_to_u2i_cs, u1_points_u2i_cs;
    double i_from_u1i_cs, i_to_u1i_cs, i_points_u1i_cs, u2_from_u1i_cs, u2_to_u1i_cs, u2_points_u1i_cs, u1_from_u1i_cs, u1_to_u1i_cs, u1_points_u1i_cs;
    double i_from_u1u2_cs, i_to_u1u2_cs, i_points_u1u2_cs, u2_from_u1u2_cs, u2_to_u1u2_cs, u2_points_u1u2_cs, u1_from_u1u2_cs, u1_to_u1u2_cs, u1_points_u1u2_cs;

    std::ifstream file;
    file.open(filename);
    file >> std::setprecision(15) >> C1 >> C2 >> R >> L >> ro >> m0 >> m1 >> i_0 >> u2_0 >> u1_0 >>
                                     m2 >> I >> Bp >> B0 >> tmax >> h0 >> uhmax >> ihmax >> t_test >> n >>
                                     nth >> pMax >>
                                     u1_from_u1i >> u1_to_u1i >> u1_points_u1i >> u2_u1i >> i_from_u1i >> i_to_u1i >> i_points_u1i >>
                                     i_from_u2i >> u2_from_u2i >> u2_to_u2i >> u2_points_u2i >> u1_u2i >> i_to_u2i >> i_points_u2i >>
                                     u1_from_u1u2 >> u1_to_u1u2 >> u1_points_u1u2 >> u2_from_u1u2 >> u2_to_u1u2 >> u2_points_u1u2 >> i_u1u2;

    int testCount;
    file >> testCount;
    this->ui->crosssection->findChild<TestInputWidget*>("test")->clearRows();
    for(int i = 0; i < testCount; i++){
        std::string name, type, color;
        bool wait;
        double u1Lo, u1Hi, u2Lo, u2Hi, iLo, iHi;
        std::getline(file, name, '\t');
        name = trim(name, "\n");
        std::getline(file, type, '\t');
        type = trim(type, "\n");

        std::getline(file, color, '\t');
        color = trim(color, "\n");

        // Backward compatibility of wait parameter
        if(color.compare("0") == 0){
            wait = false;
            std::getline(file, color, '\t');
        }else if(color.compare("1") == 0){
            wait = true;
            std::getline(file, color, '\t');
        }else{
            if(type.compare("Chaos") == 0){
                wait = true;
            }else{
                wait = false;
            }
        }

        color = trim(color, "\n");

        file >> u1Lo >> u1Hi >> u2Lo >> u2Hi >> iLo >> iHi;

        this->ui->crosssection->findChild<TestInputWidget*>("test")->addRow(QString::fromStdString(name), QString::fromStdString(type), wait, QString::fromStdString(color), u1Lo, u1Hi, u2Lo, u2Hi, iLo, iHi);
    }

    file >> u2_from_u2i_cs >> u2_to_u2i_cs >> u2_points_u2i_cs >> u1_from_u2i_cs >> u1_to_u2i_cs >> u1_points_u2i_cs >> i_from_u2i_cs >> i_to_u2i_cs >> i_points_u2i_cs;
    file >> u2_from_u1i_cs >> u2_to_u1i_cs >> u2_points_u1i_cs >> u1_from_u1i_cs >> u1_to_u1i_cs >> u1_points_u1i_cs >> i_from_u1i_cs >> i_to_u1i_cs >> i_points_u1i_cs;
    file >> u2_from_u1u2_cs >> u2_to_u1u2_cs >> u2_points_u1u2_cs >> u1_from_u1u2_cs >> u1_to_u1u2_cs >> u1_points_u1u2_cs >> i_from_u1u2_cs >> i_to_u1u2_cs >> i_points_u1u2_cs;

    int vaCharInt;
    file >> vaCharInt >> va_h0 >> va_h1 >> va_h2 >> va_h3;
    vaChar = static_cast<VACharacteristic>(vaCharInt);

    file.close();

    this->parameters = new CircuitParameters(C1, C2, R, L, I, Bp, B0, m0, m1, m2, ro, tmax, h0, uhmax, ihmax, n, t_test, nth, pMax, vaChar, va_h0, va_h1, va_h2, va_h3);

    this->ui->trajectory->findChild<QDoubleSpinBox*>("input_u1_0")->setValue(u1_0);
    this->ui->trajectory->findChild<QDoubleSpinBox*>("input_u2_0")->setValue(u2_0);
    this->ui->trajectory->findChild<QDoubleSpinBox*>("input_i_0")->setValue(i_0);

    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_from_u1i")->setValue(u1_from_u1i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_to_u1i")->setValue(u1_to_u1i);
    this->ui->crosssection->findChild<QSpinBox*>("input_u1_points_u1i")->setValue(u1_points_u1i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_u1i")->setValue(u2_u1i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_from_u1i")->setValue(i_from_u1i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_to_u1i")->setValue(i_to_u1i);
    this->ui->crosssection->findChild<QSpinBox*>("input_i_points_u1i")->setValue(i_points_u1i);

    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_from_u2i")->setValue(i_from_u2i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_from_u2i")->setValue(u2_from_u2i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_to_u2i")->setValue(u2_to_u2i);
    this->ui->crosssection->findChild<QSpinBox*>("input_u2_points_u2i")->setValue(u2_points_u2i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_u2i")->setValue(u1_u2i);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_to_u2i")->setValue(i_to_u2i);
    this->ui->crosssection->findChild<QSpinBox*>("input_i_points_u2i")->setValue(i_points_u2i);

    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_from_u1u2")->setValue(u1_from_u1u2);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_to_u1u2")->setValue(u1_to_u1u2);
    this->ui->crosssection->findChild<QSpinBox*>("input_u1_points_u1u2")->setValue(u1_points_u1u2);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_from_u1u2")->setValue(u2_from_u1u2);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_to_u1u2")->setValue(u2_to_u1u2);
    this->ui->crosssection->findChild<QSpinBox*>("input_u2_points_u1u2")->setValue(u2_points_u1u2);
    this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_u1u2")->setValue(i_u1u2);

    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u2i")->setValue(i_from_u2i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u2i")->setValue(i_to_u2i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_i_points_u2i")->setValue(i_points_u2i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u2i")->setValue(u2_from_u2i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u2i")->setValue(u2_to_u2i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u2_points_u2i")->setValue(u2_points_u2i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u2i")->setValue(u1_from_u2i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u2i")->setValue(u1_to_u2i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u1_cs_u2i")->setValue(u1_points_u2i_cs);

    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u1i")->setValue(i_from_u1i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u1i")->setValue(i_to_u1i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_i_points_u1i")->setValue(i_points_u1i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u1i")->setValue(u2_from_u1i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u1i")->setValue(u2_to_u1i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u2_cs_u1i")->setValue(u2_points_u1i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u1i")->setValue(u1_from_u1i_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u1i")->setValue(u1_to_u1i_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u1_points_u1i")->setValue(u1_points_u1i_cs);

    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u1u2")->setValue(i_from_u1u2_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u1u2")->setValue(i_to_u1u2_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_i_cs_u1u2")->setValue(i_points_u1u2_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u1u2")->setValue(u2_from_u1u2_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u1u2")->setValue(u2_to_u1u2_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u2_points_u1u2")->setValue(u2_points_u1u2_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u1u2")->setValue(u1_from_u1u2_cs);
    this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u1u2")->setValue(u1_to_u1u2_cs);
    this->ui->csserie->findChild<QSpinBox*>("input_u1_points_u1u2")->setValue(u1_points_u1u2_cs);

    this->ui->trajectory->updateParameters(this->parameters);
    this->ui->crosssection->updateParameters(this->parameters);
    this->ui->csserie->updateParameters(this->parameters);

    this->setStatusText(filename);
}


void MainWindow::setStatusText(std::string filename){
    /*std::string truncatedFileName;
    if(filename.length() > 60){
        truncatedFileName = "..." + filename.substr(filename.length() - 55, 55);
    }else{
        truncatedFileName = filename;
    }
    loadedFileLabel->setText(QString(("Loaded file: " +truncatedFileName).c_str()));
    */

    loadedFileLabel->setText(QString(("Loaded file: " +filename).c_str()));
}


void MainWindow::loadParametersAction(){
    QString fileName = QFileDialog::getOpenFileName(this, QString("Open parameters file"), QString(), QString("Text files (*.txt);;All Files (*)"));

    if(fileName.isEmpty()){
        return;
    }

    this->loadParametersFromFile(fileName.toStdString());
}

void MainWindow::saveParametersAction(){
    QString fileName = QFileDialog::getSaveFileName(this, QString("Save parameters file"), QString(), QString("Text files (*.txt);;All Files (*)"));

    if(fileName.isEmpty()){
        return;
    }

    std::ofstream output;
    output.open(fileName.toStdString());

    std::vector<TrajectoryTest>* tests = this->ui->crosssection->findChild<TestInputWidget*>("test")->getTests();

    output << std::fixed << std::setprecision(15) <<

              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_C1")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_C2")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_R")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_L")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_ro")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_m0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_m1")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_i_0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_u2_0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_u1_0")->value() << "\n" <<

              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_m2")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_I")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_Bp")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_B0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_tmax")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_h0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_uhmax")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_ihmax")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_t_test")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_n")->value() << "\t" <<
              this->ui->trajectory->findChild<QSpinBox*>("input_nth")->value() << "\t" <<
              this->ui->trajectory->findChild<QSpinBox*>("input_pmax")->value() << "\n\n" <<


              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_from_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_to_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_u1_points_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_from_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_to_u1i")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_i_points_u1i")->value() << "\n" <<

              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_from_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_from_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_to_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_u2_points_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_to_u2i")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_i_points_u2i")->value() << "\n" <<

              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_from_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u1_to_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_u1_points_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_from_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_u2_to_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QSpinBox*>("input_u2_points_u1u2")->value() << "\t" <<
              this->ui->crosssection->findChild<QDoubleSpinBox*>("input_i_u1u2")->value() << "\n\n" <<
              tests->size() << "\n";

              for(std::vector<TrajectoryTest>::const_iterator test = tests->cbegin(); test != tests->cend(); ++test){
                  std::string typeString;
                  if(test->type == CHA){
                      typeString = "Chaos";
                  }else{
                      typeString = "LC";
                  }
                 output << test->name << "\t" << typeString << "\t"  << test->wait << "\t" << test->color << "\t"  << test->u1Lo << "\t" << test->u1Hi << "\t" << test->u2Lo << "\t" << test->u2Hi << "\t" << test->iLo << "\t" << test->iHi << "\n";
              }

              output << "\n" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u2_points_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u1_cs_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u2i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_i_points_u2i")->value() << "\n" <<

              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u2_cs_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u1_points_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u1i")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_i_points_u1i")->value() << "\n" <<

              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_from_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u2_to_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u2_points_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_from_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_u1_to_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_u1_points_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_from_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QDoubleSpinBox*>("input_i_to_u1u2")->value() << "\t" <<
              this->ui->csserie->findChild<QSpinBox*>("input_i_cs_u1u2")->value() << "\n\n";

              if(this->ui->trajectory->findChild<QRadioButton*>("va_three")->isChecked()){
                  output << VACharacteristic::THREE_SEGMENT << "\t";
              }else if(this->ui->trajectory->findChild<QRadioButton*>("va_five")->isChecked()){
                  output << VACharacteristic::FIVE_SEGMENT << "\t";
              }else if(this->ui->trajectory->findChild<QRadioButton*>("va_cubic")->isChecked()){
                  output << VACharacteristic::CUBIC << "\t";
              }

              output <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_va_h0")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_va_h1")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_va_h2")->value() << "\t" <<
              this->ui->trajectory->findChild<QDoubleSpinBox*>("input_va_h3")->value() << "\n";


              output << "\n\nC1\tC2\tR\tL\tro\tm0\tm1\ti_zp\tu2_zp\tu1_zp\n" <<
              "m2\tI\tBp\tBo\ttmax\tho\tuhmax\tihmax\tt_test\tn\tnth\tpMax\n\n" <<
              "u1_from_u1i\tu1_to_u1i\tu1_points_u1i\tu2_u1i\ti_from_u1i\ti_to_u1i\ti_points_u1i\n" <<
              "i_from_u2i\tu2_from_u2i\tu2_to_u2i\tu2_points_u2i\tu1_u2i\ti_to_u2i\ti_points_u2i\n" <<
              "u1_from_u1u2\tu1_to_u1u2\tu1_points_u1u2\tu2_from_u1u2\tu2_to_u1u2\tu2_points_u1u2\ti_u1u2\n\n" <<
              "number_of_tests_for_attractors\n" <<
              "test_name\ttest_type\ttest_wait\ttest_color\ttest_u1Lo\ttest_u1Hi\ttest_u2Lo\ttest_u2Hi\ttest_iLo\ttest_iHi\n...\n\nCS serie\n" <<
              "u2_from_u2i\tu2_to_u2i\tu2_points_u2i\tu1_from_u2i\tu1_to_u2i\tu1_cs_u2i\ti_from_u2i\ti_to_u2i\ti_points_u2i\n" <<
              "u2_from_u1i\tu2_to_u1i\tu2_cs_u1i\tu1_from_u1i\tu1_to_u1i\tu1_points_u1i\ti_to_u1i\ti_to_u1i\ti_points_u1i\n" <<
              "u2_from_u1u2\tu2_to_u1u2\tu2_points_u1u2\tu1_from_u1u2\tu1_to_u1u2\tu1_points_u1u2\ti_from_u1u2\ti_to_u1u2\ti_cs_u1u2\n\n" <<
              "va_char_type\tva_h0\tva_h1\t_va_h2\tva_h3\n";


    output.close();
    this->setStatusText(fileName.toStdString());
}

void MainWindow::aboutAction(){
    QMessageBox::about(this, "About Chuaviz", "<h3>Chuaviz 1.1</h3><p>Chuaviz is a tool to research chaotic phenomena by taking place in Chua's circuit.</p><p>Zsolt Rácz, 2017, Technical University of Košice</p>");
}

void MainWindow::exitAction()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent* event){
    qApp->closeAllWindows();
}

void MainWindow::switchToTrajectoryAction(){
    this->ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::switchToCrossSectionAction(){
    this->ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::switchToCSSerieAction(){
    this->ui->stackedWidget->setCurrentIndex(2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

