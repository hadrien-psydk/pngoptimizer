///////////////////////////////////////////////////////////////////////////////
// This file is part of the chustd library
// Copyright (C) ChuTeam
// For conditions of distribution and use, see copyright notice in chustd.h
///////////////////////////////////////////////////////////////////////////////

#ifndef CHUSTD_MATH_H
#define CHUSTD_MATH_H

namespace chustd {

class Math  
{
public:
	static const float64 fPiDiv2_64;
	static const float64 fPi_64;
	static const float64 f2Pi_64;

	static const float32 fPiDiv2_32;
	static const float32 fPi_32;
	static const float32 f2Pi_32;
	
	static const float64 fSqrt2_64;
	static const float64 fSqrt3_64;
	static const float64 fSqrt3Div2_64;

	static const float32 fSqrt2_32;
	static const float32 fSqrt3_32;
	static const float32 fSqrt3Div2_32;
	
	static inline float64 DegToRad(float64 fDegAngle)
	{
		return ( fDegAngle * fPi_64) / 180.0;
	}
	
	static inline float64 RadToDeg(float64 fRadAngle)
	{
		return ( fRadAngle * 180.0) / fPi_64;
	}

	static inline float64 DegToRad(float32 fDegAngle)
	{
		return ( fDegAngle * fPi_32) / 180.0f;
	}
	
	static inline float64 RadToDeg(float32 fRadAngle)
	{
		return ( fRadAngle * 180.0f) / fPi_32;
	}

	static inline float32 CosDeg(float64 fDegAngle)
	{
		return float32( cos( DegToRad(fDegAngle) ) );
	}

	static inline float32 SinDeg(float64 fDegAngle)
	{
		return float32( sin( DegToRad(fDegAngle) ) );
	}

	static inline float32 TanDeg(float64 fDegAngle)
	{
		return float32( tan( DegToRad(fDegAngle) ) );
	}

	static inline float32 CosRad(float64 dRadAngle)
	{
		return float32( cos( dRadAngle ) );
	}

	static inline float32 SinRad(float64 dRadAngle)
	{
		return float32( sin( dRadAngle ) );
	}

	static inline float64 SinRad64(float64 dRadAngle)
	{
		return float64( sin( dRadAngle ) );
	}

	static inline float32 TanRad(float64 dRadAngle)
	{
		return float32( tan( dRadAngle ) );
	}
	
	static inline float32 Min3f(float32 s0, float32 s1, float32 s2)
	{
		if( s0 < s1 )
		{
			if( s0 < s2 )
				return s0; 
			
			return s2;
		}
		
		if( s1 < s2 )
			return s1;
		
		return s2;
	}

	static inline float32 Max3f(float32 s0, float32 s1, float32 s2)
	{
		if( s0 > s1 )
		{
			if( s0 > s2 )
				return s0; 
			
			return s2;
		}
		
		if( s1 > s2 )
			return s1;
		
		return s2;
	}

	static inline int32 Min3n(int32 s0, int32 s1, int32 s2)
	{
		if( s0 < s1 )
		{
			if( s0 < s2 )
				return s0; 
			
			return s2;
		}
		
		if( s1 < s2 )
			return s1;
		
		return s2;
	}

	static inline int32 Max3n(int32 s0, int32 s1, int32 s2)
	{
		if( s0 > s1 )
		{
			if( s0 > s2 )
				return s0; 
			
			return s2;
		}
		
		if( s1 > s2 )
			return s1;
		
		return s2;
	}

	static inline float32 Min3fIndex(float32 s0, float32 s1, float32 s2, int8& miindex)
	{
		if( s0 < s1 )
		{
			if( s0 < s2 )
			{
				miindex = 0;
				return s0;
			}
			
			miindex = 2;
			return s2;
		}
		
		if( s1 < s2 )
		{
			miindex = 1;
			return s1;
		}
		
		miindex = 2;
		return s2;
	}

	static inline int32 Min3index(int32 s0, int32 s1, int32 s2, int8& miindex)
	{
		if( s0 < s1 )
		{
			if( s0 < s2 )
			{
				miindex = 0;
				return s0;
			}
			
			miindex = 2;
			return s2;
		}
		
		if( s1 < s2 )
		{
			miindex = 1;
			return s1;
		}
		
		miindex = 2;
		return s2;
	}

	static inline int Min3Index(int s0, int s1, int s2)
	{
		if( s0 < s1 )
		{
			if( s0 < s2 )
			{
				return 0;
			}
			return 2;
		}
		if( s1 < s2 )
		{
			return 1;
		}
		return 2;
	}

	template <class T>
	static void Swap(T& t0, T& t1)
	{
		T t2 = t0;
		t0 = t1;
		t1 = t2;
	}

	static inline void Clamp1(float32& f)
	{
		f = (f > 1.0f) ? 1.0f : f;
	}

