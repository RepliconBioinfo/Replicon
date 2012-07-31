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
#ifndef MICROARRAY_H
#define MICROARRAY_H
#include <vector>
#include <iostream>
#include "dna.h"

using std::vector;
using std::ostream;
/**
	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class microArray
{
	public:
		microArray();
		microArray ( const microArray &A );
		~microArray();
		void clear();
		double intensity ( unsigned int c, unsigned int p ) const;

		template <class A> void hybridize ( const A &a )
		{
			unsigned int n=a.nFragment();
			for ( unsigned int i=0; i < n; i++ ) hybridize ( a.nacentDna ( i ) );
			return;
		}

		ostream &operator<< ( ostream & ) const;
		microArray &operator= ( const microArray &A );
		microArray &operator+= ( const microArray &A );
	private:
		vector<vector<int> > _array;
		unsigned int _nfrag;
};

template <> void microArray::hybridize ( const dna::fragment & );

ostream &operator<< ( ostream &o, const microArray &a );
#endif
