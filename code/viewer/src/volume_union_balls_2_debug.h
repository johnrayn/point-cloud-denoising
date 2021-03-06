#ifndef _VOLUME_UNION_BALLS_2_DEBUG_H_
#define _VOLUME_UNION_BALLS_2_DEBUG_H_

// Test if p and q are in a counter clockwise order wrt ref
template <typename Point>
bool in_counter_clockwise_debug (Point const& p, Point const& q,
                                 Point const& ref) {
    double thetap = std::atan2(p.y() - ref.y(), p.x() - ref.x()),
           thetaq = std::atan2(q.y() - ref.y(), q.x() - ref.x());

    return thetap < thetaq;
}

// Intersection between a segment and a sphere
template <class Point, class Segment, class OutputIterator>
bool segment_sphere_intersect_debug (Point o, double r,
                                     Segment seg,
                                     OutputIterator out) {
    typedef typename CGAL::Kernel_traits<Point>::Kernel Kernel;
    typedef typename Kernel::Vector_2 Vector;
    typedef typename Kernel::FT FT;

    Point s = seg.source();
    Vector v = seg.target() - seg.source(),
           e = s - o;

    FT a = v.squared_length(),
       b = 2 * v * e,
       c = e.squared_length() - r * r;

    FT delta = b * b - 4 * a * c;

    if (delta < 0) {
        return false;
    }

    bool result = false;
    const double pm[] = {+1.0, -1.0};
    for (size_t i = 0; i < 2; ++i) {
        FT ti = (-b + pm[i] * CGAL::sqrt(delta)) / (2 * a);
        Point pi = s + ti * v;
        if (ti >= 0 && ti <= 1) {
            *out++ = pi;
            result = true;
        }
    }

    return result;
}

// Positive floating modulus
double fmodpos_debug (double x, double N) {
    return std::fmod(std::fmod(x, N) + N, N);
}

// Area of an angular sector defined by the vectors op and oq
template <typename Vector>
double angular_sector_area_debug (Vector op, Vector oq,
                                  double radius) {
    if (op == Vector(0, 0) || oq == Vector(0, 0)) {
        return 0;
    }

    double theta1 = fmodpos_debug(std::atan2(op.y(), op.x()), 2 * M_PI),
           theta2 = fmodpos_debug(std::atan2(oq.y(), oq.x()), 2 * M_PI);
    double angle = fmodpos_debug(theta2 - theta1, 2 * M_PI);

    return radius * radius * angle / 2;
}

