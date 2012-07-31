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
//#include <ostream>
#include "measurement.h"
#include "centrifuge.h"
#include "microarray.h"
#include "kineticmeas.h"
#include "nacentdnasimul.h"
#include "cell.h"
#include "thread.h"

using std::string;
//flow-not-mentioned
//measure::flowSortT measure::flowsorter = { 0.05, 0.35, 0.55, 0.95 };
//flow1
//measure::flowSortT measure::flowsorter = { 0.05, 0.25, 0.65, 0.98 };
//flow2
measure::flowSortT measure::flowsorter = { 0.05, 0.50, 0.75, 0.98 };

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

class hybridize : public _measureAPI {
public:
    hybridize(thread *t) : _measureAPI(t), _Z ( *static_cast<threadVariables *>(t) ) {
        _Z.setCutoff ( 80 ) ;
    }

    virtual void measureNonBlocked(void *) {
        const unsigned int n = cells().size();
        for ( unsigned int i=0; i < n; i++ ) {
            _Z.source ( cells()[i] );
            _array.hybridize ( _Z );
        }
        return;
    }

    virtual void prepareSingle(void *) {
        _Garray = new microArray;
        _Garray->clear();
    }

    virtual void cleanupSingle(void *) {
        delete _Garray;
    }

    virtual void finishMulti(void *) {
        *_Garray += _array;
    }


    measureAPI *clone(thread *t) {
        _measureAPI *r=new hybridize(t);
        return r;
    }

    unsigned int outputType() {
        return SIMULATION;
    }

    virtual void writeSimulationResults() {
        SIMULATION_STREAM() << *_Garray;
        return;
    }

    virtual std::string measFnam() { return "hybridize"; } 

    virtual void setSimStream(std::ostream *s) { _simstream = s; }
    virtual std::ostream &SIMULATION_STREAM()  { assert(_simstream); return (*_simstream); }
    
  private:
    microArray _array;
    cutoffCentrifuge _Z;
    static microArray *_Garray;
    static std::ostream *_simstream;
};


class indices : public _measureAPI {
public:
    indices(thread *t) : _measureAPI(t) {}

    measureAPI *clone(thread *t) {
        _measureAPI *r=new indices(t);
        return r;
    }

    virtual unsigned int outputType() {
        return CELL;
    }

    virtual void _perCellHeader() {
        *_cstream << "lenSPhase\treplicatorIndex\tdnaIndex";
    }

    virtual void _perCellOutput(unsigned int i) {
        CELL_STREAM() <<  cells()[i].lenSPhase() << "\t" << cells()[i].replicatorIndex() << "\t" <<
        cells()[i].dnaIndex();
    }
};


class timing : public _measureAPI {
public:
    timing(thread *t) : _measureAPI(t) {}
    measureAPI *clone(thread *t) {
        _measureAPI *r=new timing(t);
        return r;
    }


    virtual void measureNonBlocked(void *) {
        const unsigned int n = cells().size();
        std::fill ( _t.begin(), _t.end(), 0. );
        std::fill ( _t2.begin(), _t2.end(), 0. );
        for ( unsigned int i=0; i < n; i++ ) {
            for ( unsigned int j=0; j < cell::chromosomeLength ( 0 ); ++j ) {
                double t =  static_cast<double> ( cells()[i].replicationTime ( 0 ) [j] );
                _t[j] += t;
                _t2[j] += t * t;
            }
        }
        double nu = static_cast<const double> ( n );
        std::transform ( _t.begin(), _t.end(), _t.begin(), std::bind2nd ( std::divides<double>(), nu ) );
        std::transform ( _t2.begin(), _t2.end(), _t2.begin(), std::bind2nd ( std::divides<double>(), nu ) );
        std::transform ( _t.begin(), _t.end(), _t2.begin(), _t2.begin(), mkVar() );
        return;
    }


