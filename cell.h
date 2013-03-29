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
#ifndef CELL_H
#define CELL_H
#include <vector>
#include "threadvariablesclient.h"
#include "dna.h"
#include "replicator.h"
#include "config.h"
using std::vector;



/**
	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class cell : protected threadVariablesClient
{
	public:
		typedef vector<float> pTableType;
		typedef vector<pTableType> pTableSetType;

		cell ( threadVariables &t );
		~cell();
		bool inG2Phase() const {return _g2; }
		bool inSPhase()  const {return !_g2; }
		void timeStep();
		void enterSPhase();
		void enterG2Phase();

        int timeSPhase() { if(inSPhase()) return _curSPhase; return -1; }
// Some static functions used to distribute common parameters to all
// instances of "cell". Note that no instances of cell may exist when changing
// any of the associated parameters, these methods are used within the instance-methods
// without check for changed parameters !

		static float &pEnterS()                  { return _penters; }
		static unsigned int  &nReplicator()      { return _nreplicator; }
		static unsigned int  &nReplicator(unsigned int n)      { return _nreplicator = n; }
		static pTableType &pInitiate ( int chr ) { return _pinitiate[chr]; }
		static pTableType &pProgress ( int chr ) { return _pprogress[chr];  }
		static pTableType &pRelease ( int chr )  { return _prelease[chr];  }
		static unsigned int nChromosomes ()      { return _nchromosomes; }
		static void setNChromosomes ( unsigned int );
		static void setChromosomeLength ( unsigned int c, unsigned int s );
		static unsigned int chromosomeLength ( unsigned int c );

		float dnaIndex() const;
		float replicatorIndex() const;
		unsigned int nFragment() const;
		const dna::fragment &nacentDna ( int ) const;
		dna &chromosome ( int i ) const { return _dna[i]; }
		int lenSPhase() { int  tmp =  _lenSPhase; _lenSPhase = -1; return tmp; }
		int nInitiation()  { int tmp = _nInitiation; _nInitiation = -1; return tmp; }

	protected:
		mutable vector<dna> _dna;
		vector<replicator>  _replicator;

		static float           _penters;
		static unsigned int    _nchromosomes;
		static unsigned int    _nreplicator;
		static float           _induceAttempt;
		static pTableSetType   _pinitiate;
		static pTableSetType   _pprogress;
		static pTableSetType   _prelease;

	private:
		bool _g2;
		int _curSPhase, _lenSPhase, _curNInitiation, _nInitiation;

		vector<replicator *> _idleRep;
#ifdef MEASURE_REPLICATION_TIMING
	public:
		typedef vector<unsigned int > tTableType;
		typedef vector<tTableType> tTableSetType;
		tTableType &replicationTime ( int i )  { return _replicationTime[i]; }
	private:
		tTableSetType _replicationTime;
#endif
};
#endif
