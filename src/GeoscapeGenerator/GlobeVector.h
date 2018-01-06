#pragma once
/*
 * Copyright 2010-2018 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../fmath.h"
#include <math.h>
#include <cmath>
#include <algorithm>
#include <cfloat>
#include "../Engine/Logger.h"
#define _USE_MATH_DEFINES

#ifndef M_DEG_TO_RAD
#define M_DEG_TO_RAD  0.01745329251994329547
#define M_RAD_TO_DEG  57.29577951308232286465
#endif
 
namespace OpenXcom
{

/*
 * Class for handling of vector rotation in 3d
 */
class Quaternion
{
public:
	double c, i, j, k;
	
	/// Default constructor
	constexpr Quaternion() : c(0.0), i(0.0), j(0.0), k(0.0) {};
	/// Constructor from components
	constexpr Quaternion(double c_, double i_, double j_, double k_) : c(c_), i(i_), j(j_), k(k_) {};
	/// Constructor for copying
	constexpr Quaternion(const Quaternion& quat) : c(quat.c), i(quat.i), j(quat.j), k(quat.k) {};
	
	/// Define multiplication operator
	Quaternion operator*(const Quaternion& quat)
	{
		return Quaternion(c * quat.c - i * quat.i - j * quat.j - k * quat.k,
			c * quat.i + i * quat.c + j * quat.k - k * quat.j,
			c * quat.j - i * quat.k + j * quat.c + k * quat.i,
			c * quat.k + i * quat.j - j * quat.i + k * quat.c);
	}
	
	/// Define inverse, also used for rotations
	Quaternion inverse()
	{
		return Quaternion(c, -i, -j, -k);
	}

	/// Write values to openxcom.log
	void writeToLog() const
	{
		Log(LOG_INFO) << "  (" << c << ", " << i << ", " << j << ", " << k << ")";
	}
};

/*
 * Class for handling of positions on the globe as vectors on a unit sphere
 * Assumes all angles in degrees as latitude and longitude
 */
class GlobeVector
{
public:
	double lat, lon, x, y, z;
	
	/// Default constructor
	constexpr GlobeVector() : lat(0), lon(0), x(0), y(0), z(0) {};
	/// Constructor for given latitude and longitude in degrees
	GlobeVector(double lat_, double lon_)
	{
		lat = std::max(-90.0, lat_); // lower bound for latitude
		lat = std::min(90.0, lat); // upper bount for latitude
		lon = std::fmod(lon_, 360.0); // limit longitude to 0-360 degrees
		
		// Convert the angles to Cartesian coordinates
		z = sin(lat * M_DEG_TO_RAD);
		y = sin(lon * M_DEG_TO_RAD) * cos(lat * M_DEG_TO_RAD);
		x = cos(lon * M_DEG_TO_RAD) * cos(lat * M_DEG_TO_RAD);
	}
	/// Constructor for given Cartesian coordinates
	GlobeVector(double x_, double y_, double z_)
	{
		// Normalize x, y, and z to put the vector on the unit sphere
		double norm = sqrt(x_ * x_ + y_ * y_ + z_ * z_);
		x = x_ / norm;
		y = y_ / norm;
		z = z_ / norm;
		
		lat = asin(z) * M_RAD_TO_DEG;
		lat = std::max(-90.0, lat); // lower bound for latitude
		lat = std::min(90.0, lat); // upper bount for latitude
		
		if (y > 0)
		{
			lon = atan2(y, x) * M_RAD_TO_DEG;
		}
		else
		{
			lon = atan2(y, x) * M_RAD_TO_DEG + 360.0;
		}
	}
	/// Converts from a quaternion (assume constant part is 0) to a unit vector
	GlobeVector(const Quaternion& quat)
	{
		double norm = sqrt(quat.i * quat.i + quat.j * quat.j + quat.k * quat.k);
		x = quat.i / norm;
		y = quat.j / norm;
		z = quat.k / norm;
		
		lat = asin(z) * M_RAD_TO_DEG;
		lat = std::max(-90.0, lat); // lower bound for latitude
		lat = std::min(90.0, lat); // upper bount for latitude
		
		if (y > 0)
		{
			lon = atan2(y, x) * M_RAD_TO_DEG;
		}
		else
		{
			lon = atan2(y, x) * M_RAD_TO_DEG + 360.0;
		}
	}
	/// Constructor for copying
	constexpr GlobeVector(const GlobeVector& vec) : lat(vec.lat), lon(vec.lon), x(vec.x), y(vec.y), z(vec.z) {};
	
	/// Define multiplication operator as cross-product of vectors, returns a unit vector along direction of cross-product
	GlobeVector operator*(const GlobeVector& vec) const
	{
		return GlobeVector(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
	}

	/// Define scalar multiplication by int
	GlobeVector operator*(int c) const { return GlobeVector(x*c, y*c, z*c); };

	/// Get the dot product of this globe vector and another
	double dot(const GlobeVector& vec)
	{
		return (x * vec.x + y * vec.y + z * vec.z);
	}

	/// Get the great circle distance between this and another vector, using the dot product
	double distance(const GlobeVector& vec)
	{
		double dotProduct = dot(vec);
		return acos(dotProduct);
	}
	
	/// Gets a rotation of one vector around another, assume rotation angle in degrees clockwise when facing origin
	GlobeVector rotate(const GlobeVector& vec, double theta)
	{
		Quaternion quat(0.0, x, y, z);
		//GlobeVector rotationVector = (*this) * vec;
		theta *= (M_DEG_TO_RAD / 2);
		Quaternion rotationQuat(cos(theta), sin(theta) * vec.x, sin(theta) * vec.y, sin(theta) * vec.z);
		return(GlobeVector(rotationQuat * (quat * rotationQuat.inverse())));
	}

	/// Write values to openxcom.log
	void writeToLog() const
	{
		Log(LOG_INFO) << "  (" << x << ", " << y << ", " << z << ", " << lat << ", " << lon << ")";
	}
	

};

}