    virtual void measureBlocked(void *) {
        std::transform ( _t.begin(), _t.end(), gt.begin(), gt.begin(), std::plus<double>() );
        std::transform ( _t2.begin(), _t2.end(), gt2.begin(), gt2.begin(), std::plus<double>() );
        ++nMeas;
        return;
    }

    virtual void prepareMulti(void *) {
        _t.resize ( cell::chromosomeLength ( 0 ) );
        _t2.resize ( cell::chromosomeLength ( 0 ) );
    }


    virtual void prepareSingle(void *) {
        gt.resize ( cell::chromosomeLength ( 0 ) );
        gt2.resize ( cell::chromosomeLength ( 0 ) );
        std::fill ( gt.begin(), gt.end(), 0. );
        std::fill ( gt2.begin(), gt2.end(), 0. );
        nMeas = 0;
    }

    virtual void finishSingle(void *) {
        double nu = static_cast<double>(nMeas);
        std::transform ( gt.begin(),  gt.end(),  gt.begin(),  std::bind2nd ( std::divides<double>(), nu) );
        std::transform ( gt2.begin(), gt2.end(), gt2.begin(), std::bind2nd ( std::divides<double>(), nu) );
    }

    virtual unsigned int outputType() {
        return SIMULATION;
    }

    virtual void writeSimulationResults() {
        const unsigned int N = cell::chromosomeLength ( 0 );
        SIMULATION_STREAM() << "pos\tAvgT\tVarT" << std::endl;
        for (unsigned int i=0 ; i < N; ++i ) {
            SIMULATION_STREAM() << i << "\t" << gt[i] << "\t" << gt2[i] << std::endl;
        }
        return;
    }

    virtual std::string measFnam() { return ("timing"); }
    virtual void setSimStream(std::ostream *s) { _simstream = s; }
    virtual std::ostream &SIMULATION_STREAM()  { assert(_simstream); return (*_simstream); }

protected:
struct mkVar : public std::binary_function<double, double, double> {
        double operator() ( double av, double av2 ) {
            return av2 - av * av;
        }
    };

    vector<double> _t, _t2;
    static vector<double> gt, gt2;
    static unsigned int nMeas;
    static std::ostream *_simstream; 
};


class cghTiming : public _measureAPI {
public:
	cghTiming(thread *t) : _measureAPI(t) {}
    measureAPI *clone(thread *t) {
        _measureAPI *r=new cghTiming(t);
        return r;
    }


    virtual void measureNonBlocked(void *) {
        const unsigned int N = cells().size();
        std::fill ( _t1.begin(), _t1.end(), 0. );
        std::fill ( _t2.begin(), _t2.end(), 0. );
        for (unsigned int i=0; i < N; i++ ) {
        	const double sIndex = cells()[i].dnaIndex();
        	if(sIndex >= measure::flowsorter.earlyStart && sIndex <= measure::flowsorter.earlyEnd) {
               	for ( unsigned int j=0; j < cell::chromosomeLength ( 0 ); ++j ) {
                        if(cells()[i].chromosome(0).isReplicated(j)) ++_t1[j];
                    }
        	}
        	if(sIndex >= measure::flowsorter.lateStart && sIndex <= measure::flowsorter.lateEnd) {
               	for ( unsigned int j=0; j < cell::chromosomeLength ( 0 ); ++j ) {
                        if(cells()[i].chromosome(0).isReplicated(j)) ++_t2[j];
                    }
        	}
        }
        return;
    }


    virtual void measureBlocked(void *) {
        std::transform ( _t1.begin(), _t1.end(), _gt1.begin(), _gt1.begin(), std::plus<long int>() );
        std::transform ( _t2.begin(), _t2.end(), _gt2.begin(), _gt2.begin(), std::plus<long int>() );
        return;
    }

    virtual void prepareMulti(void *) {
        _t1.resize ( cell::chromosomeLength ( 0 ) );
        _t2.resize ( cell::chromosomeLength ( 0 ) );
    }

