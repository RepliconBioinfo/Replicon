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
#include <math.h>
#include <functional>
#include <numeric>
#include <fstream>
#include <iostream>
#include <assert.h>
#include "measurement.h"
#include "cell.h"
#include "threadcontrol.h"
#include "nacentdnasimul.h"
#include "dataBasic.h"
#include "dataIO.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "uiParms.h"
simulationParms PARM;


simulationParms::simulationParms() :
	  _ntherm("ntherm",       "Number of thermalization sweeps", 80000, _parms),
	  _stepsize("stepsize",   "Sweeps per measurement", 20000, _parms),
	  _ncells("ncells",       "Number of cells", 1000, _parms),
	  _nthread("threads",     "Number of threads", 1, _parms),
	  _nmeas("nmeas",         "Number of measurements", 2000, _parms),
	  _nfork("nfork",         "Number of replication forks", 50, _parms),
	  _flowsortparms("flowsorter", "Comma-separated list of flowsort boundaries", _parms),
	  _overwrite("overwrite", "Overwrite result file", _parms),
	  _pprogress("pprogress", "Probability to move replication fork (ignored if 2nd column is present in landscape file)", 1., _parms),
	  _prelease("prelease",   "Probability to release replication fork (ignored if 3nd column is present in landscape file)", 0., _parms),
	  _kinetic("singlemolecule", "Simulate single molecule experiments", _parms),
	  _hybridize("nacent", "Simulate nacent strand measurements ala Valenzuela et. al", _parms),
	  _indices("indices", "Measure S-phase and replicator engagement fraction", _parms),
	  _timing("timing", "Perform direct timing measurements", _parms),
	  _initrate("initrate", "Measure the global initiation rate as a function of S-phase fraction", _parms)
{}


int simulationParms::_setupWeights(const char *init)
{

	GULI::dataBasic buffer;
	if(GULI::IO::read(init, buffer)) return -1;

	cell::setNChromosomes( 1 );
	cell::setChromosomeLength ( 0, buffer.rows());
	std::copy(buffer.beginCol(0), buffer.endCol(0), cell::pInitiate(0).begin());

	if(buffer.cols() > 1)
		std::copy(buffer.beginCol(1), buffer.endCol(1), cell::pProgress(0).begin());
    else
    	std::fill(cell::pProgress(0).begin(), cell::pProgress(0).end(), _pprogress);

	if(buffer.cols() > 2)
		std::copy(buffer.beginCol(2), buffer.endCol(2), cell::pProgress(0).begin());
    else
    	std::fill(cell::pRelease(0).begin(), cell::pRelease(0).end(), _prelease);

	return 0;
}


string simulationParms::_parmHeader() {
	char buffer[1024];
	sprintf(buffer, "ntherm=%d stepsize=%d, ncells=%d nthread=%d nmeas=%d nfork=%d pprogress=%f pprogress=%f",
			             (int) _ntherm, (int) _stepsize, (int) _ncells, (int) _nthread, (int) _nmeas, (int) _nfork, (float) _pprogress, (float) _prelease);
	return buffer;

}

extern void initDefaultCghNCutoff();
extern vector<double> &cghNcutoff();

int simulationParms::main ( int argc, char **argv ) {
	int r = _parms.commandLine(argc, argv);
	argc -= r;
	argv += r;
	if ((r < 0) or (argc < 3)) {
		std::cerr << "Usage: " << argv[0] << " [options] landscape out-prefix\n" << std::endl;
		std::cerr << "Known options are" << std::endl;
		std::cerr << _parms.describeParms() << std::endl;
		return (-1);
	}

	const char *init = argv[1];
	const char *ofil = argv[2];


	if(_setupWeights(init)) {
		std::cerr << "Can not read landscape file " << init << std::endl;
		exit(-1);
	}
	cell::nReplicator(_nfork);

	measure M(0);

	if(_indices) M.enable(measure::INDICES);
	if(_kinetic) M.enable(measure::KINETIC);
	if(_hybridize) M.enable(measure::HYBRIDIZE);
	if(_timing) M.enable(measure::TIMING);
	if(_initrate) M.enable(measure::INITIATIONRATE);
	M.enable(measure::CGHTIMING);
	initDefaultCghNCutoff();


    if(_flowsortparms.size()) {
    	if(_flowsortparms.size() != cghNcutoff().size()) {
    		std::cerr << "Incorrect number of boundaries. This binary supports " <<  cghNcutoff().size() -1  << " flowsort groups" << std::endl;
    		return -1;
    	}
    	cghNcutoff() = _flowsortparms;
    }

    M.setResultPrefix(ofil);
    string header = _parmHeader() + " " ;
    header +=  init;
    M.setHeader(header);
    for ( unsigned int i=0; i < simulationParms::nThread(); i++ ) {
        threadControl::addThread ( * new thread(M) );
    }
    threadControl::startThreads();
    threadControl::waitForThreads();


    for ( unsigned int i=0; i < threadControl::activeThreads(); i++ ) {
        delete threadControl::getThread ( i );
    }

    return (EXIT_SUCCESS);
}


int main ( int argc, char *argv[] ) {
    return PARM.main(argc, argv);
}
