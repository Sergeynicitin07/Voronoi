#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    resize(800, 600);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *controlsLayout = new QHBoxLayout();

    QLabel *label = new QLabel("Number of points:", this);

    pointsSpinBox = new QSpinBox(this);
    pointsSpinBox->setRange(3, 10000);
    pointsSpinBox->setValue(100);

    generateButton = new QPushButton("Generate Delaunay", this);

    controlsLayout->addWidget(label);
    controlsLayout->addWidget(pointsSpinBox);
    controlsLayout->addWidget(generateButton);
    controlsLayout->addStretch();

    canvas = new Canvas(this);

    mainLayout->addLayout(controlsLayout);
    mainLayout->addWidget(canvas, 1);

    connect(generateButton, &QPushButton::clicked, this, &MainWindow::onGenerateClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onGenerateClicked() {
    int num_points = pointsSpinBox->value();
    canvas->startAnimation(num_points);
}
