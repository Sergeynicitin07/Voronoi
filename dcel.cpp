#include "dcel.h"



double pseudoscalar(const Vertex& a, const Vertex& b) {
    return (a.x * b.y) - (a.y * b.x);
}


// точки на плоскости a, b, c
// < 0 => поворот направо
// > 0 => поворот налево
// = 0 => прямо


const double EPS = 1e-9;



// to find delanay centre
Vertex we_should_find_centre(const Vertex& a, const Vertex& b, const Vertex& c) {
    double D = 2 * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));


    if (std::abs(D) < EPS) {
        return {(a.x + b.x + c.x) / 3.0, (a.y + b.y + c.y) / 3.0, -1};
    }

    double ux = ((a.x * a.x + a.y * a.y) * (b.y - c.y) +
                 (b.x * b.x + b.y * b.y) * (c.y - a.y) +
                 (c.x * c.x + c.y * c.y) * (a.y - b.y)) / D;

    double uy = ((a.x * a.x + a.y * a.y) * (c.x - b.x) +
                 (b.x * b.x + b.y * b.y) * (a.x - c.x) +
                 (c.x * c.x + c.y * c.y) * (b.x - a.x)) / D;

    return {ux, uy, -1};
}




int orientation (const Vertex& a, const Vertex& b, const Vertex& c) {
    // при движении a -> b -> c
    Vertex ab = {b.x - a.x, b.y - a.y};
    Vertex ac = {c.x - a.x, c.y - a.y};

    // если больше нуля, то движение против часовой стрелки
    if (pseudoscalar(ab, ac) > 0) return 1;
    if (pseudoscalar(ab, ac) < 0) return -1;
    return 0;

}


bool convex (const Vertex& a, const Vertex& b, const Vertex& c) {
    return orientation(a, b, c) >= 0;
}



double det3x3(double m00, double m01, double m02,
              double m10, double m11, double m12,
              double m20, double m21, double m22) {
    return m00 * (m11 * m22 - m12 * m21)
           - m01 * (m10 * m22 - m12 * m20)
           + m02 * (m10 * m21 - m11 * m20);
}


bool point_in_circle(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& p) {
    double sa = a.x * a.x + a.y * a.y;
    double sb = b.x * b.x + b.y * b.y;
    double sc = c.x * c.x + c.y * c.y;
    double sp = p.x * p.x + p.y * p.y;

    double m14 = det3x3(sb, b.x, b.y,
                        sc, c.x, c.y,
                        sp, p.x, p.y);

    double m24 = det3x3(sa, a.x, a.y,
                        sc, c.x, c.y,
                        sp, p.x, p.y);

    double m34 = det3x3(sa, a.x, a.y,
                        sb, b.x, b.y,
                        sp, p.x, p.y);

    double m44 = det3x3(sa, a.x, a.y,
                        sb, b.x, b.y,
                        sc, c.x, c.y);

    double det = -m14 + m24 - m34 + m44;

    return det > 0;
}



int DCEL::addVertex(double x, double y) {
    vertices.push_back({x, y, -1});
    return vertices.size() - 1;
}

std::vector<Vertex> generateRandomDOTS(int n, int width, int height) {
    if (n < 3) return {};
    std::vector<Vertex> dots(n);
    for (int i = 0; i < n; i ++) {
        dots[i].y = 50 + rand() % (height - 100);
        dots[i].x = 50 + rand() % (width - 100);
    }
    return dots;
}

void DCEL::bewilder () {
    int dot0 = addVertex(-1e7, -1e7);
    int dot1 = addVertex(1e7, -1e7);
    int dot2 = addVertex(0, 1e7);

    // это индекс для первой грани
    int f0 = 0;
    faces.push_back({f0});

    // то есть мы положили набор в полуреберный вектор
    edges.push_back({dot0, 3,1,2, f0});
    edges.push_back({dot1, 4,2,0, f0});
    edges.push_back({dot2, 5,0,1, f0});
    vertices[dot0].incident_edge = 0;
    vertices[dot1].incident_edge = 1;
    vertices[dot2].incident_edge = 2;

    int f0_ = -1; // полуграни направленные в сторону вечной пустоты.
    // Отметим, как -1. они идут по часовой стрелке в большом треугольнике.
    edges.push_back({dot1, 0, 5,4, f0_});
    edges.push_back({dot2, 1,3,5, f0_});
    edges.push_back({dot0, 2,4,3, f0_});


}