// Version of volume_ball_voronoi_cell_2 outputing
// a list of segments corresponding to the decomposition
template <typename Kernel, typename DT, typename Vertex_handle>
typename Kernel::FT volume_ball_voronoi_cell_2_debug (DT const& dt,
                                                      Vertex_handle const& v,
                                                      double radius,
                                                      std::vector<typename Kernel::Segment_2>& out) {
    typedef typename Kernel::Point_2 Point;
    typedef typename Kernel::Line_2 Line;
    typedef typename Kernel::FT FT;
    typedef typename Kernel::Segment_2 Segment;

    Point P = v->point();

    FT vol = 0;

    // Compute the boundary of the Voronoi cell of P
    typename DT::Face_circulator fc = dt.incident_faces(v), done(fc);
    std::vector<Point> adjacent_voronoi_vertices;
    do {
        adjacent_voronoi_vertices.push_back(dt.dual(fc));
    } while (++fc != done);

    const size_t N = adjacent_voronoi_vertices.size();

    // Test if the first Voronoi vertex is inside
    Point p0 = adjacent_voronoi_vertices[0];
    bool isInside = (p0 - P).squared_length() <= radius * radius;
    bool allOutside = !isInside;

    std::vector<Point> boundary;
    std::map<Point, Segment> edge_map;
    std::map<Point, bool> interior_map;

    // For each Voronoi vertex
    for (size_t i = 0; i < N; ++i) {
        Point p = adjacent_voronoi_vertices[i],
              next = adjacent_voronoi_vertices[(i + 1) % N];
        Segment edge(p, next);

        if (isInside) {
            boundary.push_back(p);
        }
        interior_map[p] = isInside;

        // Intersection points between the Voronoi edge [p, next] and the ball
        std::vector<Point> inter;
        segment_sphere_intersect_debug(P, radius, edge, std::back_inserter(inter));

        // Remember the edge corresponding to an intersection point
        if (inter.size() != 0) {
            for (size_t i = 0; i < inter.size(); ++i) {
                edge_map[inter[i]] = edge;
            }
            if (inter.size() == 2) {
                // Keep a counter clockwise order
                if (in_counter_clockwise_debug(inter[0], inter[1], P)) {
                    boundary.insert(boundary.end(), inter.begin(), inter.end());
                } else {
                    boundary.push_back(inter[1]);
                    boundary.push_back(inter[0]);
                }
            } else {
                boundary.insert(boundary.end(), inter.begin(), inter.end());
            }
            allOutside = false;
        }

        // Update isInside boolean
        if (inter.size() == 1) {
            isInside = !isInside;
        }
    }

    out.clear();

    // The boundary of the Voronoi cell is entirely outside of the ball
    if (allOutside) {
        vol = M_PI * radius * radius;

        return vol;
    }

    // Special case: 2 points
    if (boundary.size() == 2) {
        Point p = boundary[0], pp = boundary[1];
        Line l(p, pp);

        out.push_back(Segment(P, p));
        out.push_back(Segment(P, pp));
        out.push_back(Segment(p, pp));

        if (l.oriented_side(P) == CGAL::ON_POSITIVE_SIDE) {
            vol += CGAL::area(P, p, pp);
            vol += angular_sector_area_debug(pp - P, p - P, radius);
        } else {
            vol += CGAL::area(P, pp, p);
            vol += angular_sector_area_debug(p - P, pp - P, radius);
        }

        return vol;
    }

    for (size_t i = 0; i < boundary.size(); ++i) {
        Point p = boundary[i],
              pp = boundary[(i + 1) % boundary.size()];

        if (interior_map[p] && interior_map[pp]) {
            // 2 interior points: triangle
            vol += CGAL::area(P, p, pp);
            out.push_back(Segment(P, p));
            out.push_back(Segment(P, pp));
            out.push_back(Segment(p, pp));
        } else if (interior_map[p] && !interior_map[pp]) {
            // 1 interior point: triangle
            vol += CGAL::area(P, p, pp);
            out.push_back(Segment(P, p));
            out.push_back(Segment(P, pp));
            out.push_back(Segment(p, pp));
        } else if (!interior_map[p] && interior_map[pp]) {
            // 1 interior point: triangle
            vol += CGAL::area(P, p, pp);
            out.push_back(Segment(P, p));
            out.push_back(Segment(P, pp));
            out.push_back(Segment(p, pp));
        } else {
            // 0 interior points: 2 on the boundary
            // It depends on the corresponding edges
            Segment pedge = edge_map[p],
                    ppedge = edge_map[pp];

            if (pedge == ppedge) {
                // Same edge: triangle
                vol += CGAL::area(P, p, pp);
                out.push_back(Segment(P, p));
                out.push_back(Segment(P, pp));
                out.push_back(Segment(p, pp));
            } else {
                // Different edges: angular sector
                vol += angular_sector_area_debug(p - P, pp - P, radius);
                out.push_back(Segment(P, p));
                out.push_back(Segment(P, pp));
            }
        }
    }


    return vol;
}

// Version of volume_union_balls_2 outputing
// a list of segments corresponding to the decomposition
template <typename FT, typename Segment, typename InputIterator>
FT volume_union_balls_2_debug (InputIterator begin,
                               InputIterator beyond,
                               double radius,
                               std::vector<Segment>& out) {
    typedef typename std::iterator_traits<InputIterator>::value_type Point;
    typedef typename CGAL::Kernel_traits<Point>::Kernel Kernel;
    typedef typename CGAL::Delaunay_triangulation_2<Kernel> DT;

    // Bounding box
    CGAL::Bbox_2 b = CGAL::bbox_2(begin, beyond);
    Point bl(b.xmin() - 2 * radius, b.ymin() - 2 * radius),
          br(b.xmax() + 2 * radius, b.ymin() - 2 * radius),
          tl(b.xmin() - 2 * radius, b.ymax() + 2 * radius),
          tr(b.xmax() + 2 * radius, b.ymax() + 2 * radius);

    // Delaunay triangulation
    DT dt(begin, beyond);
    dt.insert(bl); dt.insert(br);
    dt.insert(tl); dt.insert(tr);

    FT vol = 0;

    out.clear();
    // Decomposition of the intersection of a Voronoi cell and a ball
    // It is made of triangles and agular sectors
    for (typename DT::Finite_vertices_iterator vit = dt.finite_vertices_begin();
         vit != dt.finite_vertices_end();
         ++vit) {
        Point P = vit->point();

        if (P == bl || P == br || P == tl || P == tr) {
            continue;
        }

        std::vector<Segment> segments;
        vol += volume_ball_voronoi_cell_2_debug<Kernel>(dt, vit, radius, segments);
        out.insert(out.end(), segments.begin(), segments.end());
    }

    return vol;
}

#endif

