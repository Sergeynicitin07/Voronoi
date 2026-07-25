#ifndef UNTITLED41_DCEL_H
#define UNTITLED41_DCEL_H
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <random>


struct Vertex {
    double x, y;
    int incident_edge = -1;
};

struct HalfEdge {
    int origin = -1;
    int twin = -1;
    int next = -1;
    int prev = -1;
    int face = -1;
};


struct Face {
    int inner_comp = -1;
};



class DCEL {
public:
    std::vector<Vertex> vertices;
    std::vector<HalfEdge> edges;
    std::vector<Face> faces;

    int addVertex(double x, double y);
    void bewilder();
    std::vector<int> stun(int face_idx, double x, double y);
    void change_edge(int edge_ab);
};



class Delaunay {
public:
    DCEL dcel;
    bool is_finalized = false;

    void turn_into(double x, double y);

    void finalize();

private:
    int locate(const Vertex& p);
    void manage(int p0, int holy_edge);

    struct History_Face {
        bool isdead = false;
        std::vector<int> children;
        int v[3] = {-1, -1, -1};
        int face0 = -1;
    };

    std::vector<History_Face> history;
};

std::vector<Vertex> generateRandomDOTS(int n, int width, int height);
Vertex we_should_find_centre(const Vertex& a, const Vertex& b, const Vertex& c);
double det3x3(double m00, double m01, double m02,
              double m10, double m11, double m12,
              double m20, double m21, double m22);
double pseudoscalar(const Vertex& a, const Vertex& b);
int orientation(const Vertex& a, const Vertex& b, const Vertex& c);
bool convex(const Vertex& a, const Vertex& b, const Vertex& c);
bool point_in_circle(const Vertex& a, const Vertex& b, const Vertex& c, const Vertex& p);

#endif //UNTITLED41_DCEL_H

