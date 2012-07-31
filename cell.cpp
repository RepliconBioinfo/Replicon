/***************************************************************************
 *   Copyright (C) 2006 by Sven Bilke                                      *
 *   bilkes@mail.nih.gov                                                   Sven *
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
#include "cell.h"
#include "dna.h"
#include "replicator.h"


float cell::_penters = 0.01;
unsigned int    cell::_nchromosomes = 1;
unsigned int    cell::_nreplicator  = 100;
float cell::_induceAttempt = 5;
cell::pTableSetType cell::_pinitiate;
cell::pTableSetType cell::_pprogress;
cell::pTableSetType cell::_prelease;

/*!
    \fn cell::setNChromosomes(unsigned int n)
 */
void cell::setNChromosomes ( unsigned int n )
{
	_nchromosomes = n;
	_pinitiate.resize ( n );
	_pprogress.resize ( n );
	_prelease.resize ( n );
}

/*!
    \fn cell::setChromosomeSize(unsigned int c, unsigned int s)
 */
void cell::setChromosomeLength ( unsigned int chr, unsigned int s )
{
	pInitiate ( chr ).resize ( s );
	pProgress ( chr ).resize ( s );
	pRelease ( chr ).resize ( s );
}

/*!
    \fn cell::chromosomeSize(unsigned int)
 */
unsigned int cell::chromosomeLength ( unsigned int chr )
{
	return pInitiate ( chr ).size();
}

cell::cell ( threadVariables &t ) : threadVariablesClient ( t ), 
              _replicator ( _nreplicator, replicator ( t ) ), _g2 ( true ), _lenSPhase ( 0 )
{


	for ( unsigned int c=0; c < nChromosomes(); c++ )
	{
		_dna.push_back ( dna ( c ) );
	}

#ifdef MEASURE_REPLICATION_TIMING
	const unsigned int n = nChromosomes();
	_replicationTime.resize ( n );
	for ( unsigned int c=0; c < n; c++ )
	{
		replicationTime ( c ).resize ( chromosomeLength ( c ) );
		std::fill ( replicationTime ( c ).begin(), replicationTime ( c ).end(), 0 );
	}
#endif

	_idleRep.resize ( nReplicator() );
	return;
}


cell::~cell()
{
	return;
}


/*!
    \fn cell::timeStep()
 */
void cell::timeStep()
{
	if ( inG2Phase() )
	{
		if ( pEnterS() >= ran() ) enterSPhase();
		return;
	}
	++_curSPhase;

	unsigned int idle = 0;
	for ( unsigned int r=0; r < nReplicator(); r++ )
	{
		replicator &cur = _replicator[r];
		if ( cur.isAttached() )
		{
			cur.step ( _curSPhase );
		}
		else
		{
			_idleRep[idle++] = &cur;
		}
	}

	const int attempts = static_cast<int> ( idle * _induceAttempt + 0.5 );
	int cur = 0;
	for ( unsigned int i=0; ( i < attempts ) && ( cur < idle-1 ); i++ ) {
		const unsigned int c =  nChromosomes() > 1 ? ranN ( nChromosomes() ) : 0;
		dna &d = _dna[c];
		const int l = ranN ( d.length() );
		if ( ( !d.isReplicated ( l ) ) && ( ran() <= pInitiate ( c ) [l] ) )
		{
			d.replicate ( l, _curSPhase );
//			bool o = ( ran() >= 0.5 ); // Turn this one if you want to allow uni-directional replicators
			const bool o = true;
			_idleRep[cur++]->initiate ( d,l,o );
			if ( cur >= idle ) break;
			_idleRep[cur++]->initiate ( d,l,!o );
		}
	}
	bool finishedS = true;
	for ( unsigned int d=0; finishedS && ( d < nChromosomes() ); d++ ) finishedS = finishedS && _dna[d].complete();
	if ( finishedS ) enterG2Phase();
	return;
}

static const dna::fragment _none={-1,-1,-1};

const dna::fragment &cell::nacentDna ( int i ) const
{
	for (unsigned  int j=0; j < nChromosomes(); j++ )
	{
		dna &cur=chromosome ( j );
		const int n = cur.nFragment();
		if ( i < n ) return cur.nacentDna ( i );
		i -= n;
	}
	return _none;
}

unsigned int cell::nFragment() const
{
	unsigned int n = 0;
	for (unsigned  int j=0; j < nChromosomes(); j++ ) n += chromosome ( j ).nFragment();
	return n;
}



/*!
    \fn cell::enterSPhase()
 */
void cell::enterSPhase()
{
	for ( unsigned int d=0; d < nChromosomes(); d++ ) _dna[d].enterSPhase();
	_g2 = false;
	_curSPhase = 0;
	return;
}


/*!
    \fn cell::enterSPhase()
 */
void cell::enterG2Phase()
{
	_g2 = true;
	_lenSPhase = _curSPhase;
#ifdef MEASURE_REPLICATION_TIMING
	for ( unsigned int i=0; i < nChromosomes(); i++ )
	{
		_replicationTime[i] = chromosome ( i ).replicationTime();
	}
#endif
	return;
}




/*!
    \fn cell::replicated() const
 */
float cell::dnaIndex() const
{
	int done=0;
	int length=0;
	for ( unsigned int d=0; d < nChromosomes(); d++ )
	{
		done    += _dna[d].replicated();
		length  += _dna[d].length();
	}
	return static_cast<float> ( done ) / static_cast<float> ( length );
}

/*!
    \fn cell::replicated() const
 */
float cell::replicatorIndex() const
{
	int active=0;
	for ( unsigned int r=0; r < nReplicator(); r++ )
	{
		active   += _replicator[r].isAttached() ? 1 : 0;
	}
	return static_cast<float> ( active ) / static_cast<float> ( nReplicator() );
}




