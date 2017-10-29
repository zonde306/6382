/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//  Vector.h
// A subset of the extdll.h in the project HL Entity DLL
//

#if !defined FILE_UTIL_VECTOR_H
#define FILE_UTIL_VECTOR_H

// Misc C-runtime library headers
// #include "STDIO.H"
// #include "STDLIB.H"
// #include "MATH.H"

// Header file containing definition of globalvars_t and entvars_t
typedef int	func_t;					//
typedef int	string_t;				// from engine's pr_comp.h;
typedef float vec_t;				// needed before including progdefs.h

//=========================================================
// 2DVector - used for many pathfinding and many other 
// operations that are treated as planar rather than 3d.
//=========================================================
class Vector2D
{
public:
	Vector2D(void);
	Vector2D(float X, float Y);
	Vector2D operator+(const Vector2D& v) const;
	Vector2D operator-(const Vector2D& v) const;
	Vector2D operator*(float fl) const;
	Vector2D operator/(float fl) const;
	
	float Length(void) const;

	Vector2D Normalize ( void ) const;

	vec_t	x, y;
};

float DotProduct(const Vector2D& a, const Vector2D& b);
Vector2D operator*(float fl, const Vector2D& v);

//=========================================================
// 3D Vector
//=========================================================
class Vector						// same data-layout as engine's vec3_t,
{								//		which is a vec_t[3]
public:
	// Construction/destruction
	Vector(void);
	Vector(float X, float Y, float Z);
	Vector(double X, double Y, double Z);
	Vector(int X, int Y, int Z);
	Vector(const Vector& v);
	Vector(const Vector&& v);
	Vector(float rgfl[3]);

	// Operators
	Vector operator-(void) const;
	bool operator==(const Vector& v) const;
	bool operator!=(const Vector& v) const;
	Vector operator+(const Vector& v) const;
	Vector operator-(const Vector& v) const;
	Vector operator*(float fl) const;
	Vector operator/(float fl) const;
	Vector& operator=(const Vector& v);
	Vector& operator=(const Vector&& v);

	Vector& operator*=(float fl);
	Vector& operator/=(float fl);
	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);
	Vector& operator*=(const Vector& v);
	Vector& operator/=(const Vector& v);

	// Methods
	void CopyToArray(float* rgfl) const;
	float Length(void) const;
	operator float *(); // Vectors will now automatically convert to float * when needed
	operator const float *() const; // Vectors will now automatically convert to float * when needed
	Vector Normalize(void) const;

	Vector2D Make2D ( void ) const;
	float Length2D(void) const;

	// Members
	vec_t x, y, z;
};

Vector operator*(float fl, const Vector& v);
float DotProduct(const Vector& a, const Vector& b);
Vector CrossProduct(const Vector& a, const Vector& b);

#ifndef vec3_t
#define vec3_t Vector
#endif


#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorClear(a) { a[0]=0.0;a[1]=0.0;a[2]=0.0;}

#endif
