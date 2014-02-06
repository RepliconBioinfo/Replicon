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
#ifndef REPLICATOR_H
#define REPLICATOR_H
#include "threadvariablesclient.h"
#include "config.h"
class dna;


/**
This object simulates the replication machinery

	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class replicator : protected threadVariablesClient
{
	public:
		replicator ( threadVariables & );

		~replicator();

		bool isAttached() const
		{
			return _attached;
		}
		void initiate ( dna &, int position, bool orientation );

		dna &Dna() const { return *_dna; }

		int position() const {return _position; }
		void step ( unsigned int t );
	protected:
		void _attach ( int pos, bool orient );
		void _release();

	private:
		bool _attached;
		unsigned char _chr;
		char _orientation;
		int _position;
		dna  *_dna;
};

#endif
