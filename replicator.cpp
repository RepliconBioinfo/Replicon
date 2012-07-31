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
#include "replicator.h"
#include "threadvariablesclient.h"
#include "dna.h"
#include "cell.h"

replicator::replicator ( threadVariables &t ) : threadVariablesClient ( t ), _attached ( false ) {}


replicator::~replicator() {}


/*!
    \fn replicator::attach(dna &)
 */
void replicator::initiate ( dna &d, int position, bool orientation )
{
	_dna = &d;
	_chr  = d.chromosomeId();
	_attach ( position, orientation );
	return;
}

void replicator::step ( unsigned int t )
{
	if ( (cell::pRelease( _chr ) [position() ] > 0.) &&
		 (ran() <= cell::pRelease ( _chr ) [position() ]) )
	{
		_release();
		return;
	}
	if ( (cell::pProgress ( _chr ) [position() ] < 1.) &&
		  (ran() >= cell::pProgress ( _chr ) [position() ] ))
		return;

	int newPos = position() + _orientation;
	if ( ( newPos >= Dna().length() ) || ( newPos < 0 ) || Dna().isReplicated ( newPos ) )
	{
		_release();
		return;
	}
	Dna().replicate ( newPos, t );
	_position = newPos;
}

void replicator::_attach ( int p, bool o )
{
	_position = p;
	_orientation = o ? 1 : -1;
	_attached=true;
	return;
}

void replicator::_release()
{
	_attached = false;
	return;
}
