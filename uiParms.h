/*
 * uiparms.h
 *
 *  Created on: Dec 28, 2008
 *      Author: sven
 */

#ifndef UIPARMS_H_
#define UIPARMS_H_
#include <map>
#include <sstream>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>
#include "stdinc.h"
namespace GULI {
	class uiParms {

	public:
		uiParms() : _quiet(false) {};

		/** Derive any parameters you want the UI to control from this class **/
		class varbase;

		/** Add a variable to the control of this instance of uiparm **/
		bool addParm(varbase *v);

		/** Remove a variable from the control of this instance of uiparm **/
		bool delParm(varbase *v);

		varbase *getVar(const string &id);

		/** Create help information for the variable controlled by this instance of uiparm **/
		string describeParms();

		/** For simple applications with one instance of uiparm, make this instance known **/
		static uiParms &defaultParms() {
			return _defaultparm;
		}

		/** Process a command line **/
		int commandLine(int argc, char **argv, bool honor_redir = true);

		string errormessage() {
			return _errmsg;
		}

		bool quiet() const { return _quiet; }
		bool quiet(bool b)  { return _quiet = b; }

		class varbase {
		public:
			varbase(const string &name, const string &help, uiParms &t =
					uiParms::defaultParms());
			virtual ~varbase();
			varbase(const varbase &);

			virtual int eatArgs(int argc, char **argv) {
				std::cerr << "eatArgs not implemented for " << id()
						<< std::endl;
				return 0;
			}

			const string &id() const {
				return _name;
			}

			const string &help() const {
				return _help;
			}

			virtual const string value() const {
				return "";
			}

		protected:
			template<class T> bool _convertArg(const char *, T &t);

		private:
			string _name, _help;
			uiParms *_linked2;
		};

		/** Implements a "no-argument" option, can be interrogated as a bool variable to test if the option
		 * had been present on the command line. If the toggle argument is true, each mention of the option
		 * will invert the current state
		 */
		class TouchVar: public varbase {
		public:
			TouchVar(const string &name, const string &help, uiParms &t =
					uiParms::defaultParms(), bool toggle = false) :
				varbase(name, help, t), _touched(false), _toggle(toggle) {
			}

			virtual int eatArgs(int argc, char **argv) {
				if (_toggle) _touched = !_touched;
				else _touched = true;
				return 0;
			}

			bool operator=(const bool &b) {
				return _touched = b;
			}
			operator bool() const {
				return _touched;
			}

		private:
			bool _touched, _toggle;
		};




		template<class ENUMT>  class Enumvar : public varbase {
		public:

			typedef struct  {
			public:
				const char *label;;
				ENUMT value;
			} enumvarcfg;

			Enumvar(const string &name, const string &help, const  enumvarcfg *cfg, ENUMT v,
					uiParms &t = uiParms::defaultParms()) :
					varbase(name, help, t), _content(_space), _cfg(cfg) {
				_content = v;
			}

			Enumvar(const string &name, const string &help,
					const enumvarcfg *cfg, 	uiParms &t = uiParms::defaultParms()) :
						varbase(name, help, t), _content(_space), _cfg(cfg) {}

			operator ENUMT() const {
				return _content;
			}


			const virtual string value() const {
				for(unsigned int i=0;  *_cfg[i].label;  ++i) {
					if(_content == _cfg[i].value) {
						return _cfg[i].label;
					}
				}
				return "ERR";
			}

			virtual ENUMT operator=(const ENUMT &d){
				return _content = d;
			}

			virtual bool operator==(const ENUMT &d) {
				return _content == d;
			}
			virtual bool operator!=(const ENUMT &d) {
				return _content != d;
			}


		private:
			virtual int eatArgs(int argc, char **argv) {
				if (argc >= 1) {
					for(unsigned int i=0; *_cfg[i].label; ++i) {
						if(!strcmp(_cfg[i].label, argv[0])) {
							_content = _cfg[i].value;
							return 1;
						}
					}
				}
				return -1;
			}


			ENUMT &_content, _space;
			const enumvarcfg * _cfg;
		};


		template<class TYPE> class Variable: public varbase {
		public:

			Variable(const string &name, const string &help, TYPE v,
					uiParms &t = uiParms::defaultParms()) :
				varbase(name, help, t), _content(_space) {
				_content = v;
			}

			Variable(const string &name, const string &help, uiParms &t =
					uiParms::defaultParms()) :
				varbase(name, help, t), _content(_space) {
			}

			operator TYPE() const {
				return _content;
			}

			virtual const string value() const {
				std::stringstream buffer;
				buffer << _content;
				return buffer.str();
			}

			virtual TYPE operator=(const TYPE &d){
				return _content = d;
			}
			virtual bool operator==(const TYPE &d) {
				return _content == d;
			}
			virtual bool operator!=(const TYPE &d) {
				return _content != d;
			}

		protected:
			TYPE &_content;

		private:
			virtual int eatArgs(int argc, char **argv) {
				if (argc >= 1 && _convertArg<TYPE> (argv[0], _content)) return 1;
				return -1;
			}

			TYPE _space;
		};


		template <class TYPE> class VariableVector : public vector<TYPE>, public varbase{
		typedef vector<TYPE> myT;

		public:
			VariableVector(const string &name, const string &help, TYPE v, 	uiParms &t = uiParms::defaultParms()) : varbase(name, help, t) {
				regcomp(&_regex, "([^,]*),?", REG_EXTENDED);
			}

			VariableVector(const string &name, const string &help, uiParms &t = uiParms::defaultParms()) : varbase(name, help, t) {
				regcomp(&_regex, "([^,]*),?", REG_EXTENDED);
			}

			virtual ~VariableVector() {
				regfree(&_regex);
			}

			operator myT() const {
				return *static_cast<myT *>(this);
			}

			virtual const string value() const {
				std::stringstream buffer;
				for(size_t i=0; i < this->size(); ++i) {
					buffer << (*this)[i];
					if(i != this->size()-1) buffer << ',';
				}
				return buffer.str();
			}

			virtual myT operator=(const myT &d){
				return *static_cast<myT *>(this) = d;
			}
			virtual bool operator==(const myT &d) {
				return *static_cast<myT *>(this) == d;
			}
			virtual bool operator!=(const myT &d) {
				return *static_cast<myT *>(this) != d;
			}

		protected:
			virtual int eatArgs(int argc, char **argv) {
				this->clear();
				if (argc >= 1) {
					regmatch_t R[2];
					size_t off = 0;
					const string copy = argv[0];
					while(argv[0][off] && !regexec(&_regex, argv[0]+off, 2, R, REG_NOTBOL)) {
						TYPE a;
						string itm = copy.substr(off + R[1].rm_so, R[1].rm_eo - R[1].rm_so);
						if(!_convertArg(itm.c_str(), a)) return false;
						this->push_back(a);
						off += R[0].rm_eo;
					}
					return 1;
				}
				return -1;
			}


		private:
			regex_t _regex;
		};

	private:
		typedef std::map<string, varbase *> storaget;
		storaget _db;
		static uiParms _defaultparm;
		string _errmsg;
		bool _quiet;
	};
}
#endif /* UIPARMS_H_ */
