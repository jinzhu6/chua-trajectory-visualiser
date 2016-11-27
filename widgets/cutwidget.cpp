#include "widgets/cutwidget.h"
#include "ui_cutwidget.h"

CutWidget::CutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutWidget)
{
    ui->setupUi(this);

    this->ui->progressBar->setDisabled(true);

    this->calculatButtonSignalMapper = new QSignalMapper(this);

    this->connect(this->calculatButtonSignalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(calculateButtonPressed(QWidget*)));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_u1i, this->ui->button_calculate_u1i);
    this->connect(this->ui->button_calculate_u1i, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_parallel_u1i, this->ui->button_calculate_parallel_u1i);
    this->connect(this->ui->button_calculate_parallel_u1i, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_u2i, this->ui->button_calculate_u2i);
    this->connect(this->ui->button_calculate_u2i, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_parallel_u2i, this->ui->button_calculate_parallel_u2i);
    this->connect(this->ui->button_calculate_parallel_u2i, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_u1u2, this->ui->button_calculate_u1u2);
    this->connect(this->ui->button_calculate_u1u2, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->calculatButtonSignalMapper->setMapping(this->ui->button_calculate_parallel_u1u2, this->ui->button_calculate_parallel_u1u2);
    this->connect(this->ui->button_calculate_parallel_u1u2, SIGNAL(clicked()), this->calculatButtonSignalMapper, SLOT(map()));

    this->testChangedSignalMapper = new QSignalMapper(this);

    this->connect(this->testChangedSignalMapper, SIGNAL(mapped(QWidget*)), this, SLOT(testExpressionChanged(QWidget*)));

    this->testChangedSignalMapper->setMapping(this->ui->test_chaos, this->ui->test_chaos);
    this->connect(this->ui->test_chaos, SIGNAL(textChanged()), this->testChangedSignalMapper, SLOT(map()));

    this->testChangedSignalMapper->setMapping(this->ui->test_lc, this->ui->test_lc);
    this->connect(this->ui->test_lc, SIGNAL(textChanged()), this->testChangedSignalMapper, SLOT(map()));

    this->connect(this->ui->button_cancel_u1i, SIGNAL(clicked()), this, SLOT(cancelCalculation()));
    this->connect(this->ui->button_cancel_u2i, SIGNAL(clicked()), this, SLOT(cancelCalculation()));
    this->connect(this->ui->button_cancel_u1u2, SIGNAL(clicked()), this, SLOT(cancelCalculation()));

    this->connect(&this->updateProgressTimer, SIGNAL(timeout()), this, SLOT(updateProgressBar()));
    this->connect(&this->FutureWatcher, SIGNAL (finished()), this, SLOT (calculationFinished()));
    this->initPlot();
}

CutWidget::~CutWidget()
{
    delete ui;
    delete parameters;
    delete calculator;
    delete colorMap;
    delete calculatButtonSignalMapper;
}

void CutWidget::setParameters(CircuitParameters* parameters){
    this->parameters = parameters;
}


void CutWidget::updateGuiByParameters(){
    this->ui->value_c1->setText(QString::number(parameters->C1));
    this->ui->value_c2->setText(QString::number(parameters->C2));
    this->ui->value_l->setText(QString::number(parameters->L));
    this->ui->value_bp->setText(QString::number(parameters->Bp));
    this->ui->value_b0->setText(QString::number(parameters->B0));
    this->ui->value_r->setText(QString::number(parameters->R));
    this->ui->value_ro->setText(QString::number(parameters->ro));
    this->ui->value_i->setText(QString::number(parameters->I));
    this->ui->value_m0->setText(QString::number(parameters->m0));
    this->ui->value_m1->setText(QString::number(parameters->m1));
    this->ui->value_m2->setText(QString::number(parameters->m2));
    this->ui->value_tmax->setText(QString::number(parameters->t_max));
    this->ui->value_h0->setText(QString::number(parameters->h0));
    this->ui->value_ihmax->setText(QString::number(parameters->iStepMax));
    this->ui->value_uhmax->setText(QString::number(parameters->uStepMax));
}

