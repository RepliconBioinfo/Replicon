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
#include <iostream>
#include <stdlib.h>
#include "nacentdnasimul.h"
#include "threadcontrol.h"
#include "thread.h"
#include "measurement.h"


static  int sync1 = 0, sync2 = 0;
extern simulationParms PARM;

thread::thread(measureAPI &m) : _measure(*m.clone(this)) {
    int R1  = rand(), R2 = rand(), R3=rand();
    seed ( R1, R2, R3 );
    _dummy = malloc ( 1024 ); // make sure we get some space to separate cache-lines
    return;
};

thread::~thread() {
    free ( _dummy );
    delete &_measure;
}

void thread::threadEntryPoint() {
    _abort = false;
    int n = PARM.nCells() / threadControl::activeThreads();
    _cells.resize ( n, cell ( *dynamic_cast<threadVariables *> ( this ) ) );


    prepareSimulation();
    // Initialize measurement subsystem
    _syncstate = INIT_MEAS;
    _synchronizeThreads(sync1);
    _syncstate = GENERIC;
    _measure.prepareMulti();

    timeSteps ( PARM.nTherm() );
    postThermHook();

    for ( unsigned i = 0; ( i < PARM.nMeas() ) && !_abortRequested(); i++ ) {
        timeSteps ( PARM.stepsPerMeasurement() );

        //Measurements
        _measure.measureNonBlocked();
        threadControl::block();
        _measure.measureBlocked();
        threadControl::release();
    }

    // Signal "finish" to  Measurement subsystem
    threadControl::block();
    _measure.finishMulti();
    threadControl::release();
    _syncstate = FINISH_MEAS;
    _synchronizeThreads(sync2);

    postRunHook();

    threadControl::block();
    _measure.cleanupMulti();
    threadControl::release();
    _syncstate = CLEANUP;
    _synchronizeThreads(sync1);
    _syncstate = GENERIC;
    return;
}


void thread::timeSteps ( unsigned int s ) {
    const unsigned int ncells = _cells.size();
    for ( unsigned int  n=0; ( n < s ) && !_abortRequested(); ++n )
        for ( unsigned int i=0; i < ncells; ++i )
            _cells[i].timeStep();
};

void thread::_synchronizeThreads(int &control) {
    threadControl::block();
    unsigned int me = ++control;
    threadControl::release();
    if ( me == threadControl::activeThreads() ) {
        switch(_syncstate) {
        	case CLEANUP: 	  _measure.cleanupSingle(); break;
            case FINISH_MEAS: _measure.finishSingle(); break;
            case INIT_MEAS:   _measure.prepareSingle(); break;
            case GENERIC:     _lastThreadSyncing(); break;
        }
        control = 0;
        threadControl::wakeUp();
    } else {
        do {
            threadControl::sleep();
        } while ( control );
    }
    return;
}
