/***************************************************************************
 *   Copyright (C) 2006 by Sven Bilke                                      *
 *   bilkes@mail.nih.gov                                                   *
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
#include <assert.h>
#include "dna.h"
#include "cell.h"
dna::dna ( int c ) :  _length ( cell::chromosomeLength ( c ) ), 
                      _replicated ( 0 ),
                      _id ( c ),
                      _fragDirty (false ),
                      _nfrag ( 0 )
{
	_dnaCopyNumber.resize ( length() );

#ifdef MEASURE_REPLICATION_TIMING
	_replicationTime.resize ( length() );
	_initiationTime.resize ( length() );
	std::fill ( _replicationTime.begin(), _replicationTime.end(), 0 );
	std::fill ( _initiationTime.begin(), _initiationTime.end(), 0 );
#endif
}

dna::~dna()
{}

void dna::enterSPhase()
{
	std::fill ( _dnaCopyNumber.begin(), _dnaCopyNumber.end(), false );
#ifdef MEASURE_REPLICATION_TIMING
	std::fill ( _replicationTime.begin(), _replicationTime.end(), 0 );
	std::fill ( _initiationTime.begin(), _initiationTime.end(), -1 );
#endif
	_fragDirty = false;
	_fragCache.clear();
	_nfrag = 0;
	_replicated = 0;
}



void dna::_createNacentList() const
{
	_nfrag = 0;
	_fragCache.clear();

	fragment cur;
	cur.chromosome = _id;
	bool inFrag = false;
	for ( int i=0; i < length(); i++ )
	{
		if ( _dnaCopyNumber[i] && !inFrag )
		{
			inFrag = true;
			cur.begin = i;
			continue;
		}
		if ( inFrag && !_dnaCopyNumber[i] )
		{
			inFrag = false;
			cur.end = i - 1;
			_fragCache.push_back ( cur );
			++_nfrag;
			continue;
		}
	}
	if ( inFrag )
	{
		cur.end = length() - 1;
		_fragCache.push_back ( cur );
		++_nfrag;
	}
	_fragDirty = false;
	return;
}
