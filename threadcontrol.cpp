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
#include <pthread.h>
#include <algorithm>
#include "threadcontrol.h"
#include "nacentdnasimul.h"


extern "C"
{
	static void *start ( void *threadArg )
	{
		thread *task = reinterpret_cast<thread *> ( threadArg );
		task->threadEntryPoint();
		pthread_exit ( NULL );
		return 0; // we will never get here, just to keep the compiler from complaining
	}
}

/** A few static local variables **/
typedef struct
{
	pthread_t ctrl;
	thread    *tr;
}
control_t;


static vector<control_t> _control;
static pthread_mutex_t _mutex_client;
static pthread_mutex_t _mutex_self;
static pthread_cond_t _cond;

void threadControl::block()   {pthread_mutex_lock ( &_mutex_client );}
void threadControl::release() {pthread_mutex_unlock ( &_mutex_client );}

void threadControl::sleep()
{
	pthread_mutex_lock ( &_mutex_self );
	pthread_cond_wait ( &_cond, &_mutex_self );
	pthread_mutex_unlock ( &_mutex_self );
	return;
}

void threadControl::wakeUp()
{
	pthread_cond_broadcast ( &_cond );
	return;
}


unsigned int threadControl::startThreads()
{
	pthread_mutex_init ( &_mutex_client, NULL );
	pthread_mutex_init ( &_mutex_self, NULL );
	pthread_cond_init ( &_cond, NULL );

	const unsigned int sz = _control.size();
	for ( unsigned int i=0; i < sz; ++i )
	{
		void *arg = reinterpret_cast<void *> ( _control[i].tr );
		pthread_create ( &_control[i].ctrl, NULL, start, arg );
	}
	return sz;
}

void threadControl::waitForThreads()
{
	const unsigned int sz = _control.size();
	for ( unsigned int i=0; i < sz; ++i )
	{
		pthread_join ( _control[i].ctrl, NULL );
	}
	pthread_cond_destroy ( &_cond );
	pthread_mutex_destroy ( &_mutex_self );
	pthread_mutex_destroy ( &_mutex_client );
	return;
};

void threadControl::addThread ( thread &t )
{
	control_t c;
	c.tr = &t;
	_control.push_back ( c );
}

unsigned int threadControl::activeThreads() { return _control.size(); }
thread *threadControl::getThread ( unsigned int i ) { return _control[i].tr; }


