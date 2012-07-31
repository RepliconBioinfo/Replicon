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
#ifndef THREADVARIABLESCLIENT_H
#define THREADVARIABLESCLIENT_H
#include "threadvariables.h"

//class threadVariables;

/**
	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class threadVariablesClient
{
	public:
		threadVariablesClient ( threadVariables & );
		~threadVariablesClient();

		double gauss() const    { return _tr->gauss(); }
		double ran() const      { return _tr->ran(); }
		int ranN ( int max ) const { return _tr->ranN ( max ); }

	protected:
		threadVariables *_tr;
};

#endif
