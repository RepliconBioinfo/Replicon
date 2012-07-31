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
#ifndef __MEASUREMENT_H
#define __MEASUREMENT_H
#include <vector>
#include <fstream>
using std::vector;

class thread;
class cell;


/** API for measurements called from threads **/
class measureAPI {
  protected:
    measureAPI(thread *t) : _myThread(t) {};
    thread *_myThread;
    vector<cell> & cells(); 

  
  public:
    virtual ~measureAPI() {}
    /** measurements that can be done without blocking **/
    virtual void measureNonBlocked(void *arg=0) { return; };
    /** measurements that require blocking **/
    virtual void measureBlocked(void *arg=0) { return; }

    /** gets called once per thread */
    virtual void prepareMulti(void *arg=0)   { return; }
    /** gets called only once in the LAST thread */
    virtual void prepareSingle(void *arg=0) { return; }

    /** gets called once per thread */
    virtual void finishMulti(void *arg=0)   { return; }
    /** gets called only once in the LAST thread */
    virtual void finishSingle(void *arg=0) { return; }

    /** gets called once per thread */
    virtual void cleanupMulti(void *arg=0)   { return; }
    /** gets called only once in the LAST thread */
    virtual void cleanupSingle(void *arg=0) { return; }

    virtual measureAPI *clone(thread *)= 0 ;
};


class _measureAPI;

class measure : public measureAPI {
public:   
    typedef enum {INDICES, HYBRIDIZE, KINETIC, TIMING, CGHTIMING} builtin;
    void enable(builtin B);

	typedef struct {
		double earlyStart;
		double earlyEnd;
		double lateStart;
		double lateEnd;
	} flowSortT;

	static flowSortT flowsorter;

    measure(thread *t=0) : measureAPI(t), _prefix("output") {}
    void setResultPrefix(const char *p) {_prefix = p; }

    virtual ~measure();
    void setHeader(const std::string &h) {_hdr = h; }

protected:
  virtual void measureNonBlocked(void *arg=0) {_callAll(&measureAPI::measureNonBlocked,arg);}
  virtual void measureBlocked(void *arg=0); 
  virtual void prepareMulti(void *arg=0){_callAll(&measureAPI::prepareMulti,arg);}
  virtual void prepareSingle(void *arg=0);
  virtual void finishMulti(void *arg=0) {_callAll(&measureAPI::finishMulti,arg);}
  virtual void finishSingle(void *arg=0);
  virtual void cleanupMulti(void *arg=0) {_callAll(&measureAPI::cleanupMulti,arg);}
  virtual void cleanupSingle(void *arg=0);
  virtual measureAPI *clone(thread *);
  virtual unsigned int outputType();
  virtual std::string _fnam(const char *);
  virtual std::string _header();

  
  private:
  typedef void (measureAPI::* method)(void *);
  void _callAll(method, void *);
  vector<_measureAPI *> _M;
  std::string _prefix;
  static std::string _hdr;
};

#endif
