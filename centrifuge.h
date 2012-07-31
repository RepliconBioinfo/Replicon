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
#ifndef CENTRIFUGE_H
#define CENTRIFUGE_H
#include <vector>
#include "dna.h"
#include "threadvariablesclient.h"
class cell;

/**
Simulates the behavior of a centrifuge by preferentially selecting short bubble-sequence
It behaves like a wrapper around "cell" in that it filters out fragments from the DNA presented
by the cell.
	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class centrifuge : protected threadVariablesClient
{
	public:
		centrifuge ( threadVariables &t );
		virtual ~centrifuge();
		void source ( cell & );
		const dna::fragment &nacentDna ( int i ) const;
		unsigned int nFragment() const        { return _n; }
		virtual bool accept ( const dna::fragment & ) =0;
	private:
		vector<int> _fragment;
		unsigned int _n;
		cell *_cell;
};

class cutoffCentrifuge : public centrifuge
{
	public:
		cutoffCentrifuge ( cell &c, unsigned int k, threadVariables &t ) : centrifuge ( t ), _cutoff ( k ) {source ( c );}
		cutoffCentrifuge ( threadVariables &t ) : centrifuge ( t ) {}
		void setCutoff ( unsigned int c ) {_cutoff = c; }
		virtual bool accept ( const dna::fragment &f );
	private:
		unsigned int _cutoff;
};



class gaussCentrifuge : public centrifuge
{
	public:
		gaussCentrifuge ( cell &cl, unsigned int c, unsigned int v, threadVariables &t ) :
				centrifuge ( t ), _center ( ( float ) c ), _variance ( ( float ) v ) {source ( cl );}
		gaussCentrifuge ( threadVariables &t ) : centrifuge ( t ) {}
		void setCenter ( unsigned int c )   {_center = c; }
		void setVariance ( unsigned int v ) {_variance = v; }
		unsigned int center() { return static_cast<unsigned int> ( _center ); }
		unsigned int variance() { return static_cast<unsigned int> ( _variance ); }
		virtual bool accept ( const dna::fragment &f );
	private:
		float _center, _variance;
};

#endif