int Delaunay::locate(const Vertex& p) {
    if (history.empty()) return -1;
    int current = 0;

    while (history[current].isdead) {
        bool found_child = false;

        for (int child_idx : history[current].children) {
            Vertex a = dcel.vertices[history[child_idx].v[0]];
            Vertex b = dcel.vertices[history[child_idx].v[1]];
            Vertex c = dcel.vertices[history[child_idx].v[2]];

            if (convex(a, b, p) && convex(c, a, p) && convex(b, c, p)) {
                current = child_idx;
                found_child = true;
                break;
            }
        }

        if (!found_child) {
            if (!history[current].children.empty()) {
                current = history[current].children[0];
            } else {
                break;
            }
        }
    }
    return current;
}



std::vector<int> DCEL::stun (int face_idx, double x, double y) {
    // добавляем в список вершин
    int p = addVertex(x, y);
    // полуребра старого трегольника, куда упала точка
    int edge0 = faces[face_idx].inner_comp; // Ребро AB
    int edge1 = edges[edge0].next; // Ребро BC
    int edge2 = edges[edge1].next; // Ребро CA

    int a = edges[edge0].origin; // Вершина A
    int b = edges[edge1].origin; // Вершина B
    int c = edges[edge2].origin; // Вершина C
    // создаем новые грани. в качестве индексов возьмем длину массива на данный момент
    // При создании новых граней g0, g1, g2 сразу фиксируем их вершины
    int g0 = faces.size();
    faces.push_back({edge0});

    int g1 = faces.size();
    faces.push_back({edge1});

    int g2 = faces.size();
    faces.push_back({edge2});


    int edge_pa = edges.size();
    int edge_ap = edges.size() + 1;
    int edge_pb = edges.size() + 2;
    int edge_bp = edges.size() + 3;
    int edge_pc = edges.size() + 4;
    int edge_cp = edges.size() + 5;
    edges.push_back({p, edge_ap, edge0, edge_bp, g0});
    edges.push_back({a, edge_pa, edge_pc, edge2, g2});
    edges.push_back({p, edge_bp, edge1,  edge_cp, g1});
    edges.push_back({b, edge_pb, edge_pa, edge0, g0});
    edges.push_back({p, edge_cp, edge2,  edge_ap, g2});
    edges.push_back({c, edge_pc, edge_pb, edge1, g1});




    edges[edge0].origin = a;
    edges[edge0].next = edge_bp;
    edges[edge0].prev = edge_pa;
    edges[edge0].face = g0;



    edges[edge1].origin = b;
    edges[edge1].next = edge_cp;
    edges[edge1].prev = edge_pb;
    edges[edge1].face = g1;




    edges[edge2].origin = c;
    edges[edge2].next = edge_ap;
    edges[edge2].prev = edge_pc;
    edges[edge2].face = g2;


    // точка p должна быть привязана к какому-то полуребру
    vertices[p].incident_edge = edge_pa;

    return {edge0, edge1, edge2};
}


void DCEL::change_edge (int edge_ab){

    int edge_ba = edges[edge_ab].twin;
    int edge_bc = edges[edge_ab].next;
    int edge_ca = edges[edge_ab].prev;
    int edge_ad = edges[edge_ba].next;
    int edge_db = edges[edge_ba].prev;

    int a = edges[edge_ab].origin;
    int b = edges[edge_ba].origin;
    int c = edges[edge_ca].origin;
    int d = edges[edge_db].origin;


    // выпишим грани


    int f_newA = faces.size();
    faces.push_back({edge_ab}); // Треугольник P - D - B

    int f_newB = faces.size();
    faces.push_back({edge_ba}); // Треугольник D - P - A



    // теперь точка A стала C, а точка B - D
    edges[edge_ab].origin = c;
    edges[edge_ba].origin = d;


    edges[edge_ab].face = f_newA;
    edges[edge_ab].next = edge_db;
    edges[edge_ab].prev = edge_bc;


    edges[edge_ba].face = f_newB;
    edges[edge_ba].next = edge_ca;
    edges[edge_ba].prev = edge_ad;

    edges[edge_db].next = edge_bc;
    edges[edge_db].prev = edge_ab;
    edges[edge_db].face = f_newA;


    edges[edge_bc].face = f_newA;
    edges[edge_bc].next = edge_ab;
    edges[edge_bc].prev = edge_db;


    edges[edge_ba].face = f_newB;
    edges[edge_ba].next = edge_ca;
    edges[edge_ba].prev = edge_ad;


    edges[edge_ca].face = f_newB;
    edges[edge_ca].next = edge_ad;
    edges[edge_ca].prev = edge_ba;


    edges[edge_ad].face = f_newB;
    edges[edge_ad].prev = edge_ca;
    edges[edge_ad].next = edge_ba;


    faces[f_newA].inner_comp = edge_ab;
    faces[f_newB].inner_comp = edge_ba;

    vertices[a].incident_edge = edge_ad;
    vertices[b].incident_edge = edge_bc;
    vertices[c].incident_edge = edge_ab;
    vertices[d].incident_edge = edge_ba;






}



