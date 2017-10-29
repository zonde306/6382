#include "./Engine/util_vector.h"
#include <cmath>

Vector2D::Vector2D()
{

}

Vector2D::Vector2D(float X, float Y) : x(X), y(Y)
{

}

Vector2D Vector2D::operator+(const Vector2D& v) const
{
	return Vector2D(x + v.x, y + v.y);
}

Vector2D Vector2D::operator-(const Vector2D& v) const
{
	return Vector2D(x - v.x, y - v.y);
}

Vector2D Vector2D::operator*(float fl) const
{
	return Vector2D(x*fl, y*fl);
}

Vector2D Vector2D::operator/(float fl) const
{
	return Vector2D(x / fl, y / fl);
}

float Vector2D::Length(void) const
{
	return (float)sqrt(x*x + y*y);
}

Vector2D Vector2D::Normalize(void) const
{
	Vector2D vec2;

	float flLen = Length();
	if (flLen == 0)
	{
		return Vector2D((float)0, (float)0);
	}
	else
	{
		flLen = 1 / flLen;
		return Vector2D(x * flLen, y * flLen);
	}
}

float DotProduct(const Vector2D& a, const Vector2D& b)
{
	return(a.x*b.x + a.y*b.y);
}

Vector2D operator*(float fl, const Vector2D& v)
{
	return v * fl;
}

Vector::Vector(void)
{

}

Vector::Vector(float X, float Y, float Z) : x(X), y(Y), z(Z)
{

}

Vector::Vector(double X, double Y, double Z) : x((float)X), y((float)Y), z((float)Z)
{

}

Vector::Vector(int X, int Y, int Z) : x((float)X), y((float)Y), z((float)Z)
{

}

Vector::Vector(const Vector& v) : x(v.x), y(v.y), z(v.z)
{

}

Vector::Vector(const Vector && v) : x(v.x), y(v.y), z(v.z)
{

}

Vector::Vector(float rgfl[3]) : x(rgfl[0]), y(rgfl[1]), z(rgfl[2])
{

}

Vector & Vector::operator=(const Vector & v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector & Vector::operator=(const Vector && v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector Vector::operator-(void) const
{
	return Vector(-x, -y, -z);
}

bool Vector::operator==(const Vector& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool Vector::operator!=(const Vector& v) const
{
	return !(*this == v);
}

Vector Vector::operator+(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator-(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator*(float fl) const
{
	return Vector(x*fl, y*fl, z*fl);
}

Vector Vector::operator/(float fl) const
{
	return Vector(x / fl, y / fl, z / fl);
}

void Vector::CopyToArray(float* rgfl) const
{
	rgfl[0] = x, rgfl[1] = y, rgfl[2] = z;
}

float Vector::Length(void) const
{
	return (float)sqrt(x*x + y*y + z*z);
}

Vector::operator float *() { return &x; }
Vector::operator const float *() const { return &x; }

Vector Vector::Normalize(void) const
{
	float flLen = Length();
	if (flLen == 0) return Vector(0, 0, 1); // ????
	flLen = 1 / flLen;
	return Vector(x * flLen, y * flLen, z * flLen);
}

Vector2D Vector::Make2D(void) const
{
	Vector2D	Vec2;

	Vec2.x = x;
	Vec2.y = y;

	return Vec2;
}

float Vector::Length2D(void) const
{
	return (float)sqrt(x*x + y*y);
}

Vector operator*(float fl, const Vector& v)
{
	return v * fl;
}

float DotProduct(const Vector& a, const Vector& b)
{
	return(a.x*b.x + a.y*b.y + a.z*b.z);
}

Vector CrossProduct(const Vector& a, const Vector& b)
{
	return Vector(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

Vector & Vector::operator*=(float fl)
{
	x *= fl;
	y *= fl;
	z *= fl;
	return *this;
}

Vector & Vector::operator/=(float fl)
{
	x /= fl;
	y /= fl;
	z /= fl;
	return *this;
}

Vector & Vector::operator+=(const Vector & v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector & Vector::operator-=(const Vector & v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector & Vector::operator*=(const Vector & v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vector & Vector::operator/=(const Vector & v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}
