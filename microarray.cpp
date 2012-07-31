/***************************************************************************
 *   Copyright (C) 2006 by Sven Bilke   *
 *   bilkes@mail.nih.gov   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <algorithm>
#include <functional>
#include "microarray.h"
#include "cell.h"

microArray::microArray()
{
	clear();
}

microArray::microArray ( const microArray &A )
{
	*this = A;
}

microArray::~microArray()
{}

microArray &microArray::operator= ( const microArray &A )
{
	_array = A._array;
	_nfrag = A._nfrag;
	return *this;
}

microArray &microArray::operator+= ( const microArray &A )
{
	for ( unsigned int c=0; c<cell::nChromosomes(); ++c )
	{
		std::transform ( _array[c].begin(), _array[c].end(), A._array[c].begin(), _array[c].begin(), std::plus<int>() );
	}
	_nfrag += A._nfrag;
	return *this;
}

void microArray::clear()
{
	const unsigned int n = cell::nChromosomes();
	_array.resize ( n );
	for ( unsigned int c=0; c < n; c++ )
	{
		_array[c].resize ( cell::chromosomeLength ( c ) );
		std::fill ( _array[c].begin(), _array[c].end(), 0 );
	}
	_nfrag = 0;
}

double microArray::intensity ( unsigned int c, unsigned int p ) const
{
	int count = _array[c][p];
	double i = static_cast<double> ( count ) / static_cast<double> ( _nfrag );
	return i;
}

template <> void microArray::hybridize ( const dna::fragment &f )
{
	vector<int> &a = _array[f.chromosome];
	std::transform ( a.begin() +f.begin, a.begin() +f.end+1, a.begin() +f.begin,
	                 std::bind1st ( std::plus<int>(), 1 ) );
	++_nfrag;
	return;
}

std::ostream &microArray::operator << ( ostream &o ) const
{
	const unsigned int n = cell::nChromosomes();
	o << "Chromosome\tpos\tI" << std::endl;
    for ( unsigned int c=0; c < n; c++ )
	{
		const unsigned int l=cell::chromosomeLength ( c );
		for ( unsigned int p=0; p < l; p++ )
		{
			o << c << "\t" << p << "\t" << intensity ( c,p ) << "\n";
		}
	}
	return o;
}

ostream &operator<< ( ostream &o, const microArray &a ) { return a.operator<< ( o ); }
