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

Vector::Vector(void) : x(0.0f), y(0.0f), z(0.0f)
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

Vector & Vector::operator=(float * v)
{
	x = v[0];
	y = v[1];
	z = v[2];
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

bool Vector::operator==(float * v) const
{
	return x == v[0] && y == v[1] && z == v[2];
}

bool Vector::operator==(float v) const
{
	return x == v && y == v && z == v;
}

bool Vector::operator!=(const Vector& v) const
{
	return !(*this == v);
}

bool Vector::operator!=(float* v) const
{
	return !(*this == v);
}

bool Vector::operator!=(float v) const
{
	return !(*this == v);
}

Vector Vector::operator+(const Vector& v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator+(float * v) const
{
	return Vector(x + v[0], y + v[1], z + v[2]);
}

Vector Vector::operator+(float v) const
{
	return Vector(x + v, y + v, z + v);
}

Vector Vector::operator-(const Vector& v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator-(float * v) const
{
	return Vector(x - v[0], y - v[1], z - v[2]);
}

Vector Vector::operator-(float v) const
{
	return Vector(x - v, y - v, z - v);
}

Vector Vector::operator*(const Vector & v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator*(float fl) const
{
	return Vector(x*fl, y*fl, z*fl);
}

Vector Vector::operator*(float * v) const
{
	return Vector(x * v[0], y * v[1], z * v[2]);
}

Vector Vector::operator/(const Vector & v) const
{
	return Vector(x / v.x, y / v.y, z / v.z);
}

Vector Vector::operator/(float fl) const
{
	return Vector(x / fl, y / fl, z / fl);
}

Vector Vector::operator/(float * v) const
{
	return Vector(x / v[0], y / v[1], z / v[2]);
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

bool Vector::IsZero(float tolerance) const
{
	return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance && z > -tolerance && z < tolerance);
}

bool Vector::WithinAABB(const Vector & boxmin, const Vector & boxmax) const
{
	return ((x >= boxmin.x) && (x <= boxmax.x) && (y >= boxmin.y) && (y <= boxmax.y) && (z >= boxmin.z) && (z <= boxmax.z));
}

Vector Vector::Min(const Vector & vOther) const
{
	return Vector(x < vOther.x ? x : vOther.x, y < vOther.y ? y : vOther.y, z < vOther.z ? z : vOther.z);
}

Vector Vector::Max(const Vector & vOther) const
{
	return Vector(x > vOther.x ? x : vOther.x, y > vOther.y ? y : vOther.y, z > vOther.z ? z : vOther.z);
}

int Vector::Compare(const Vector & other) const
{
	float diff = GetDifference(other);
	if (diff > 0.0f)
		return 1;
	else if (diff < 0.0f)
		return -1;

	return 0;
}

float Vector::GetDifference(const Vector & other) const
{
	return (*this - other).Length();
}

float Vector::DistanceTo(const Vector & other) const
{
	return (*this - other).Length();
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

void SinCos(float radians, float * sine, float * cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

/*
void AngleVectors(const Vector & angles, Vector & forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);

	forward.x = cp*cy;
	forward.y = cp*sy;
	forward.z = -sp;
}
*/

void VectorAngles(const Vector & forward, Vector & angles)
{
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI_F);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI_F);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

float VectorNormalize(Vector& v)
{
	int		i;
	float	length;

	length = 0;
	for (i = 0; i< 3; i++)
		length += v[i] * v[i];
	length = sqrtf(length);

	for (i = 0; i< 3; i++)
		v[i] /= length;

	return length;
}

void CrossProduct(const float* v1, const float* v2, float* cross)
{
	// Assert(s_bMathlibInitialized);
	// Assert(v1 != cross);
	// Assert(v2 != cross);
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void VectorAngles(const Vector & forward, const Vector & pseudoup, Vector & angles)
{
	Vector left;

	CrossProduct(pseudoup, forward, left);
	VectorNormalize(left);

	float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

	// enough here to get angles?
	if (xyDist > 0.001f)
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));

		// The engine does pitch inverted from this, but we always end up negating it in the DLL
		// UNDONE: Fix the engine to make it consistent
		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

		float up_z = (left[1] * forward[0]) - (left[0] * forward[1]);

		// (roll)	z = ATAN( left.z, up.z );
		angles[2] = RAD2DEG(atan2f(left[2], up_z));
	}
	else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		angles[1] = RAD2DEG(atan2f(-left[0], left[1])); //This was originally copied from the "void MatrixAngles( const matrix3x4_t& matrix, float *angles )" code, and it's 180 degrees off, negated the values and it all works now (Dave Kircher)

														// The engine does pitch inverted from this, but we always end up negating it in the DLL
														// UNDONE: Fix the engine to make it consistent
														// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		angles[2] = 0;
	}
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

Vector & Vector::operator+=(float fl)
{
	x += fl;
	y += fl;
	z += fl;
	return *this;
}

Vector & Vector::operator-=(float fl)
{
	x -= fl;
	y -= fl;
	z -= fl;
	return *this;
}

Vector & Vector::operator*=(float * fl)
{
	x *= fl[0];
	y *= fl[1];
	z *= fl[2];
	return *this;
}

Vector & Vector::operator/=(float * fl)
{
	x /= fl[0];
	y /= fl[1];
	z /= fl[2];
	return *this;
}

Vector & Vector::operator+=(float * fl)
{
	x += fl[0];
	y += fl[1];
	z += fl[2];
	return *this;
}

Vector & Vector::operator-=(float * fl)
{
	x -= fl[0];
	y -= fl[1];
	z -= fl[2];
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

Vector ToEulerAngles(const Vector& src)
{
	float Pitch, Yaw, Length;

	Length = src.Length2D();

	if (Length > 0)
	{
		Pitch = (atan2(-src.z, Length) * 180 / M_PI_F);

		if (Pitch < 0)
		{
			Pitch += 360;
		}

		Yaw = (atan2(src.y, src.x) * 180 / M_PI_F);

		if (Yaw < 0)
		{
			Yaw += 360;
		}
	}
	else
	{
		Pitch = (src.z > 0.0f) ? 270 : 90;
		Yaw = 0;
	}

	return Vector(Pitch, Yaw, 0.0f);
}

Vector ToEulerAngles(Vector* PseudoUp, const Vector& src)
{
	Vector Left;

	float Length, Yaw, Pitch, Roll;

	Left = CrossProduct(*PseudoUp, src);

	Left.Normalize();

	Length = src.Length2D();

	if (PseudoUp)
	{
		if (Length > 0.001)
		{
			Pitch = (atan2(-src.z, Length) * 180 / M_PI_F);

			if (Pitch < 0)
			{
				Pitch += 360;
			}

			Yaw = (atan2(src.y, src.x) * 180 / M_PI_F);

			if (Yaw < 0)
			{
				Yaw += 360;
			}

			float up_z = (Left[1] * src.x) - (Left[0] * src.y);

			Roll = (atan2(Left[2], up_z) * 180 / M_PI_F);

			if (Roll < 0)
			{
				Roll += 360;
			}
		}
		else
		{
			Yaw = (atan2(src.y, src.x) * 180 / M_PI_F);

			if (Yaw < 0)
			{
				Yaw += 360;
			}

			Pitch = (atan2(-src.z, Length) * 180 / M_PI_F);

			if (Pitch < 0)
			{
				Pitch += 360;
			}

			Roll = 0;
		}
	}
	else
	{
		if (Length > 0)
		{
			Pitch = (atan2(-src.z, Length) * 180 / M_PI_F);

			if (Pitch < 0)
			{
				Pitch += 360;
			}

			Yaw = (atan2(src.y, src.x) * 180 / M_PI_F);

			if (Yaw < 0)
			{
				Yaw += 360;
			}
		}
		else
		{
			Pitch = (src.z > 0.0f) ? 270.0f : 90.0f;
			Yaw = 0;
		}
	}

	return  Vector(Pitch, Yaw, Roll);
}

void AngleNormalize(float* angles)
{
	if (angles[0] > 89.f)
	{
		angles[0] = 89.f;
	}
	else if (-89.f > angles[0])
	{
		angles[0] = -89.f;
	}

	if (angles[1] > 180.f)
	{
		angles[1] -= 360.f;
	}
	else if (-180.f > angles[1])
	{
		angles[1] += 360.f;
	}

	angles[2] = 0.f;
}

void AngleVectors(const Vector & angles, Vector & forward)
{
	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);

	forward.x = cp*cy;
	forward.y = cp*sy;
	forward.z = -sp;
}

void AngleVectors(const Vector & angles, Vector & forward, Vector & right, Vector & up)
{
	float sr, sp, sy, cr, cp, cy;

#ifdef _X360
	fltx4 radians, scale, sine, cosine;
	radians = LoadUnaligned3SIMD(angles.Base());
	scale = ReplicateX4(M_PI_F / 180.f);
	radians = MulSIMD(radians, scale);
	SinCos3SIMD(sine, cosine, radians);
	sp = SubFloat(sine, 0);	sy = SubFloat(sine, 1);	sr = SubFloat(sine, 2);
	cp = SubFloat(cosine, 0);	cy = SubFloat(cosine, 1);	cr = SubFloat(cosine, 2);
#else
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);
#endif

	if (forward)
	{
		forward.x = cp*cy;
		forward.y = cp*sy;
		forward.z = -sp;
	}

	if (right)
	{
		right.x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right.y = (-1 * sr*sp*sy + -1 * cr*cy);
		right.z = -1 * sr*cp;
	}

	if (up)
	{
		up.x = (cr*sp*cy + -sr*-sy);
		up.y = (cr*sp*sy + -sr*cy);
		up.z = cr*cp;
	}
}

void VectorVectors(const Vector & forward, Vector & right, Vector & up)
{
	Vector tmp;

	if (forward[0] == 0 && forward[1] == 0)
	{
		// pitch 90 degrees up/down from identity
		right[0] = 0;
		right[1] = -1;
		right[2] = 0;
		up[0] = -forward[2];
		up[1] = 0;
		up[2] = 0;
	}
	else
	{
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 1.0;
		CrossProduct(forward, tmp, right);
		VectorNormalize(right);
		CrossProduct(right, forward, up);
		VectorNormalize(up);
	}
}
