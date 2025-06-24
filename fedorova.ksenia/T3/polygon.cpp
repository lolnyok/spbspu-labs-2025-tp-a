#include "polygon.h"

bool fedorova::operator==(const Point& lhs, const Point& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool fedorova::operator==(const Polygon& lhs, const Polygon& rhs)
{
    return lhs.points == rhs.points;
}

std::istream& fedorova::operator>>(std::istream& in, Point& p)
{
    char discard;
    if (!(in >> discard >> p.x >> discard >> p.y >> discard) || discard != ')')
    {
        in.setstate(std::ios::failbit);
    }
    return in;
}

std::istream& fedorova::operator>>(std::istream& in, Polygon& poly)
{
    size_t vertexCount;
    if (!(in >> vertexCount) || vertexCount < 3)
    {
        in.setstate(std::ios::failbit);
        return in;
    }

    poly.points.resize(vertexCount);
    for (auto& point : poly.points)
    {
        if (!(in >> point))
        {
            in.setstate(std::ios::failbit);
            break;
        }
    }

    return in;
}
