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
#ifndef SIMULATIONPARMS_H
#define SIMULATIONPARMS_H
#include <guli/uiParms.h>
/**
A collector class for technical parameters of the simulation


	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class simulationParms
{

public:
	simulationParms();
	int main(int argc, char **argv);

	typedef enum {FLAT, PERIODIC, LANDSCAPE} maptypeT;

	unsigned int nTherm() const {return _ntherm; }
	unsigned int nMeas() const 	{return _nmeas;  }
	unsigned int stepsPerMeasurement() const {return _stepsize;	}
	unsigned int nCells() const 	{return _ncells; 	}
	unsigned int nThread() const	{return _nthread;	}

private:
	typedef GULI::uiParms::Variable<int> intvarT;
	typedef GULI::uiParms::Variable<double> doublevarT;
	typedef GULI::uiParms::Variable<string> stringvarT;
	typedef GULI::uiParms::Variable<bool> boolvarT;
	typedef GULI::uiParms::TouchVar touchVarT;
	typedef GULI::uiParms::VariableVector<double> doublevecT;;

	string _parmHeader();
	int _setupWeights(const char *);;
	GULI::uiParms   _parms;
	intvarT	        _ntherm, _stepsize, _ncells, _nthread, _nmeas, _nfork;
	doublevecT      _flowsortparms;
	touchVarT       _overwrite;
	doublevarT      _pprogress, _prelease;
};

#endif
