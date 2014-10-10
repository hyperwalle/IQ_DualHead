// debugalloc.cpp: Debug Extension code
//
// Contians debug_mem_allocation
/*******************************************************************
//		*** COPYRIGHT NOTICE ****
//
//	Copyright 2003. David Maisonave (www.axter.com)
//	
//	This code is free for use under the following conditions:
//
//		You may not claim authorship of this code.
//		You may not sell or distrubute this code without author's permission.
//		You can use this code for your applications as you see fit, but this code may not be sold in any form
//		to others for use in their applications.
//		This copyright notice may not be removed.
//
//
//  No guarantees or warranties are expressed or implied. 
//	This code may contain bugs.
*******************************************************************/
//  Please let me know of any bugs, bug fixes, or enhancements made to this code.
//  If you have ideas for this, and/or tips or admonitions, I would be glad to hear them.
//
//////////////////////////////////////////////////////////////////////
/*
Purpose:
	debug_mem_allocation class:
		The debug_mem_allocation class is used to detect memory leaks.  It does so by replacing the new and delete operator via macros.  It also replaces malloc, realloc, and free functions via macros.
		The macros are setup to only compile for the DEBUG version of the code, and only if USE_DEBUG_MEM_ALLOCATION is defined.
		These macros will log into memory all instance in which the new operator and free function is called.  When a call is made to delete operator or free function, the pointer is removed from the log.  When the program is ready to exit, a call can be made to the following static function:
		debug_ext::debug_mem_allocation::SendOutCurrentLog();
		The above function performs a DebugLogger call for any pointer that was not deleted or free.
Usage:
	debug_mem_allocation class:
		To use this class, the majority of the code should not have to be modified.  However compile errors will occur when the delete[] operator is used in multithread programs, and may occur for casting while using new and when calling delete this.

		For multithread applications, define the DEBUGALLOC_MULTITHREAD_ macro and, 
		calls to delete[] operator must be replaced with the macro DELETE_ARRAY_AND_NULLIFY.
		Example:
			char *pbuffer = new char[33];
			strcpy(pbuffer, "foo foo");
			DELETE_ARRAY_AND_NULLIFY pbuffer;


		On some compilers, code that performs a new, and cast at the same time, might need to be modified.
		Example of original code:
		CStringData* pData1xx = (CStringData*) new BYTE[99];

		Example of workaround method:
		CStringData* pData1xx = (CStringData*) (new BYTE[99]);

		On some compilers delete this will fail to compile.
		Example:
		delete this;

		Example workaround:
		foo* tmp_foo = this;
		delete tmp_foo;
*/
#include "stdafx.h"
#include <stdlib.h>

#ifdef WIN32
#pragma warning(disable : 4786)
#endif //WIN32
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#define		DEBUG_LOG_INCLUDE_FROM_DEBUGLOGGER_CPP_
#include "debugalloc.h"
//#include "..\matlab\libiqapi.h"

namespace debug_ext
{
	static bool StopDeleteLoggingLogic = false;
	struct LocationID
	{
		std::string FileName;
		int LineNo;
		std::string FunctionName;
	};
	
	static std::map<void const * const, LocationID> LogData;;
	void debug_mem_allocation::Log(void const * const Ptr, ACTION_SWITCH Sw)
	{
		if (Sw == AllocateMem)
		{
			LocationID locationid;
			locationid.FileName = m_FileName;
			locationid.LineNo = m_LineNo;
			locationid.FunctionName = m_FuncName;
			LogData[Ptr] = locationid;
		}
		else if (Sw == DeAllocMem)
		{
			std::map<void const * const, LocationID>::iterator i =
				LogData.find(Ptr);
			if (i != LogData.end())
			{
				LogData.erase(i);
			}
		}
	}
	LocationID locationid;
	void debug_mem_allocation::SaveMe()
	{
		locationid.FileName = m_FileName;
		locationid.LineNo = m_LineNo;
		locationid.FunctionName = m_FuncName;
	}
	
	void debug_mem_allocation::SendOutCurrentLog()
	{
//		std::map<void const * const, LocationID>::iterator i = NULL, e = NULL;
		std::map<void const * const, LocationID>::iterator i, e;

      bool   bOKOut = false, bOKIn = false;
      bool   bFirst = true;
      FILE  *TestOut = NULL;
      FILE  *TestIn = NULL;
		for (i = LogData.begin(), e = LogData.end(); i!=e; ++i)
		{
         if ( bFirst ) 
         {
            if ( freopen_s(&TestIn, "CONIN$", "r", stdin) == 0 ) 
               bOKIn = true;
            if ( freopen_s(&TestOut, "CONOUT$", "w", stdout) == 0 )
               bOKOut = true;

            bFirst = false;
         }
			std::cout << "Memory not deallocated from " << i->second.FileName << "(" <<
				i->second.LineNo << ")" << "<" <<
            i->second.FunctionName << ">" << "<pointer address:" <<
            i->first << ">" <<
            std::endl;
		}
		StopDeleteLoggingLogic = true;

      if ( bOKOut && (TestOut != NULL) ) 
         fclose(TestOut);

      if ( bOKIn && (TestIn != NULL) ) 
         fclose(TestIn);

	}
	
