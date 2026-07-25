#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include "dcel.h"

class Canvas : public QWidget {
    Q_OBJECT

public:
    explicit Canvas(QWidget *parent = nullptr);
    void startTriangulation(int num_points);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void addNextPoint();

private:
    Delaunay triangulation;
    std::vector<Vertex> points_to_add;
    int current_point_index = 0;
    QTimer *timer;
    bool is_finalized = false;
};

#endif // CANVAS_H
