#pragma once
#include "vec3.h"

//------------------------------------------------------------------------------
/**
    @struct mat4

    4x4 matrix
*/
struct mat4
{
    float 
    m00, m01, m02, m03,
    m10, m11, m12, m13,
    m20, m21, m22, m23,
    m30, m31, m32, m33;
};

//------------------------------------------------------------------------------
/**
*/
inline vec3
get_row0(mat4 m)
{
    return {m.m00, m.m01, m.m02};
}

//------------------------------------------------------------------------------
/**
*/
inline vec3
get_row1(mat4 m)
{
    return { m.m10, m.m11, m.m12 };
}

//------------------------------------------------------------------------------
/**
*/
inline vec3
get_row2(mat4 m)
{
    return { m.m20, m.m21, m.m22 };
}

//------------------------------------------------------------------------------
/**
*/
inline vec3
get_position(mat4 m)
{
    return { m.m30, m.m31, m.m32 };
}

//------------------------------------------------------------------------------
/**
    transform vector with matrix basis
*/
inline vec3
transform(vec3 v, mat4 m)
{
    vec3 x = vec3{ m.m00, m.m01, m.m02 } * v.x;
    vec3 y = vec3{ m.m10, m.m11, m.m12 } * v.y;
    vec3 z = vec3{ m.m20, m.m21, m.m22 } * v.z;
    return (x + y + z);

    ////swizzle!
    ////this should be easy to vectorize! ;)
    //vec3 x = {v.x, v.x, v.x};
    //vec3 y = {v.y, v.y, v.y};
    //vec3 z = {v.z, v.z, v.z};
    //vec3 r0 = { m.m00, m.m01, m.m02 };
    //vec3 r1 = { m.m10, m.m11, m.m12 };
    //vec3 r2 = { m.m20, m.m21, m.m22 };
    //// multiply
    //vec3 a = mul(x, r0);
    //vec3 b = mul(y, r1);
    //vec3 c = mul(z, r2);
    //// add
    //vec3 res = add(a,b);
    //res = add(res,c);
    //return res;
}

//------------------------------------------------------------------------------
/**
    Create a cartesian space transform from a single normal
*/
inline mat4
TBN(vec3 normal)
{
    mat4 ret;
    ret.m10 = normal.x;
    ret.m11 = normal.y;
    ret.m12 = normal.z;
    ret.m13 = 0.0f;

    if (normal.z < -0.999805696f)
    {
        ret.m00 = 0.0f;
        ret.m01 = -1.0f;
        ret.m02 = 0.0f;
        ret.m03 = 0.0f;

        ret.m20 = -1.0f;
        ret.m21 = 0.0f;
        ret.m22 = 0.0f;
        ret.m23 = 0.0f;
    }
    else
    {
        float a = 1.0f / (1.0f + normal.z);
        float b = -normal.x * normal.y * a;

        ret.m00 = 1.0f - normal.x * normal.x * a;
        ret.m01 = b;
        ret.m02 = -normal.x;
        ret.m03 = 0.0f;

        ret.m20 = b;
        ret.m21 = 1.0f - normal.y * normal.y * a;
        ret.m22 = normal.y;
        ret.m23 = 0.0f;
    }

    ret.m30 = 0.0f;
    ret.m31 = 0.0f;
    ret.m32 = 0.0f;
    ret.m33 = 0.0f;
    return ret;
}

//------------------------------------------------------------------------------
/**
    Calculate determinant
*/
inline float
det(mat4 m)
{
    return 
        (m.m00 * m.m11 - m.m01 * m.m10) * (m.m22 * m.m33 - m.m23 * m.m32)
       -(m.m00 * m.m12 - m.m02 * m.m10) * (m.m21 * m.m33 - m.m23 * m.m31)
       +(m.m00 * m.m13 - m.m03 * m.m10) * (m.m21 * m.m32 - m.m22 * m.m31)
       +(m.m01 * m.m12 - m.m02 * m.m11) * (m.m20 * m.m33 - m.m23 * m.m30)
       -(m.m01 * m.m13 - m.m03 * m.m11) * (m.m20 * m.m32 - m.m22 * m.m30)
       +(m.m02 * m.m13 - m.m03 * m.m12) * (m.m20 * m.m31 - m.m21 * m.m30);
}

