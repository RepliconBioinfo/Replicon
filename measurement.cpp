/***************************************************************************
 *   Copyright (C) 2007 by Sven Bilke                                      *
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
#include <assert.h>
#include <string>
#include <fstream>
#include <numeric>
#include "measurement.h"
#include "nacentdnasimul.h"
#include "cell.h"
#include "thread.h"

using std::string;
// provide some default implementations for IO streams. It is not published
// in the .h file because it is very closely tight to the implementation of the
// measure class.

class _measureAPI : public measureAPI {
protected:
    friend class measure;
    _measureAPI(thread *t) : measureAPI(t) {}
    virtual ostream &CELL_STREAM() {
        assert(_cstream);
        return *_cstream;
    }
    virtual ostream &SIMULATION_STREAM() {
        assert(0);
        return *_mstream;
    }
    
    virtual ostream &MEAS_STREAM() {
        assert(_mstream);
        return *_mstream;
    }

    virtual void setSimStream(std::ostream *) {return; }
    // the type of output required by each measurement
    enum {CELL = 1, MEASUREMENT = 2, SIMULATION = 4};
    virtual unsigned int outputType() {
        return 0;
    }
    virtual void _perCellOutput(unsigned int i) {
        return;
    }

    virtual void _perCellHeader() {
        return;
    }

    virtual void _measurementHeader() {
        return;
    }

    static std::ofstream *_cstream, *_mstream;
    
  public:
    virtual void writeSimulationResults() {
        return;
    }
    virtual std::string measFnam() {
        return "unspecified";
    }

};


class initiationRate : public _measureAPI {
public:
    initiationRate(thread *t) : _measureAPI(t)  {}

    virtual void measureNonBlocked(void *) {
        const size_t n = cells().size();
        for ( size_t i=0; i < n; i++ ) {
        	cell::rTableType &rates = cells()[i].initiationRate();
        	std::transform(rates.begin(), rates.end(), _rates.begin(), _rates.begin(), std::plus<double>());
        }
        return;
    }

    virtual void prepareMulti(void *) {
    	_rates.resize(cell::nInitiationRateBins);
    	std::fill(_rates.begin(), _rates.end(), 0.);
    }
    virtual void prepareSingle(void *) {
    	_Grates = new vector<double>(cell::nInitiationRateBins,0.);
    }

    virtual void cleanupSingle(void *) {
        delete _Grates;
    }

    virtual void finishMulti(void *) {
       	std::transform(_rates.begin(), _rates.end(), _Grates->begin(), _Grates->begin(), std::plus<double>());
    }


    virtual void finishSingle(void *) {
    	double total = std::accumulate(_Grates->begin(), _Grates->end(),  0.);
    	std::transform(_Grates->begin(), _Grates->end(),  _Grates->begin(), std::bind2nd(std::divides<double>(), total));
    }

    measureAPI *clone(thread *t) {
        _measureAPI *r=new initiationRate(t);
        return r;
    }


    unsigned int outputType() {
        return SIMULATION;
    }

    virtual void writeSimulationResults() {
        SIMULATION_STREAM() << "#SPhase-Fraction\tFraction\n";
        for(size_t i=0; i < _Grates->size(); ++i) {
        	double t = (i + 0.5) / cell::nInitiationRateBins;
        	SIMULATION_STREAM() << t << "\t" << (*_Grates)[i] << "\n";
        }
        return;
    }

    virtual std::string measFnam() { return "initiationRates"; }

    virtual void setSimStream(std::ostream *s) { _simstream = s; }

    virtual std::ostream &SIMULATION_STREAM()  { assert(_simstream); return (*_simstream); }

  private:
    cell::rTableType _rates;
    static  vector<double>  *_Grates;
    static std::ostream *_simstream;
};


// Simulate the method used by Gilbert and others to measure timing by "CGH" analysis
class cghNTiming : public _measureAPI {
public:
	cghNTiming(thread *t) : _measureAPI(t) {}

	measureAPI *clone(thread *t) {
        _measureAPI *r=new cghNTiming(t);
        return r;
    }


    virtual void measureNonBlocked(void *) {
        const unsigned int N = cells().size();
        for(int i=0; i < _NTIMEPOINTS; ++i) {
        	std::fill(_T[i].begin(), _T[i].end(), 0.);
        }
        std::fill(_CELLFRACTION.begin(), _CELLFRACTION.end(), 0);

        for (unsigned int i=0; i < N; i++ ) {
        	const double sIndex = cells()[i].dnaIndex();
        	for(int j =0; j < _NTIMEPOINTS; ++j){
        		if(sIndex >= _cutoff[j] && sIndex < _cutoff[j+1]) {
            		++_CELLFRACTION[j];
                   	for ( unsigned int k=0; k < cell::chromosomeLength ( 0 ); ++k ) {
                            if(cells()[i].chromosome(0).isReplicated(k)) ++_T[j][k];
                        }
            	}
        	}
        }
        return;
    }


    virtual void measureBlocked(void *) {
        for(int i=0; i < _NTIMEPOINTS; ++i) {
        	std::transform ( _T[i].begin(), _T[i].end(), _GT[i].begin(), _GT[i].begin(), std::plus<long int>() );
        	_GCELLFRACTION[i] += _CELLFRACTION[i];
        }
        return;
    }

    virtual void prepareMulti(void *) {
        for(int i=0; i < _NTIMEPOINTS; ++i)
        	_T[i].resize ( cell::chromosomeLength ( 0 ) );

        _CELLFRACTION.resize(_NTIMEPOINTS);
    }

    virtual void prepareSingle(void *) {
        for(int i=0; i < _NTIMEPOINTS; ++i) {
        	_GT[i].resize ( cell::chromosomeLength ( 0 ) );
        	std::fill ( _GT[i].begin(), _GT[i].end(), 0 );
        }
        _GCELLFRACTION.resize(_NTIMEPOINTS);
        std::fill(_GCELLFRACTION.begin(), _GCELLFRACTION.end(), 0);
    }

    virtual void finishSingle(void *) {}

    virtual unsigned int outputType() {
        return SIMULATION;
    }

    virtual void writeSimulationResults() {
        const unsigned int N = cell::chromosomeLength ( 0 );
        SIMULATION_STREAM() << "pos";

        SIMULATION_STREAM() << "# CellFraction: ";
        for(int i=0; i < _NTIMEPOINTS; ++i) {
                 SIMULATION_STREAM() << "\t" << "[" << _cutoff[i] << "," << _cutoff[i+1] << "] " << _GCELLFRACTION[i];
                 SIMULATION_STREAM() << "\n";
      }

        for(int i=0; i < _NTIMEPOINTS; ++i)
            SIMULATION_STREAM() << "\tT" << i;
        SIMULATION_STREAM() << "\n";

        for (unsigned int i=0 ; i < N; ++i ) {
            SIMULATION_STREAM() << i;
            for(int j=0; j < _NTIMEPOINTS; ++j) {
            	double r = _GCELLFRACTION[j] ? (double) _GT[j][i] / _GCELLFRACTION[j] : 0.;
                SIMULATION_STREAM() << "\t" <<  r;
            }
            SIMULATION_STREAM() << "\n";
        }
        return;
    }

    virtual std::string measFnam() { return ("CGHnTiming"); }
    virtual void setSimStream(std::ostream *s) { _simstream = s; }
    virtual std::ostream &SIMULATION_STREAM()  { assert(_simstream); return (*_simstream); }


protected:
    friend vector<double> &cghNcutoff();
    friend void initDefaultCghNCutoff();
//    static const int _NTIMEPOINTS = 5;
    static const int _NTIMEPOINTS = 6;
    vector<long int> _T[_NTIMEPOINTS], _CELLFRACTION;
    static vector<long int>  _GT[_NTIMEPOINTS], _GCELLFRACTION;
    static std::ostream *_simstream;
    static vector<double> _cutoff;
};

vector<double> &cghNcutoff() { return cghNTiming::_cutoff; }

void initDefaultCghNCutoff() {
	vector<double> &c = cghNcutoff();
	c.resize(cghNTiming::_NTIMEPOINTS + 1, 0.);
	c[cghNTiming::_NTIMEPOINTS] = 1.;
	for(int i=0; i < cghNTiming::_NTIMEPOINTS; ++i) {
		c[i] = i * 1. / cghNTiming::_NTIMEPOINTS;
	}
}

typedef std::ofstream *dataStream;

void measure::prepareSingle(void *arg) {
  dataStream &_cstream(_measureAPI::_cstream);  
  dataStream &_mstream(_measureAPI::_mstream);  
  // Open files and send message to child-objects
    const unsigned int N = _M.size();
    unsigned int O = outputType();
    _cstream =  _mstream = 0;

    if (O & _measureAPI::CELL) {
        string fnam = _fnam("percell");
        _cstream = new std::ofstream(fnam.c_str());
        assert(*_cstream);
        *_cstream << _header();
        bool need_sep = false;
        for (unsigned int i = 0; i < N; ++i) {
            _M[i]->_cstream = _cstream;
            if (_M[i]->outputType() & _measureAPI::CELL) {
                if (need_sep) *_cstream << "\t";
                _M[i]->_perCellHeader();
                need_sep = true;
            }
        }
        *_cstream << std::endl;
    }

    if (O & _measureAPI::MEASUREMENT) {
        string fnam = _fnam("permeas");
        _mstream = new std::ofstream(fnam.c_str());
        assert(*_mstream);
        *_mstream << _header();
        for (unsigned int i = 0; i < N; ++i) {
            _M[i]->_mstream = _mstream;
            bool need_sep = false;
            if (_M[i]->outputType() & _measureAPI::MEASUREMENT) {
                if (need_sep) *_cstream << "\t";
                _M[i]->_measurementHeader();
                need_sep = true;
            }
        }
        *_mstream << std::endl;
    }

    for (unsigned int i=0; i < N; ++i) {
        if (_M[i]->outputType() & _measureAPI::SIMULATION) {
            string fnam = _fnam(_M[i]->measFnam().c_str());
            _M[i]->setSimStream(new std::ofstream(fnam.c_str()));
            _M[i]->SIMULATION_STREAM() << _header();
        }
    }
    _callAll(&measureAPI::prepareSingle,arg);
}

// Close files
void measure::cleanupSingle(void *arg) {
  dataStream &_cstream(_measureAPI::_cstream);  
  dataStream &_mstream(_measureAPI::_mstream);  
  _callAll(&measureAPI::cleanupSingle,arg);
  if (_cstream) _cstream->close();
  if (_mstream) _mstream->close();
 // delete &_cstream;
 // delete &_mstream;
  const unsigned int N = _M.size();
  for (unsigned int i=0; i < N; ++i) {
    if (_M[i]->outputType() & _measureAPI::SIMULATION) {
      std::ofstream *s = dynamic_cast<std::ofstream *> (&_M[i]->SIMULATION_STREAM());
      assert(s);
      s->close();
      delete s;
    }
  }
}


void measure::finishSingle(void *arg) {
    _callAll(&measureAPI::finishSingle,arg);
    const unsigned int N = _M.size();
    for (unsigned int i=0; i < N; ++i) {
        if (_M[i]->outputType() & _measureAPI::SIMULATION) {
            _M[i]->writeSimulationResults();
        }
    }
}

// Inject EOL in continuous streams.
void measure::measureBlocked(void *arg) {
  dataStream &_cstream(_measureAPI::_cstream);  
  dataStream &_mstream(_measureAPI::_mstream);  
  _callAll(&measureAPI::measureBlocked,arg);
    if (_cstream) {
        const unsigned int N = cells().size();
        const unsigned int M = _M.size();
        for (unsigned int i=0; i < N; ++i) {
            bool need_sep = false;
            for (unsigned int j=0; j < M; ++j) {
                if (_M[j]->outputType() & _measureAPI::CELL) {
                    if (need_sep) *_cstream << "\t";
                    need_sep = true;
                    _M[j]->_perCellOutput(i);
                }
            }
            *_cstream << std::endl;
        }
    }
    if (_mstream) *_mstream << std::endl;
    return;
}

std::string measure::_header() {
    string r = "# " + _hdr;
    r += "\n";
    return (r);
}

std::string measure::_fnam(const char *i) {
    string r = _prefix + "."+i+".csv";
    return (r);
}


void measure::_callAll(method m, void *arg) {
    const unsigned int N = _M.size();
    for (unsigned int i=0; i < N; ++i) (_M[i]->*m)(arg);
}


std::vector<cell> & measureAPI::cells() {
    return (_myThread->cells());
}


measureAPI *measure::clone(thread *t) {
    const unsigned int N = _M.size();
    measure *r = new measure(t);
    r->_M.resize(N);
    for (unsigned int i=0; i < N; ++i)
        r->_M[i] = dynamic_cast<_measureAPI *>(_M[i]->clone(t));
    r->_prefix = _prefix;
    return (r);
}

unsigned int measure::outputType() {
    const unsigned int N = _M.size();
    unsigned int o = 0;
    for (unsigned int i=0; i < N; ++i) o |= _M[i]->outputType();
    return (o);
}

void measure::enable(measure::builtin b) {
    switch (b) {

    case CGHTIMING:
        _M.push_back(new cghNTiming(0));
        break;

    case INITIATIONRATE:
         _M.push_back(new initiationRate(0));
         break;

    default:
        std::cerr << "Internal error in measure::enable";
        exit(-1);
    }
    return;
}

measure::~measure() {
    const unsigned int N = _M.size();
    for (unsigned int i=0; i < N; ++i) {
        delete _M[i];
    }
    return;
}

vector<double> cghNTiming::_cutoff;
vector<double>  *initiationRate::_Grates;
vector<long int> cghNTiming::_GT[cghNTiming::_NTIMEPOINTS], cghNTiming::_GCELLFRACTION;
std::ofstream *_measureAPI::_cstream, *_measureAPI::_mstream;
std::ostream   *initiationRate::_simstream, *cghNTiming::_simstream;
std::string measure::_hdr = "USELESS HEADER";
