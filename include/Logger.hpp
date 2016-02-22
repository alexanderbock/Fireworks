#ifndef _LOGGER_
#define _LOGGER_

#include <iostream>

#ifndef LERROR
#define LERROR(msg) std::cout<<"Error: "<<__FILE__<<"("<<__LINE__<<") - "<<msg<<std::endl;
#endif
#ifndef LDEBUG
#if defined(_DEBUG)
	#define LDEBUG(msg) std::cout<<"Debug: "<<__FILE__<<"("<<__LINE__<<") - "<<msg<<std::endl;
#endif
#endif
#ifndef LINFO
#define LINFO(msg) std::cout<<"Info: "<<__FILE__<<"("<<__LINE__<<") - "<<msg<<std::endl;
#endif

#endif