	static inline void Clamp1(float64& d)
	{
		d = (d > 1.0) ? 1.0 : d;
	}

	static inline void Clamp01(float32& f)
	{
		if( f > 1.0 )
		{
			f = 1.0;
		}
		else if( f < 0 )
		{
			f = 0;
		}
	}

	static inline void Clamp01(float64& d)
	{
		if( d > 1.0 )
		{
			d = 1.0;
		}
		else if( d < 0 )
		{
			d = 0;
		}
	}

	static inline int32 Max(int32 value1, int32 value2)
	{
		if(value1 > value2)
			return value1;
		else
			return value2;
	}

	static inline int32 Min(int32 value1, int32 value2)
	{
		if(value1 < value2)
			return value1;
		else
			return value2;
	}

	static inline float32 MaxAbs(float32 f1, float32 f2)
	{
		float32 f1abs = AbsF(f1);
		float32 f2abs = AbsF(f2);

		if( f1abs > f2abs )
			return f1abs;
		return f2abs;
	}

	// -24 is ~24 + 1
	static inline int32 Abs(int32 n)
	{
		int32 mask = n >> 31;
		int32 add = mask & 1;
		return (n ^ mask) + add;
	}

	static inline float32 AbsF(float32 f)
	{
		return float32( fabs( float64(f) ));
	}

	static inline float64 AbsF(float64 f)
	{
		return fabs(f);
	}
	
	static inline float32 Sqrt(float32 f)
	{
		return float32( sqrt( float64(f) ));
	}

	static inline float64 Sqrt64(float64 d)
	{
		return sqrt(d);
	}

	static inline bool FloatEq(float32 x, float32 v, float32 epsilon = 0.00001f)
	{
		if( (((v - epsilon) < x) && (x <( v + epsilon))) )
			return true;
		return false;
	}

	static inline float32 ToZero(float32 x, float32 epsilon = 0.00001f)
	{
		if( FloatEq(x, 0.0f, epsilon) )
			return 0.0f;
		return x;
	}

	static inline float32 Atan2Deg(float32 x, float32 y)
	{
		return float32( RadToDeg( atan2(y, x)) );
	}

	static inline float32 Atan2Rad(float32 x, float32 y)
	{
		return float32( atan2(y, x));
	}

	// Positive result between 0 and 2pi
	static inline float32 Atan2RadPos(float32 x, float32 y)
	{
		return float32( fmod( (atan2(y, x) + f2Pi_64), f2Pi_64));
	}

	// Positive result between 0 and 2pi
	static inline float32 Atan2DegPos(float32 x, float32 y)
	{
		return float32( RadToDeg( fmod( (atan2(y, x) + f2Pi_64), f2Pi_64)));
	}

	// Divides large by divider. Adds 1 if a remainder exists
	static inline int32 DivCeil(int32 large, int32 divider)
	{
		const int32 res = large / divider + ((large % divider) > 0 ? 1 : 0);
		return res;
	}

	// Divides large by divider. Adds 1 if a remainder exists
	static inline int64 DivCeil64(int64 large, int64 divider)
	{
		const int64 res = large / divider + ((large % divider) > 0 ? 1 : 0);
		return res;
	}

	static bool Clamp(int32& nToClamp, int32 min, int32 max)
	{
		ASSERT(min < max);

		if ( nToClamp > max )
		{
			nToClamp = max;
			return true;
		}
		else if ( nToClamp < min )
		{
			nToClamp = min;
			return true;
		}

		return false;
	}

	static bool Clamp(float32& fToClamp, float32 fMin, float32 fMax)
	{
		ASSERT(fMin < fMax);

		if ( fToClamp > fMax )
		{
			fToClamp = fMax;
			return true;
		}
		else if ( fToClamp < fMin )
		{
			fToClamp = fMin;
			return true;
		}

		return false;
	}

	// Computes the logarithm of 2 as n = ToIntTruncate( Log2f(n));
	static uint32 Log2(uint32 val);

	static inline uint32 NextPowerOf2(uint32 n)
	{
		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;
		++n;
		return n;
	}

	static inline bool IsPowerOf2(int32 n)
	{
		int32 n2 = n & -n;
		return n2 == n;
	}

	static int32 RoundAtNearestMultiple(int32 value, const int32 multiple)
	{
		int32 remain = value % multiple;
		value -= remain;
		
		if( remain > (multiple / 2) )
			value += multiple;

		return value;
	}

	static int64 RoundToNextMultiple64(int64 value, const int64 multiple)
	{
		return DivCeil64(value, multiple) * multiple;
	}

	// TODO: uncomment once sqrt is implemented
	//static bool ResolveQuadraticEquation(const float64 a, const float64 b, const float64 c,	float64& dRoot1, float64& dRoot2);

	// Integer square root
	static uint32 IntegerSqrt(uint32 v);
};

} // namespace chustd

#endif // ndef CHUSTD_MATH_H