	void debug_mem_allocation::free_and_log(void *memblock)
	{
		Log(memblock, DeAllocMem);
		free(memblock);
	}
	
	void* debug_mem_allocation::malloc_and_log(int size )
	{
		void *memblock = malloc(size);
		Log(memblock, AllocateMem);
		return memblock;
	}
	
	void* debug_mem_allocation::realloc_and_log(void *memblock, int size )
	{
		Log(memblock, DeAllocMem);
		memblock = realloc(memblock, size);
		Log(memblock, AllocateMem);
		return memblock;
	}


	////spt1000
#ifdef MATLAB2007a

	mxArray* debug_mem_allocation::mxCreateString_log(const char *stringToCreate)
	{
		mxArray *memblock = mxCreateString(stringToCreate);
		Log(memblock, AllocateMem);
		return memblock;
	}
	mxArray* debug_mem_allocation::mxCreateStructArray_730_log(mwSize ndim, const mwSize *dims, int nfields, const char **fieldnames)
	{
		mxArray *memblock = mxCreateStructArray_730(ndim, dims, nfields, fieldnames);
		Log(memblock, AllocateMem);
		return memblock;
	}
   mxArray* debug_mem_allocation::mxCreateDoubleMatrix_730_log(mwSize m, mwSize n, mxComplexity flag)
   {
      mxArray *memblock = mxCreateDoubleMatrix_730(m ,n, flag);
		Log(memblock, AllocateMem);
		return memblock;
   }
   mxArray *debug_mem_allocation::mxCreateDoubleScalar_log(double value)
   {
      mxArray *memblock = mxCreateDoubleScalar(value);
		Log(memblock, AllocateMem);
		return memblock;
   }
   mxArray *debug_mem_allocation::mxCreateLogicalScalar_log(bool value)
   {
      mxArray *memblock = mxCreateLogicalScalar(value);
		Log(memblock, AllocateMem);
		return memblock;
   }

   bool MW_CALL_CONV debug_mem_allocation::mlfIqapitc_log(int nargout, mxArray** mx_result, mxArray* mx_func, mxArray* mx_input)
   {
       mlfIqapitc(1, mx_result, mx_func, mx_input);	
       Log(*mx_result, AllocateMem);
       return true;
   }

   void debug_mem_allocation::mxDstroyArray_log(mxArray *memblock)
	{
		Log(memblock, DeAllocMem);
		mxDestroyArray(memblock);
	}	
	void debug_mem_allocation::mxSetField_730_log(mxArray *pa, mwIndex i, const char *fieldname, mxArray *memblock)
	{
		Log(memblock, DeAllocMem);
      mxSetField_730(pa, i, fieldname, memblock);
	}

   HANDLE   debug_mem_allocation::CreateThread_log( LPSECURITY_ATTRIBUTES lpThreadAttributes,
                              SIZE_T dwStackSize,
                              LPTHREAD_START_ROUTINE lpStartAddress,
                              LPVOID lpParameter,
                              DWORD dwCreationFlags,
                              LPDWORD lpThreadId
                              )
   {
      HANDLE h = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
      Log(h, AllocateMem);
      return h;
   }
   HANDLE WINAPI debug_mem_allocation::CreateEvent_log(
                               LPSECURITY_ATTRIBUTES lpEventAttributes,
                               BOOL bManualReset,
                               BOOL bInitialState,
                               LPCTSTR lpName
                           )
   {
      HANDLE h = CreateEvent(lpEventAttributes, bManualReset, bInitialState, lpName);
      Log(h, AllocateMem);
      return h;
   }
   HANDLE WINAPI debug_mem_allocation::CreateFile_log(
                               LPCTSTR lpFileName,
                               DWORD dwDesiredAccess,
                               DWORD dwShareMode,
                               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                               DWORD dwCreationDisposition,
                               DWORD dwFlagsAndAttributes,
                               HANDLE hTemplateFile
                           )
   {
      HANDLE h =CreateFile(
                             lpFileName,
                             dwDesiredAccess,
                             dwShareMode,
                             lpSecurityAttributes,
                             dwCreationDisposition,
                             dwFlagsAndAttributes,
                             hTemplateFile);

      Log(h, AllocateMem);
      return h;
   }

   BOOL     debug_mem_allocation::CloseHandle_log(HANDLE h)
   {
      Log(h, DeAllocMem);
      return CloseHandle(h);
   }
#endif
	////ept1000
	
}//End of namespace debug_ext

//#if defined(DEBUG_VERSION_) && defined(USE_DEBUG_MEM_ALLOCATION) && !defined(DEBUG_LOG_INCLUDE_FROM_DEBUGLOGGER_CPP_)
inline void operator delete( void *pvMem )
{
	if (!debug_ext::StopDeleteLoggingLogic && pvMem && debug_ext::LogData.find(pvMem) != debug_ext::LogData.end())
		debug_ext::debug_mem_allocation(debug_ext::locationid.FileName.c_str(), debug_ext::locationid.LineNo, debug_ext::locationid.FunctionName.c_str()).free_and_nullify(pvMem, "na");
	else
		free(pvMem);
}
//#endif

