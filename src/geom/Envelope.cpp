/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: geom/Envelope.java rev 1.46 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

#include <algorithm>
#include <sstream>
#include <cmath>

#ifndef GEOS_INLINE
# include <geos/geom/Envelope.inl>
#endif

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif

namespace geos {
namespace geom { // geos::geom

/*public*/
bool
Envelope::intersects(const Coordinate& p1, const Coordinate& p2,
                     const Coordinate& q)
{
    //OptimizeIt shows that Math#min and Math#max here are a bottleneck.
    //Replace with direct comparisons. [Jon Aquino]
    if(((q.x >= (p1.x < p2.x ? p1.x : p2.x)) && (q.x <= (p1.x > p2.x ? p1.x : p2.x))) &&
            ((q.y >= (p1.y < p2.y ? p1.y : p2.y)) && (q.y <= (p1.y > p2.y ? p1.y : p2.y)))) {
        return true;
    }
    return false;
}

/*public*/
bool
Envelope::intersects(const Coordinate& p1, const Coordinate& p2,
                     const Coordinate& q1, const Coordinate& q2)
{
    double minq = std::min(q1.x, q2.x);
    double maxq = std::max(q1.x, q2.x);
    double minp = std::min(p1.x, p2.x);
    double maxp = std::max(p1.x, p2.x);
    if(minp > maxq) {
        return false;
    }
    if(maxp < minq) {
        return false;
    }
    minq = std::min(q1.y, q2.y);
    maxq = std::max(q1.y, q2.y);
    minp = std::min(p1.y, p2.y);
    maxp = std::max(p1.y, p2.y);
    if(minp > maxq) {
        return false;
    }
    if(maxp < minq) {
        return false;
    }
    return true;
}

/*public*/
bool
Envelope::intersects(const Coordinate& a, const Coordinate& b) const
{

    double envminx = (a.x < b.x) ? a.x : b.x;
    if(envminx > maxx) {
        return false;
    }

    double envmaxx = (a.x > b.x) ? a.x : b.x;
    if(envmaxx < minx) {
        return false;
    }

    double envminy = (a.y < b.y) ? a.y : b.y;
    if(envminy > maxy) {
        return false;
    }

    double envmaxy = (a.y > b.y) ? a.y : b.y;
    if(envmaxy < miny) {
        return false;
    }

    return true;
}

/*public*/
Envelope::Envelope(const std::string& str)
{
    // The string should be in the format:
    // Env[7.2:2.3,7.1:8.2]

    // extract out the values between the [ and ] characters
    std::string::size_type index = str.find("[");
    std::string coordString = str.substr(index + 1, str.size() - 1 - 1);

    // now split apart the string on : and , characters
    std::vector<std::string> values = split(coordString, ":,");

    // create a new envelope
    init(strtod(values[0].c_str(), nullptr),
         strtod(values[1].c_str(), nullptr),
         strtod(values[2].c_str(), nullptr),
         strtod(values[3].c_str(), nullptr));
}

#if 0
/**
 *  Initialize an <code>Envelope</code> from an existing Envelope.
 *
 *@param  env  the Envelope to initialize from
 */
void
Envelope::init(Envelope env)
{
    init(env.minx, env.maxx, env.miny, env.maxy);
}
#endif // 0


/*public*/
bool
Envelope::covers(double x, double y) const
{
    if(isNull()) {
        return false;
    }
    return x >= minx &&
           x <= maxx &&
           y >= miny &&
           y <= maxy;
}


/*public*/
bool
Envelope::covers(const Envelope& other) const
{
    if(isNull() || other.isNull()) {
        return false;
    }

    return
        other.getMinX() >= minx &&
        other.getMaxX() <= maxx &&
        other.getMinY() >= miny &&
        other.getMaxY() <= maxy;
}

/*public*/
bool
Envelope::equals(const Envelope* other) const
{
    if(isNull()) {
        return other->isNull();
    }
    return  other->getMinX() == minx &&
            other->getMaxX() == maxx &&
            other->getMinY() == miny &&
            other->getMaxY() == maxy;
}

/* public */
std::ostream&
operator<< (std::ostream& os, const Envelope& o)
{
    os << "Env[" << o.minx << ":" << o.maxx << ","
       << o.miny << ":" << o.maxy << "]";
    return os;
}


/*public*/
std::string
Envelope::toString() const
{
    std::ostringstream s;
    s << *this;
    return s.str();
}

/*public*/
bool
operator==(const Envelope& a, const Envelope& b)
{
    if(a.isNull()) {
        return b.isNull();
    }
    if(b.isNull()) {
        return a.isNull();
    }
    return a.getMaxX() == b.getMaxX() &&
           a.getMaxY() == b.getMaxY() &&
           a.getMinX() == b.getMinX() &&
           a.getMinY() == b.getMinY();
}

/*public*/
size_t
Envelope::hashCode() const
{
    auto hash = std::hash<double>{};

    //Algorithm from Effective Java by Joshua Bloch [Jon Aquino]
    std::size_t result = 17;
    result = 37 * result + hash(minx);
    result = 37 * result + hash(maxx);
    result = 37 * result + hash(miny);
    result = 37 * result + hash(maxy);
    return result;
}

/*public static*/
std::vector<std::string>
Envelope::split(const std::string& str, const std::string& delimiters)
{
    std::vector<std::string> tokens;

    // Find first "non-delimiter".
    std::string::size_type lastPos = 0;
    std::string::size_type pos = str.find_first_of(delimiters, lastPos);

    while(std::string::npos != pos || std::string::npos != lastPos) {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));

        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);

        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens;
}

/*public*/
bool
Envelope::centre(Coordinate& p_centre) const
{
    if(isNull()) {
        return false;
    }
    p_centre.x = (getMinX() + getMaxX()) / 2.0;
    p_centre.y = (getMinY() + getMaxY()) / 2.0;
    return true;
}

/*public*/
bool
Envelope::intersection(const Envelope& env, Envelope& result) const
{
    if(isNull() || env.isNull() || ! intersects(env)) {
        return false;
    }

    double intMinX = minx > env.minx ? minx : env.minx;
    double intMinY = miny > env.miny ? miny : env.miny;
    double intMaxX = maxx < env.maxx ? maxx : env.maxx;
    double intMaxY = maxy < env.maxy ? maxy : env.maxy;
    result.init(intMinX, intMaxX, intMinY, intMaxY);
    return true;
}

/*public*/
void
Envelope::translate(double transX, double transY)
{
    if(isNull()) {
        return;
    }
    init(getMinX() + transX, getMaxX() + transX,
         getMinY() + transY, getMaxY() + transY);
}


/*public*/
void
Envelope::expandBy(double deltaX, double deltaY)
{
    if(isNull()) {
        return;
    }

    minx -= deltaX;
    maxx += deltaX;
    miny -= deltaY;
    maxy += deltaY;

    // check for envelope disappearing
    if(minx > maxx || miny > maxy) {
        setToNull();
    }
}



} // namespace geos::geom
} // namespace geos