    virtual void prepareSingle(void *) {
        _gt1.resize ( cell::chromosomeLength ( 0 ) );
        _gt2.resize ( cell::chromosomeLength ( 0 ) );
        std::fill ( _gt1.begin(), _gt1.end(), 0 );
        std::fill ( _gt2.begin(), _gt2.end(), 0 );
    }

    virtual void finishSingle(void *) {}

    virtual unsigned int outputType() {
        return SIMULATION;
    }

    virtual void writeSimulationResults() {
        const unsigned int N = cell::chromosomeLength ( 0 );
        SIMULATION_STREAM() << "pos\tT1\tT2" << std::endl;
        for (unsigned int i=0 ; i < N; ++i ) {
            SIMULATION_STREAM() << i << "\t" << _gt1[i] << "\t" << _gt2[i] << "\n";
        }
        return;
    }

    virtual std::string measFnam() { return ("CGHtiming"); }
    virtual void setSimStream(std::ostream *s) { _simstream = s; }
    virtual std::ostream &SIMULATION_STREAM()  { assert(_simstream); return (*_simstream); }

protected:
    vector<long int> _t1, _t2;
    static vector<long int>_gt1, _gt2;
    static std::ostream *_simstream;
};
vector<long int> cghTiming::_gt1, cghTiming::_gt2;
std::ostream *cghTiming::_simstream;


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
    static const int _NTIMEPOINTS = 5;
    vector<long int> _T[_NTIMEPOINTS], _CELLFRACTION;
    static vector<long int>  _GT[_NTIMEPOINTS], _GCELLFRACTION;
    static std::ostream *_simstream;
    static vector<double> _cutoff;
};
vector<long int> cghNTiming::_GT[cghNTiming::_NTIMEPOINTS], cghNTiming::_GCELLFRACTION;
vector<double>   cghNTiming::_cutoff;
std::ostream *cghNTiming::_simstream;

vector<double> &cghNcutoff() { return cghNTiming::_cutoff; }

void initDefaultCghNCutoff() {
	vector<double> &c = cghNcutoff();
	c.resize(cghNTiming::_NTIMEPOINTS + 1, 0.);
	c[cghNTiming::_NTIMEPOINTS] = 1.;
	for(int i=0; i < cghNTiming::_NTIMEPOINTS; ++i) {
		c[i] = i * 1. / cghNTiming::_NTIMEPOINTS;
	}
}



class kinetic : public _measureAPI {
public:
    kinetic(thread *t) : _measureAPI(t) {}
    measureAPI *clone(thread *t) {
        _measureAPI *r=new kinetic(t);
        return r;
    }

    virtual unsigned int outputType() {
        return CELL;
    }

    virtual void _perCellHeader() {
        *_cstream << "time\teye\teye2eye\thole\tnFragment";
    }

    virtual void _perCellOutput(unsigned int i) {
        kineticMeas M(cells()[i]);
        CELL_STREAM() << cells()[i].timeSPhase() << "\t" << M.eye() << "\t" << M.eye2eye() 
                      << "\t" << M.hole() << "\t" << cells()[i].nFragment();
    }
};

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
    case INDICES:
        _M.push_back(new indices(0));
        break;
    case HYBRIDIZE:
        _M.push_back(new hybridize(0));
        break;
    case KINETIC:
        _M.push_back(new kinetic(0));
        break;
    case TIMING:
        _M.push_back(new timing(0));
        break;

    case CGHTIMING:
        _M.push_back(new cghNTiming(0));
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

vector<double> timing::gt;
vector<double> timing::gt2;
microArray *hybridize::_Garray;
unsigned int timing::nMeas = 0;
std::ofstream *_measureAPI::_cstream, *_measureAPI::_mstream;
std::ostream  *hybridize::_simstream, *timing::_simstream;
std::string measure::_hdr = "USELESS HEADER";
