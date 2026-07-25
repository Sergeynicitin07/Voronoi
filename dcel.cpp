#include "dcel.h"



double pseudoscalar(const Vertex& a, const Vertex& b) {
    return (a.x * b.y) - (a.y * b.x);
}

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
// точки на плоскости a, b, c
// < 0 => поворот направо
// > 0 => поворот налево
// = 0 => прямо


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
    int dot0 = addVertex(-1e4, -1e4);
    int dot1 = addVertex(1e4, -1e4);
    int dot2 = addVertex(0, 1e4);


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




void Delaunay::finalize() {
    if (is_finalized) return;

    DCEL clean;


    std::vector<int> old_to_new_v(dcel.vertices.size(), -1);
    for (size_t i = 3; i < dcel.vertices.size(); ++i) {
        old_to_new_v[i] = clean.addVertex(dcel.vertices[i].x, dcel.vertices[i].y);
    }


    std::vector<bool> good_face(dcel.faces.size(), true);
    for (size_t i = 0; i < dcel.faces.size(); ++i) {
        int e0 = dcel.faces[i].inner_comp;
        if (e0 == -1) { good_face[i] = false; continue; }

        int e1 = dcel.edges[e0].next;
        int e2 = dcel.edges[e1].next;


        if (dcel.edges[e0].origin < 3 ||
            dcel.edges[e1].origin < 3 ||
            dcel.edges[e2].origin < 3) {
            good_face[i] = false;
        }
    }


    std::vector<int> old_to_new_f(dcel.faces.size(), -1);
    for (size_t i = 0; i < dcel.faces.size(); ++i) {
        if (good_face[i]) {
            old_to_new_f[i] = clean.faces.size();
            clean.faces.push_back({-1});
        }
    }


    std::vector<int> old_to_new_e(dcel.edges.size(), -1);
    for (size_t i = 0; i < dcel.edges.size(); ++i) {
        int f = dcel.edges[i].face;
        if (f != -1 && good_face[f]) {
            old_to_new_e[i] = clean.edges.size();
            clean.edges.push_back(HalfEdge());
        }
    }


    for (size_t i = 0; i < dcel.edges.size(); ++i) {
        int new_e = old_to_new_e[i];
        if (new_e != -1) {
            clean.edges[new_e].origin = old_to_new_v[dcel.edges[i].origin];
            clean.edges[new_e].face   = old_to_new_f[dcel.edges[i].face];
            clean.edges[new_e].next   = old_to_new_e[dcel.edges[i].next];
            clean.edges[new_e].prev   = old_to_new_e[dcel.edges[i].prev];

            int twin = dcel.edges[i].twin;

            if (twin != -1 && old_to_new_e[twin] != -1) {
                clean.edges[new_e].twin = old_to_new_e[twin];
            } else {
                clean.edges[new_e].twin = -1;
            }
        }
    }


    for (size_t i = 0; i < dcel.faces.size(); ++i) {
        if (good_face[i]) {
            clean.faces[old_to_new_f[i]].inner_comp = old_to_new_e[dcel.faces[i].inner_comp];
        }
    }


    for (size_t e = 0; e < clean.edges.size(); ++e) {
        int v = clean.edges[e].origin;
        if (clean.vertices[v].incident_edge == -1) {
            clean.vertices[v].incident_edge = e;
        }
    }


    this->dcel = clean;
    this->is_finalized = true;
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

    // Переиспользуем старую грань для первого треугольника
    int g0 = face_idx;
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
        // Запоминаем текущие грани dcel, которые будут аннигилированы
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
