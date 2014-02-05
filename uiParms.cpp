/*
 * uiparms.cpp
 *
 *  Created on: Dec 28, 2008
 *      Author: sven
 */
#include <string.h>
#include "uiParms.h"
#include "errorcodes.h"
namespace GULI {

	bool uiParms::addParm(uiParms::varbase *v) {
		uiParms::storaget::iterator f = _db.find(v->id());
		if (f != _db.end()) return false;
		_db[v->id()] = v;
		return true;
	}

	bool uiParms::delParm(varbase *v) {
		uiParms::storaget::iterator f = _db.find(v->id());
		if (f == _db.end()) return false;
		_db.erase(v->id());
		return true;
	}

	uiParms::varbase *uiParms::getVar(const string &id) {
		storaget::iterator f = _db.find(id);
		if (f == _db.end()) return NULL;
		return f->second;
	}

	string uiParms::describeParms() {
		string r = "";
		int tabstop=0;
		for (storaget::iterator f = _db.begin(); f != _db.end(); ++f) {
			const int l =  f->second->id().length();
			if(l > tabstop) tabstop = l;
		}
		tabstop += 3;

		for (storaget::iterator f = _db.begin(); f != _db.end(); ++f) {
			varbase *v = f->second;
			r += "-" + v->id();
			for(int i=0; i < tabstop - v->id().length(); ++i) r += " ";
			r +=  v->help();
			if (v->value() != "") {
				r += " (default: " + v->value() + ")";
			}
			r += "\n";
		}
		return r;
	}

	int uiParms::commandLine(int argc, char **argv, bool honor_redir) {
		int eaten = 0;
		_errmsg = "";
		argc -= 1; // Ignore executable argument
		argv += 1; // ..
		while (argc && **argv == '-') {
			if (honor_redir && !strcmp(*argv, "--")) break;

			string key = *argv + 1;
			if (key == "help") {
				_errmsg = describeParms();
				return -ENEEDHELP;
			}
			varbase *t = getVar(key);
			if (!t) {
				_errmsg = "Unrecognized option " ;
				_errmsg += key;
				if(!_quiet) std::cerr << _errmsg << std::endl;
				return -EUNKNOWNOPTION;
			}

			int r = t->eatArgs(argc - 1, argv + 1);

			if (r < 0) {
				_errmsg = "Can not parse argument for option ";
				_errmsg += key;
				if(!_quiet) std::cerr << _errmsg << std::endl;
				return -ESYNTAX;
			}

			argc -= r + 1;
			argv += r + 1;
			eaten += r + 1;
		}
		return eaten;
	}

	uiParms::varbase::varbase(const string &id, const string &help, uiParms &t) :
		_name(id), _help(help), _linked2(&t) {
		assert(t.addParm(this));
	}

	uiParms::varbase::varbase(const varbase &v) :
		_name(v.id()), _linked2(NULL) {
	}

	uiParms::varbase::~varbase() {
		if (_linked2) assert(_linked2->delParm(this));
	}

	template<> bool uiParms::varbase::_convertArg(const char *a, int &v) {
		char *endptr;
		if (!*a) return false;
		v = strtol(a, &endptr, 0);
		if (*endptr) return false;
		return true;
	}

	template<> bool uiParms::varbase::_convertArg(const char *a, long int &v) {
		char *endptr;
		if (!*a) return false;
		v = strtol(a, &endptr, 0);
		if (*endptr) return false;
		return true;
	}

	template<> bool uiParms::varbase::_convertArg(const char *a, float &v) {
		char *endptr;
		if (!*a) return false;
		v = static_cast<float> (strtod(a, &endptr));
		if (*endptr) return false;
		return true;

	}

	template<> bool uiParms::varbase::_convertArg(const char *a, double &v) {
		char *endptr;
		if (!*a) return false;
		v = strtod(a, &endptr);
		if (*endptr) return false;
		return true;
	}

	template<> bool uiParms::varbase::_convertArg(const char *a, string &v) {
		v = a;
		return true;
	}

	template<> bool uiParms::varbase::_convertArg(const char *a, bool &v) {
		if (!(strcmp(a, "true") && strcmp(a, "TRUE") && strcmp(a, "1"))) {
			v = true;
			return true;
		}

		if (!(strcmp(a, "false") && strcmp(a, "FALSE") && strcmp(a, "0"))) {
			v = false;
			return true;
		}
		return false;
	}

	uiParms uiParms::_defaultparm;
} // NAMESPACE