void CutWidget::initPlot(){
    QCustomPlot* customPlot = this->ui->plot_cut;
    this->colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);

    colorMap->setGradient(QCPColorGradient::gpHot);
    colorMap->setAntialiased(false);
    colorMap->setInterpolate(false);
    colorMap->setDataRange(QCPRange(0,100));
}

void CutWidget::calculateButtonPressed(QWidget* button){
    if(button == ui->button_calculate_u1i){
        this->reCalculate(U1_I, false);
    }else if(button == ui->button_calculate_parallel_u1i){
        this->reCalculate(U1_I, true);
    }else if(button == ui->button_calculate_u2i){
        this->reCalculate(U2_I, false);
    }else if(button == ui->button_calculate_parallel_u2i){
        this->reCalculate(U2_I, true);
    }else if(button == ui->button_calculate_u1u2){
        this->reCalculate(U1_U2, false);
    }else if(button == ui->button_calculate_parallel_u1u2){
        this->reCalculate(U1_U2, true);
    }
}

void CutWidget::reCalculate(CrossSectionType type, bool parallel){
    this->calculator = new TrajectoryCalculator(this->parameters);

    double xMin, xMax, xStep, yMin, yMax, yStep, z;
    switch (type) {
    case U1_I:
        xMin = this->ui->input_u1_from_u1i->value();
        xMax = this->ui->input_u1_to_u1i->value();
        xStep = this->ui->input_u1_step_u1i->value();
        yMin = this->ui->input_i_from_u1i->value();
        yMax = this->ui->input_i_to_u1i->value();
        yStep = this->ui->input_i_step_u1i->value();
        z = this->ui->input_u2_u1i->value();

        this->ui->button_cancel_u1i->setEnabled(true);
        break;
    case U2_I:
        xMin = this->ui->input_u2_from_u2i->value();
        xMax = this->ui->input_u2_to_u2i->value();
        xStep = this->ui->input_u2_step_u2i->value();
        yMin = this->ui->input_i_from_u2i->value();
        yMax = this->ui->input_i_to_u2i->value();
        yStep = this->ui->input_i_step_u2i->value();
        z = this->ui->input_u1_u2i->value();

        this->ui->button_cancel_u2i->setEnabled(true);
        break;
    case U1_U2:
        xMin = this->ui->input_u1_from_u1u2->value();
        xMax = this->ui->input_u1_to_u1u2->value();
        xStep = this->ui->input_u1_step_u1u2->value();
        yMin = this->ui->input_u2_from_u1u2->value();
        yMax = this->ui->input_u2_to_u1u2->value();
        yStep = this->ui->input_u2_step_u1u2->value();
        z = this->ui->input_i_u1u2->value();

        this->ui->button_cancel_u1u2->setEnabled(true);
        break;
    }

    std::string chaosTest = this->ui->test_chaos->toPlainText().toStdString();
    std::string lcTest = this->ui->test_lc->toPlainText().toStdString();

    QFuture<CalculatedCut*> future;
    if(parallel){
        future = QtConcurrent::run(std::bind(&TrajectoryCalculator::parallelCalculateCut, this->calculator, type, xMin, xMax, xStep, yMin, yMax, yStep, z, chaosTest, lcTest));
    }else{
        future = QtConcurrent::run(std::bind(&TrajectoryCalculator::calculateCut, this->calculator, type, xMin, xMax, xStep, yMin, yMax, yStep, z, chaosTest, lcTest));
    }
    this->FutureWatcher.setFuture(future);
    this->clock.start();

    this->colorMap->data()->clear();

    this->ui->button_calculate_u1i->setDisabled(true);
    this->ui->button_calculate_parallel_u1i->setDisabled(true);
    this->ui->button_calculate_u2i->setDisabled(true);
    this->ui->button_calculate_parallel_u2i->setDisabled(true);
    this->ui->button_calculate_u1u2->setDisabled(true);
    this->ui->button_calculate_parallel_u1u2->setDisabled(true);

    this->ui->time->setDisabled(false);
    this->ui->resultTable->setDisabled(true);
    this->ui->progressBar->setEnabled(true);

    this->ui->plot_cut->setInteraction(QCP::iRangeDrag, false);
    this->ui->plot_cut->setInteraction(QCP::iRangeZoom, false);

    this->updateProgressTimer.start(50);
    this->lastProgress = 0;

}

