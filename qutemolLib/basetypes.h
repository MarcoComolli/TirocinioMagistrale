#ifndef BASETYPES_H
#define BASETYPES_H

#include <vcg/space/point3.h>
#include <vcg/space/point4.h>
#include <vcg/math/matrix44.h>

namespace qmol {

/** scalar type */
typedef float Scalar;

/** types for: positions, colors, vectors...
    same thing (actually, vectors of 3 floats)
    but different type names, just for clarity. */

typedef vcg::Point3<Scalar> Pos; // Positions
typedef vcg::Point3<Scalar> Col; // Colors
typedef vcg::Point3<Scalar> Vec; // Vectors

inline Scalar dot(const Vec &a, const Vec &b){return a*b;}
inline Vec cross(const Vec &a, const Vec &b){return a^b;}

inline float sqrt(float p){return ::sqrtf(p);}
inline double sqrt(double p){return ::sqrt(p);}

inline float pow(float a, float b){return powf(a,b);}
inline double pow(double a, double b){return ::pow(a,b);}

inline float abs(float p){return ::fabsf(p);}
inline double abs(double p){return ::fabs(p);}


inline Scalar sqrDistance(Pos p1, Pos p2){
    return dot(p1-p2, p1-p2);
}

inline Scalar distance(Pos p1, Pos p2){
    return sqrt(sqrDistance(p1, p2));
}

/** a plane (passing thorugh p, with normal n), in the form ( n.x, n.y, n.z, -n*p ): */
typedef vcg::Point4<Scalar> Plane;

typedef vcg::Point4<Scalar> Vec4; // A vector of 4 elements

typedef vcg::Matrix44<Scalar> Matrix;

Matrix transposedInverse(const Matrix &m);
Matrix fromQuat(const Vec4 &v);
Vec4 quatMult(const Vec4 &a, const Vec4 &b);
Vec4 quatAxisAngle(const Vec &a, Scalar b);

typedef unsigned int uint;

/** qmol::FeedbackFunctionT:
 *  qmol functins which might take too long, take an optional function parameter
 *  of this type, which expects a "percentage-done" int value and returns true iff the action is cancelled
 */

typedef bool (*FeedbackFunctionT)(int);

}

#define DISABLE_COPY(CLASS_NAME) \
    private: \
        CLASS_NAME (const CLASS_NAME & c); \
        CLASS_NAME & operator = (const CLASS_NAME & c);


#endif // BASETYPES_H
