#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

struct Vector3 {
	float x;
	float y;
	float z;

	Vector3 operator+ (const Vector3& v) const {
		return {x + v.x, y + v.y, z + v.z};
	}
	Vector3 operator- (const Vector3& v) const {
		return {x - v.x, y - v.y, z - v.z};
	}
	Vector3 operator* (const float s) const { return {x * s, y * s, z * s}; }
	Vector3& operator+= (const Vector3& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
};

inline float dot (const Vector3& a, const Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 cross (const Vector3& a, const Vector3& b) {
	return {
		a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x
	};
}

inline float magnitude (const Vector3 v) {
	return sqrt (v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector3 normalise (const Vector3 v) {
	const float len = magnitude (v);
	return Vector3 (v.x / len, v.y / len, v.z / len);
}

inline float noise (const float x, const float y, const float z) {
	return sin (x * 3.1f) * cos (y * 2.7f) * sin (z * 4.3f);
}

#endif // VECTOR_H
