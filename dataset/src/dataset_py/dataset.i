%module dataset

%include "typemaps.i"
%include "carrays.i"

%{
#define SWIG_FILE_WITH_INIT
#include "dataset/dataset_arrays.h"
#include "tau_tools/owTauGrid.h"
#include "tau_tools/tau_tools.h"
#include "cgns_tools/cgnsGrid.h"
#include "cgns_tools/cgns_tools.h"
%}

%include "../dataset/dataset_arrays.h"
%include "../tau_tools/owTauGrid.h"
%include "../tau_tools/tau_tools.h"
%include "../cgns_tools/cgnsGrid.h"
%include "../cgns_tools/cgns_tools.h"


%extend owDoubleStream {
	double __getitem__(size_t i) {
		if (i < (*self).length){
			return (*self).stream[i];
		}
		else{
			throw("out of bounds!");
		}
	}

	void __setitem__(size_t i, const double value) {
		if (i < (*self).length){
			(*self).stream[i] = value;
		}
		else{
			throw("out of bounds!");
		}
	}
}

%extend owIntStream {
	int __getitem__(size_t i) {
		if (i < (*self).length){
			return (*self).stream[i];
		}
		else{
			throw("out of bounds!");
		}
	}

	void __setitem__(size_t i, const int value) {
		if (i < (*self).length){
			(*self).stream[i] = value;
		}
		else{
			throw("out of bounds!");
		}
	}
}

%extend owVector3dStream {
	owVector3d __getitem__(size_t i) {
		if (i < (*self).length){
			return (*self).stream[i];
		}
		else{
			throw("out of bounds!");
		}
	}

	void __setitem__(size_t i, const owVector3d value) {
		if (i < (*self).length){
			(*self).stream[i] = value;
		}
		else{
			throw("out of bounds!");
		}
	}
}

%extend _cgnsSection {
	_cgnsSection* __getitem__(size_t i) {
		return &(self[i]);
	}
}

%extend _cgnsGrid {
	_cgnsGrid* __getitem__(size_t i) {
		return &(self[i]);
	}
}

