#include "canvas.h"
#include <QPainter>
#include <QPen>
#include <random>
#include <algorithm>

Canvas::Canvas(QWidget *parent) : QWidget(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Canvas::addNextPoint);
}

void Canvas::startAnimation(int num_points) {
    triangulation = Delaunay();

    points_to_add = generateRandomDOTS(num_points, width(), height());

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(points_to_add.begin(), points_to_add.end(), g);

    current_point_index = 0;

    timer->start(50);
}

void Canvas::addNextPoint() {
    if (current_point_index < points_to_add.size()) {
        Vertex p = points_to_add[current_point_index];
        triangulation.turn_into(p.x, p.y);

        current_point_index++;
        update();
    } else {
        timer->stop();
    }
}

void Canvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::white);

    std::vector<bool> active_faces(triangulation.dcel.faces.size(), false);

    painter.setPen(QPen(QColor(150, 150, 255, 100), 1, Qt::DashLine));

    for (const auto& h_face : triangulation.history) {
        if (h_face.isdead) continue;
        
        if (h_face.face0 != -1) {
            active_faces[h_face.face0] = true;
        }

        int iv0 = h_face.v[0];
        int iv1 = h_face.v[1];
        int iv2 = h_face.v[2];

        if (iv0 < 3 || iv1 < 3 || iv2 < 3) {
            continue;
        }

        Vertex v0 = triangulation.dcel.vertices[iv0];
        Vertex v1 = triangulation.dcel.vertices[iv1];
        Vertex v2 = triangulation.dcel.vertices[iv2];

        painter.drawLine(QPointF(v0.x, v0.y), QPointF(v1.x, v1.y));
        painter.drawLine(QPointF(v1.x, v1.y), QPointF(v2.x, v2.y));
        painter.drawLine(QPointF(v2.x, v2.y), QPointF(v0.x, v0.y));
    }


    painter.setPen(QPen(QColor(0, 180, 0), 2));

    for (size_t i = 0; i < triangulation.dcel.edges.size(); ++i) {
        int twin_idx = triangulation.dcel.edges[i].twin;

        if (twin_idx == -1 || i > twin_idx) continue;

        int face1_idx = triangulation.dcel.edges[i].face;
        int face2_idx = triangulation.dcel.edges[twin_idx].face;

        if (face1_idx == -1 || face2_idx == -1) continue;
        
        if (!active_faces[face1_idx] || !active_faces[face2_idx]) continue;

        int e1_next = triangulation.dcel.edges[i].next;
        int e1_prev = triangulation.dcel.edges[i].prev;
        
        Vertex a1 = triangulation.dcel.vertices[triangulation.dcel.edges[i].origin];
        Vertex b1 = triangulation.dcel.vertices[triangulation.dcel.edges[e1_next].origin];
        Vertex c1 = triangulation.dcel.vertices[triangulation.dcel.edges[e1_prev].origin];

        int e2_next = triangulation.dcel.edges[twin_idx].next;
        int e2_prev = triangulation.dcel.edges[twin_idx].prev;

        Vertex a2 = triangulation.dcel.vertices[triangulation.dcel.edges[twin_idx].origin];
        Vertex b2 = triangulation.dcel.vertices[triangulation.dcel.edges[e2_next].origin];
        Vertex c2 = triangulation.dcel.vertices[triangulation.dcel.edges[e2_prev].origin];

        Vertex circum1 = we_should_find_centre(a1, b1, c1);
        Vertex circum2 = we_should_find_centre(a2, b2, c2);

        painter.drawLine(QPointF(circum1.x, circum1.y), QPointF(circum2.x, circum2.y));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::red);
    for (int i = 0; i < current_point_index; ++i) {
        painter.drawEllipse(QPointF(points_to_add[i].x, points_to_add[i].y), 3, 3);
    }

    if (current_point_index > 0 && current_point_index <= points_to_add.size()) {
        painter.setBrush(Qt::green);
        painter.drawEllipse(QPointF(points_to_add[current_point_index-1].x, points_to_add[current_point_index-1].y), 5, 5);
    }
}
