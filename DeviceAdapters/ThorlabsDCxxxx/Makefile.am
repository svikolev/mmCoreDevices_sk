
AM_CXXFLAGS = $(MMDEVAPI_CXXFLAGS)
deviceadapter_LTLIBRARIES = libmmgr_dal_ThorlabsDCxxxx.la

libmmgr_dal_ThorlabsDCxxxx_la_SOURCES =  \
			  DC2XXX.cpp \
			  DC2XXX.h \
			  DC3100.cpp \
			  DC3100.h \
			  DC4100.cpp \
			  DC4100.h \
			  DCxxxx_Plugin.cpp \
			  DCxxxx_Plugin.h \
			  DynError.cpp \
			  DynError.h
# Note that DC2200 requires a Windows-only SDK and should not be listed here.

libmmgr_dal_ThorlabsDCxxxx_la_LDFLAGS = $(MMDEVAPI_LDFLAGS) 
libmmgr_dal_ThorlabsDCxxxx_la_LIBADD = $(MMDEVAPI_LIBADD)
