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
#ifndef THREAD_H
#define THREAD_H
#include <vector>
#include <pthread.h>
#include "threadvariables.h"
#include "cell.h"

class threadControl;
class measureAPI;

/**
	@author Sven Bilke <bilkes@mail.nih.gov>
*/
class thread : public threadVariables {
public:
    thread(measureAPI &);
    virtual ~thread();
    void timeSteps ( unsigned int s );
    void threadEntryPoint();
    virtual void measure() {
        return;
    }

    vector<cell> &cells() { return _cells; }
protected:
    void _synchronizeThreads(int &);
    virtual void _lastThreadSyncing() {
        return;
    }
    virtual void prepareSimulation() {
        return;
    } // If a simulation needs some preparation,     re-implement
    virtual void postThermHook() {
        return;
    }
    virtual void postRunHook()   {
        return;
    }

    void _requestAbort() {
        _abort = true;
        return;
    }
    bool _abortRequested() {
        return _abort;
    }

    std::vector<cell> _cells;
    volatile bool _abort;
    void *_dummy;
    measureAPI &_measure;

private:
    enum {GENERIC, INIT_MEAS, FINISH_MEAS, CLEANUP};
    unsigned int _syncstate;
};

#endif
