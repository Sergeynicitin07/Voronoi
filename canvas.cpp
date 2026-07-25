#include "canvas.h"
#include <QPen>

Canvas::Canvas(QWidget *parent) : QWidget(parent) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Canvas::addNextPoint);
}

void Canvas::startTriangulation(int num_points) {

    points_to_add = generateRandomDOTS(num_points, width(), height());
    current_point_index = 0;
    is_finalized = false;


    triangulation = Delaunay();

    timer->start(100);

}

void Canvas::addNextPoint() {
    if (current_point_index < points_to_add.size()) {
        Vertex p = points_to_add[current_point_index];
        triangulation.turn_into(p.x, p.y);
        current_point_index++;
        update();
    } else {

        if (!is_finalized) {
            triangulation.finalize();
            is_finalized = true;
            update();
        }
        timer->stop();
    }
}

void Canvas::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    DCEL& d = triangulation.dcel;


    painter.setPen(QPen(Qt::lightGray, 1));
    for (size_t i = 0; i < d.edges.size(); ++i) {
        int next_e = d.edges[i].next;
        if (next_e != -1) {
            Vertex p1 = d.vertices[d.edges[i].origin];
            Vertex p2 = d.vertices[d.edges[next_e].origin];
            painter.drawLine(QPointF(p1.x, p1.y), QPointF(p2.x, p2.y));
        }
    }


    if (is_finalized) {
        painter.setPen(QPen(Qt::red, 2));


        std::vector<QPointF> voronoi_vertices(d.faces.size());
        for (size_t i = 0; i < d.faces.size(); ++i) {
            int e0 = d.faces[i].inner_comp;
            if (e0 == -1) continue;
            int e1 = d.edges[e0].next;
            int e2 = d.edges[e1].next;


            
            Vertex a = d.vertices[d.edges[e0].origin];
            Vertex b = d.vertices[d.edges[e1].origin];
            Vertex c = d.vertices[d.edges[e2].origin];

            

            Vertex center = we_should_find_centre(a, b, c);
            voronoi_vertices[i] = QPointF(center.x, center.y);
        }


        for (size_t i = 0; i < d.edges.size(); ++i) {
            int current_face = d.edges[i].face;
            int twin_edge = d.edges[i].twin;

            if (current_face == -1) continue;

            if (twin_edge != -1) {
                int twin_face = d.edges[twin_edge].face;

                if (twin_face != -1 && i < twin_edge) {
                    painter.drawLine(voronoi_vertices[current_face], voronoi_vertices[twin_face]);
                }
            } else {

                int next_e = d.edges[i].next;
                Vertex A = d.vertices[d.edges[i].origin];
                Vertex B = d.vertices[d.edges[next_e].origin];




                

                double dx = B.x - A.x;
                double dy = B.y - A.y;


                double nx = dy;
                double ny = -dx;


                double ray_length = 2000.0;
                QPointF start = voronoi_vertices[current_face];
                QPointF end(start.x() + nx * ray_length, start.y() + ny * ray_length);

                painter.drawLine(start, end);
            }
        }
    }









    
    painter.setPen(QPen(Qt::black, 4));
    for (size_t i = 0; i < d.vertices.size(); ++i) {
        painter.drawPoint(QPointF(d.vertices[i].x, d.vertices[i].y));
    }
}