void Delaunay::manage (int p0, int holy_edge) {
    int twinki_pinki = dcel.edges[holy_edge].twin;

    if (twinki_pinki == -1 || dcel.edges[twinki_pinki].face == -1) return;

    int b0 = dcel.edges[twinki_pinki].origin;
    int a0 = dcel.edges[dcel.edges[twinki_pinki].next].origin;
    int d0 = dcel.edges[dcel.edges[twinki_pinki].prev].origin;

    Vertex a = dcel.vertices[a0];
    Vertex b = dcel.vertices[b0];
    Vertex p = dcel.vertices[p0];
    Vertex d = dcel.vertices[d0];

    if (point_in_circle(b, a, d, p)) {
        // Запоминаем текущие грани dcel, которые будут анигилированы
        int f_a = dcel.edges[holy_edge].face;
        int f_b = dcel.edges[twinki_pinki].face;

        int hist_a = -1, hist_b = -1;
        for (int i = history.size() - 1; i >= 0; --i) {
            if (!history[i].isdead) {
                if (history[i].face0 == f_a) hist_a = i;
                if (history[i].face0 == f_b) hist_b = i;
            }
        }
        if (hist_a != -1) history[hist_a].isdead = true;
        if (hist_b != -1) history[hist_b].isdead = true;

        int pr = dcel.edges[twinki_pinki].prev;
        int nx = dcel.edges[twinki_pinki].next;

        dcel.change_edge(holy_edge);

        int f_newA = dcel.edges[holy_edge].face;
        int f_newB = dcel.edges[dcel.edges[holy_edge].twin].face;
        int c0 = dcel.edges[holy_edge].origin;

        int new_hA = history.size(); history.push_back({false, {}, {c0, d0, b0}, f_newA});
        int new_hB = history.size(); history.push_back({false, {}, {d0, c0, a0}, f_newB});

        if (hist_a != -1) history[hist_a].children = {new_hA, new_hB};
        if (hist_b != -1) history[hist_b].children = {new_hA, new_hB};

        manage(p0, pr);
        manage(p0, nx);
    }



   
}



void Delaunay::turn_into (double x, double y) {

    // выполняем инициализацию, если только начали
    if (history.empty()) {
        dcel.bewilder();

        int f0 = 0;
        history.push_back({false, {}, {0, 1, 2}, f0});
    }

    Vertex dot = {x, y, -1};
    int face_x = locate(dot);
    if (face_x == -1) return;


    history[face_x].isdead = true;
    int dcel_face = history[face_x].face0;




    std::vector<int> new_edges = dcel.stun(dcel_face, x, y);

    int p_ = dcel.vertices.size() - 1;
    int e0 = new_edges[0], e1 = new_edges[1], e2 = new_edges[2];

    int a = dcel.edges[e0].origin;
    int b = dcel.edges[e1].origin;
    int c = dcel.edges[e2].origin;

    int g0 = dcel.edges[e0].face;
    int g1 = dcel.edges[e1].face;
    int g2 = dcel.edges[e2].face;

    int h0 = history.size(); history.push_back({false, {}, {a, b, p_}, g0});
    int h1 = history.size(); history.push_back({false, {}, {b, c, p_}, g1});
    int h2 = history.size(); history.push_back({false, {}, {c, a, p_}, g2});

    history[face_x].children = {h0, h1, h2};

    manage(p_, e0);
    manage(p_, e1);
    manage(p_, e2);
}

