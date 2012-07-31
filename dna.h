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
#ifndef DNA_H
#define DNA_H
#include <vector>
#include <iostream>
#include "config.h"
using std::vector;

/**
Simulates a single (double stranded) DNA molecule

	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class dna
{
	public:
//    typedef vector<double> doublev;
		typedef struct { int begin, end, chromosome;}
		fragment;

		dna ( int c );
		~dna();

		bool   isReplicated ( int l )  const { return _dnaCopyNumber[l]; }
		bool   complete()           const { return _replicated >= _length; }
		int    length()             const { return _length; }
		int    replicated()         const { return _replicated; }
		unsigned int nFragment()    const { if ( _fragDirty ) _createNacentList(); return _nfrag; }
	const fragment &nacentDna ( int i )  const { if ( _fragDirty ) _createNacentList(); return _fragCache[i]; }

#ifdef MEASURE_REPLICATION_TIMING
	const vector<unsigned int> &replicationTime() const  { return _replicationTime[_timer ? 0 : 1]; } ;
#endif

		void   replicate ( int l, unsigned int t )
		{
			_dnaCopyNumber[l] = true;
			_replicated++;
			_fragDirty = true;
#ifdef MEASURE_REPLICATION_TIMING
			_replicationTime[_timer ? 0 : 1][l] = t;
#endif
			return;
		};

		void enterSPhase();
		unsigned int chromosomeId()       { return _id; }
	private:
		void _createNacentList() const;

		unsigned int _length;
		unsigned int _replicated;
		unsigned char _id;
		mutable bool _fragDirty;
		mutable unsigned int _nfrag;

		vector<bool>    _dnaCopyNumber;
		mutable vector<fragment> _fragCache;
#ifdef MEASURE_REPLICATION_TIMING
		vector<unsigned int> _replicationTime[2];
		bool _timer;
#endif
};

#endif