void CutWidget::cancelCalculation(){
    this->ui->progressBar->setValue(0);
    this->ui->progressBar->setDisabled(true);
    this->ui->button_cancel_u1u2->setDisabled(true);

}

void CutWidget::calculationFinished(){
    int time = this->clock.elapsed();

    CalculatedCut* cut = this->FutureWatcher.result();
    this->updateProgressTimer.stop();

    this->ui->button_cancel_u1i->setDisabled(true);
    this->ui->button_cancel_u2i->setDisabled(true);
    this->ui->button_cancel_u1u2->setDisabled(true);
    this->ui->button_calculate_u1i->setEnabled(true);

    this->ui->button_calculate_parallel_u1i->setEnabled(true);
    this->ui->button_calculate_u2i->setEnabled(true);
    this->ui->button_calculate_parallel_u2i->setEnabled(true);
    this->ui->button_calculate_u1u2->setEnabled(true);
    this->ui->button_calculate_parallel_u1u2->setEnabled(true);

    this->ui->progressBar->setValue(0);
    this->ui->progressBar->setDisabled(true);

    this->ui->time->setDisabled(true);
    this->ui->time->setText("0:00");


    this->initForCut(cut);
    this->reDraw(cut);

    this->ui->plot_cut->setInteraction(QCP::iRangeDrag, true);
    this->ui->plot_cut->setInteraction(QCP::iRangeZoom, true);

    this->updateResultTabe(cut, time);
    this->ui->resultTable->setEnabled(true);

    cut->writeToCSV("cut.txt");

    delete cut;
    delete this->calculator;
    this->calculator = NULL;
}

void CutWidget::testExpressionChanged(QWidget* textEdit){
    QString expressionString;
    QLabel* validityLabel;
    if(textEdit == this->ui->test_chaos){
        expressionString = this->ui->test_chaos->toPlainText();
        validityLabel = this->ui->test_chaos_validity;
    }else{
        expressionString = this->ui->test_lc->toPlainText();
        validityLabel = this->ui->test_lc_validity;
    }

    if(expressionString.isEmpty()){
        validityLabel->setDisabled(true);
        validityLabel->setText(QString("empty"));

        return;
    }

    if(this->calculator->isExpressionValid(expressionString.toStdString())){
        validityLabel->setEnabled(true);
        validityLabel->setText(QString("valid"));

        if(textEdit == this->ui->test_chaos){

        }else{

        }
    }else{
        validityLabel->setEnabled(true);
        validityLabel->setText(QString("invalid"));

        if(textEdit == this->ui->test_chaos){

        }else{

        }
    }
}

void CutWidget::initForCut(CalculatedCut* cut){
    QCustomPlot* customPlot = this->ui->plot_cut;
    if(cut->type == U1_I){
        customPlot->xAxis->setLabel("u1");
        customPlot->yAxis->setLabel("i");
    }else if(cut->type == U2_I){
        customPlot->xAxis->setLabel("u2");
        customPlot->yAxis->setLabel("i");
    }else if(cut->type == U1_U2){
        customPlot->xAxis->setLabel("u1");
        customPlot->yAxis->setLabel("u2");
    }

    this->colorMap->data()->setSize(cut->u1Size, cut->u2Size);
    this->colorMap->data()->setRange(QCPRange(cut->u1Min, cut->u1Max), QCPRange(cut->u2Min, cut->u2Max)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    customPlot->xAxis->setRange(cut->u1Min, cut->u1Max);
    customPlot->yAxis->setRange(cut->u2Min, cut->u2Max);
}

void CutWidget::reDraw(CalculatedCut* cut){
    QCustomPlot* customPlot = this->ui->plot_cut;

    for (std::vector<std::vector<CalculatedCut::TrajectoryResult>>::const_iterator vector_iterator = cut->cbegin(); vector_iterator != cut->cend(); ++vector_iterator) {
        for (std::vector<CalculatedCut::TrajectoryResult>::const_iterator result_iterator = vector_iterator->cbegin(); result_iterator != vector_iterator->cend(); ++result_iterator) {
            switch(result_iterator->result){
            case TrajectoryResultType::CHA:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 100);
                break;
            case TrajectoryResultType::LC:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 60);
                break;
            case TrajectoryResultType::UNDETERMINED:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 20);
                break;
            }
        }
    }

    customPlot->replot();
}

