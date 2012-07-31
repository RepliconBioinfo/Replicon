/* $Id: kiss.h,v 1.1 1998/06/05 13:35:20 bilke Exp $ */
// A fast random number generator
#ifndef __kiss
#define __kiss
#include <math.h>
class kiss
{
	private:
		struct
		{
			unsigned int s1;
			unsigned int s2;
			unsigned int s3;
			float gset;
			bool iset;
		}
		_k __attribute__ ( ( aligned ) );

		inline unsigned int ml ( unsigned int k, unsigned int n )
		{
			return ( k ^ ( k << n ) );
		}

		inline unsigned int mr ( unsigned int k, unsigned int n )
		{
			return ( k ^ ( k >> n ) );
		}

	public:
		kiss()
		{
			_k.s1 = 12710,
			_k.s2 = 17609,
			_k.s3 = 57344;
			_k.iset = false;
		}

		inline float ran()
		{
			_k.s1= ( 69069*_k.s1+23606797 );
			_k.s2=mr ( ml ( _k.s2,17 ),15 );
			_k.s3=mr ( ml ( _k.s3,18 ),13 );
			return ( float ) ( _k.s1+_k.s2+_k.s3 ) /  4294967295.;
		}

		inline int ranN ( unsigned int max )
		{
			_k.s1= ( 69069*_k.s1+23606797 );
			_k.s2=mr ( ml ( _k.s2,17 ),15 );
			_k.s3=mr ( ml ( _k.s3,18 ),13 );
			return ( _k.s1+_k.s2+_k.s3 ) % max;
		}


		float gauss()
		{
			if ( _k.iset )
			{
				_k.iset = false;
				return _k.gset;
			}
			float rsq, v1, v2;
			do
			{
				v1 = 2.0 * ran() - 1.0;
				v2 = 2.0 * ran() - 1.0;
				rsq = v1*v1 + v2*v2;
			}
			while ( rsq >= 1.0 ||  rsq == 0.0 );
			float fac = sqrt ( -2.0 * log ( rsq ) /rsq );
			_k.gset = v1 * fac;
			_k.iset = true;
			return v2 * fac;
		}

		void seed ( unsigned int s1, unsigned int s2, unsigned int s3 )
		{
			_k.s1=s1;
			_k.s2=s2;
			_k.s3=s3;
		}

};
#endif