//------------------------------------------------------------------------------
/**
    Calculate inverse of matrix
*/
inline mat4
inverse(mat4 m)
{
    float s = det(m);
    
    if (s == 0.0) 
		return {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}; // cannot inverse, make it identity matrix

    s = 1.0f / s;

    return { s * (m.m11*(m.m22*m.m33 - m.m23*m.m32) + m.m12*(m.m23*m.m31 - m.m21*m.m33) + m.m13*(m.m21*m.m32 - m.m22*m.m31)),
             s * (m.m21*(m.m02*m.m33 - m.m03*m.m32) + m.m22*(m.m03*m.m31 - m.m01*m.m33) + m.m23*(m.m01*m.m32 - m.m02*m.m31)),
             s * (m.m31*(m.m02*m.m13 - m.m03*m.m12) + m.m32*(m.m03*m.m11 - m.m01*m.m13) + m.m33*(m.m01*m.m12 - m.m02*m.m11)),
             s * (m.m01*(m.m13*m.m22 - m.m12*m.m23) + m.m02*(m.m11*m.m23 - m.m13*m.m21) + m.m03*(m.m12*m.m21 - m.m11*m.m22)),
             s * (m.m12*(m.m20*m.m33 - m.m23*m.m30) + m.m13*(m.m22*m.m30 - m.m20*m.m32) + m.m10*(m.m23*m.m32 - m.m22*m.m33)),
             s * (m.m22*(m.m00*m.m33 - m.m03*m.m30) + m.m23*(m.m02*m.m30 - m.m00*m.m32) + m.m20*(m.m03*m.m32 - m.m02*m.m33)),
             s * (m.m32*(m.m00*m.m13 - m.m03*m.m10) + m.m33*(m.m02*m.m10 - m.m00*m.m12) + m.m30*(m.m03*m.m12 - m.m02*m.m13)),
             s * (m.m02*(m.m13*m.m20 - m.m10*m.m23) + m.m03*(m.m10*m.m22 - m.m12*m.m20) + m.m00*(m.m12*m.m23 - m.m13*m.m22)),
             s * (m.m13*(m.m20*m.m31 - m.m21*m.m30) + m.m10*(m.m21*m.m33 - m.m23*m.m31) + m.m11*(m.m23*m.m30 - m.m20*m.m33)),
             s * (m.m23*(m.m00*m.m31 - m.m01*m.m30) + m.m20*(m.m01*m.m33 - m.m03*m.m31) + m.m21*(m.m03*m.m30 - m.m00*m.m33)),
             s * (m.m33*(m.m00*m.m11 - m.m01*m.m10) + m.m30*(m.m01*m.m13 - m.m03*m.m11) + m.m31*(m.m03*m.m10 - m.m00*m.m13)),
             s * (m.m03*(m.m11*m.m20 - m.m10*m.m21) + m.m00*(m.m13*m.m21 - m.m11*m.m23) + m.m01*(m.m10*m.m23 - m.m13*m.m20)),
             s * (m.m10*(m.m22*m.m31 - m.m21*m.m32) + m.m11*(m.m20*m.m32 - m.m22*m.m30) + m.m12*(m.m21*m.m30 - m.m20*m.m31)),
             s * (m.m20*(m.m02*m.m31 - m.m01*m.m32) + m.m21*(m.m00*m.m32 - m.m02*m.m30) + m.m22*(m.m01*m.m30 - m.m00*m.m31)),
             s * (m.m30*(m.m02*m.m11 - m.m01*m.m12) + m.m31*(m.m00*m.m12 - m.m02*m.m10) + m.m32*(m.m01*m.m10 - m.m00*m.m11)),
             s * (m.m00*(m.m11*m.m22 - m.m12*m.m21) + m.m01*(m.m12*m.m20 - m.m10*m.m22) + m.m02*(m.m10*m.m21 - m.m11*m.m20)) };
}

//------------------------------------------------------------------------------
/**
    Flip it!
*/
inline mat4
transpose(mat4 m)
{
    return {
        m.m00, m.m10, m.m20, m.m30,
        m.m01, m.m11, m.m21, m.m31,
        m.m02, m.m12, m.m22, m.m32,
        m.m03, m.m13, m.m23, m.m33
    };
}

//------------------------------------------------------------------------------
/**
*/
inline mat4
multiply(mat4 b, mat4 a)
{
    return { b.m00*a.m00 + b.m10*a.m01 + b.m20*a.m02 + b.m30*a.m03,
             b.m01*a.m00 + b.m11*a.m01 + b.m21*a.m02 + b.m31*a.m03,
             b.m02*a.m00 + b.m12*a.m01 + b.m22*a.m02 + b.m32*a.m03,
             b.m03*a.m00 + b.m13*a.m01 + b.m23*a.m02 + b.m33*a.m03,

			 b.m00*a.m10 + b.m10*a.m11 + b.m20*a.m12 + b.m30*a.m13,
             b.m01*a.m10 + b.m11*a.m11 + b.m21*a.m12 + b.m31*a.m13,
             b.m02*a.m10 + b.m12*a.m11 + b.m22*a.m12 + b.m32*a.m13,
             b.m03*a.m10 + b.m13*a.m11 + b.m23*a.m12 + b.m33*a.m13,

			 b.m00*a.m20 + b.m10*a.m21 + b.m20*a.m22 + b.m30*a.m23,
             b.m01*a.m20 + b.m11*a.m21 + b.m21*a.m22 + b.m31*a.m23,
             b.m02*a.m20 + b.m12*a.m21 + b.m22*a.m22 + b.m32*a.m23,
             b.m03*a.m20 + b.m13*a.m21 + b.m23*a.m22 + b.m33*a.m23,

			 b.m00*a.m30 + b.m10*a.m31 + b.m20*a.m32 + b.m30*a.m33,
             b.m01*a.m30 + b.m11*a.m31 + b.m21*a.m32 + b.m31*a.m33,
             b.m02*a.m30 + b.m12*a.m31 + b.m22*a.m32 + b.m32*a.m33,
             b.m03*a.m30 + b.m13*a.m31 + b.m23*a.m32 + b.m33*a.m33 };
}

//------------------------------------------------------------------------------
/**
*/
inline mat4
rotationx(float angle)
{
    float result;
	float c;
	float s;

	if (angle == 180.0f){
		result = MPI;
		s = 0;
	}
	else{
		result = (angle*MPI / 180.0f);
		s = sin(result);		
	}

	c = cos(result);

	return { 1,  0,  0,  0,
             0,  c, -s,  0,
             0,  s,  c,  0,
             0,  0,  0,  1 };
}

//------------------------------------------------------------------------------
/**
*/
inline mat4
rotationy(float angle)
{
    float result;
	float c;
	float s;

	if (angle == 180.0f){
		result = MPI;
		s = 0;
	}
	else{
		result = (angle*MPI / 180.0f);
		s = sin(result);
	}

	c = cos(result);

	return {  c, 0, s, 0,
			  0, 1, 0, 0,
			 -s, 0, c, 0,
              0, 0, 0, 1 };
}