void CutWidget::reDrawPartial(PartiallyCalculatedCut* cut){
    QCustomPlot* customPlot = this->ui->plot_cut;

    for (std::list<std::vector<CalculatedCut::TrajectoryResult>*>::const_iterator vector_iterator = cut->cbeginU1Columns(); vector_iterator != cut->cendU1Columns(); ++vector_iterator) {
        for (std::vector<CalculatedCut::TrajectoryResult>::const_iterator result_iterator = (*vector_iterator)->cbegin(); result_iterator != (*vector_iterator)->cend(); ++result_iterator) {
            switch(result_iterator->result){
            case TrajectoryResultType::CHA:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 100);
                break;
            case TrajectoryResultType::LC:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 60);
                break;
            case TrajectoryResultType::UNDETERMINED:
                this->colorMap->data()->setData(result_iterator->u1, result_iterator->u2, 20);
                break;
            }
        }
    }

    customPlot->replot();
}

void CutWidget::updateResultTabe(CalculatedCut* cut, int timeInMs){
    int chaosPoints = 0;
    int lcPoints = 0;
    int undPoints = 0;
    for (std::vector<std::vector<CalculatedCut::TrajectoryResult>>::const_iterator vector_iterator = cut->cbegin(); vector_iterator != cut->cend(); ++vector_iterator) {
        for (std::vector<CalculatedCut::TrajectoryResult>::const_iterator result_iterator = vector_iterator->cbegin(); result_iterator != vector_iterator->cend(); ++result_iterator) {
            switch(result_iterator->result){
            case TrajectoryResultType::CHA:
                ++chaosPoints;
                break;
            case TrajectoryResultType::LC:
                ++lcPoints;
                break;
            case TrajectoryResultType::UNDETERMINED:
                ++undPoints;
                break;
            }
        }
    }

    QTableWidgetItem *resolution = new QTableWidgetItem;
    resolution->setText(QString("%1 x %2").arg(cut->u1Size).arg(cut->u2Size));

    QTableWidgetItem *time = new QTableWidgetItem;
    time->setText(this->formatTime(timeInMs));

    int allPoints = cut->u1Size * cut->u2Size;

    float chaosPercent = ((float)chaosPoints / (float)allPoints) * 100;
    QTableWidgetItem *chaos = new QTableWidgetItem;
    chaos->setText(QString("%1% (%2)").arg(chaosPercent, 0, 'f', 2).arg(chaosPoints));

    float lcPercent = ((float)lcPoints / (float)allPoints) * 100;
    QTableWidgetItem *lc = new QTableWidgetItem;
    lc->setText(QString("%1% (%2)").arg(lcPercent, 0, 'f', 2).arg(lcPoints));

    float undPercent = ((float)undPoints / (float)allPoints) * 100;
    QTableWidgetItem *und = new QTableWidgetItem;
    und->setText(QString("%1% (%2)").arg(undPercent, 0, 'f', 2).arg(undPoints));

    this->ui->resultTable->setItem(0,0, resolution);
    this->ui->resultTable->setItem(0,1, time);
    this->ui->resultTable->setItem(0,2, chaos);
    this->ui->resultTable->setItem(0,3, lc);
    this->ui->resultTable->setItem(0,4, und);
}

QString CutWidget::formatTime(int timeInMs){
    int elapsedMins = timeInMs/1000/60;
    int elapsedSeconds = timeInMs/1000 - std::floor(timeInMs/1000/60)*60;
    return QString("%1:%2").arg(elapsedMins).arg(elapsedSeconds,2, 10, QChar('0'));
}

void CutWidget::updateProgressBar()
{
    int elapsed = this->clock.elapsed();
    QString formattedTime = this->formatTime(elapsed);
    this->ui->time->setText(formattedTime);

    if(!this->calculator->hasPartialResult()){
        return;
    }

    PartiallyCalculatedCut* cut = this->calculator->partialResult();

    if(cut->progress() == this->lastProgress){
        return;
    }

    double rawProgress = cut->progress();
    int progress = std::round(rawProgress * 100);
    this->ui->progressBar->setValue(progress);

    this->initForCut(cut);
    this->reDrawPartial(cut);
    this->lastProgress = cut->progress();
}
