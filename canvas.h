#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QTimer>
#include <vector>
#include "dcel.h"

class Canvas : public QWidget {
    Q_OBJECT
public:
    explicit Canvas(QWidget *parent = nullptr);
    void startAnimation(int num_points);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void addNextPoint();

private:
    DCEL dcel;
    std::vector<Vertex> points_to_add;
    int current_point_index = 0;
    QTimer *timer;
};

#endif // CANVAS_H

