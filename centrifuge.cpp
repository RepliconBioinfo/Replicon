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
#include <iostream>
#include <stdlib.h>
#include "centrifuge.h"
#include "cell.h"
/** An object simulating the effect of a centrifuge. It "accepts" nacent DNA strands and
  * enriches shorter sequences.
  */

centrifuge::centrifuge ( threadVariables &t ) : threadVariablesClient ( t )
{}


centrifuge::~centrifuge()
{}

void centrifuge::source ( cell &c )
{
	_cell = &c;
	const unsigned int n=c.nFragment();
	_fragment.clear();
	_fragment.reserve ( n );
	for ( unsigned int i=0; i < n; ++i )
	{
		if ( accept ( c.nacentDna ( i ) ) ) _fragment.push_back ( i );
	}
	_n = _fragment.size();
	return;
}


const dna::fragment &centrifuge::nacentDna ( int i ) const
{
	return _cell->nacentDna ( _fragment[i] );
}


bool cutoffCentrifuge::accept ( const dna::fragment &f )
{
	unsigned int l = f.end - f.begin;
	bool r = l <= _cutoff;
	return r;
}

bool gaussCentrifuge::accept ( const dna::fragment &f )
{
	float l = static_cast<float> ( abs ( f.end - f.begin ) )-center();;
	float g = gauss() * _variance;
//  std::cout << g << " " << l << " " << "AII\n";
	bool  r = ( ( l <= 0. ) && ( g < l ) ) || ( ( l > 0. ) && ( g > l ) );
	return r;
}
