#include "classfile.h"
#include "class.h"


symbol * symbol_pool::put(symbol * sym)
{
	auto it = symbols.find(sym->c_str());
	if (it != symbols.end()) return it->second;
	return symbols[sym->c_str()] = sym;
}
		
symbol * symbol_pool::put(const std::string sym)
{
	auto it = symbols.find(sym);
	if (it != symbols.end()) return it->second;
	symbol * s = (symbol*)new char[sizeof(symbol) + sym.length() + 1]();
	strcpy((char*)s->bytes, sym.c_str());
	return symbols[sym] = s;
}
