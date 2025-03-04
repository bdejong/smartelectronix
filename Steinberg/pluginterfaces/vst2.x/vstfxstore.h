//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/01/12 09:04:56 $
//
// Category     : VST 2.x Interfaces
// Filename     : vstfxstore.h
// Created by   : Steinberg Media Technologies
// Description  : Definition of Program (fxp) and Bank (fxb) structures
//
// © 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __vstfxstore__
#define __vstfxstore__

#ifndef __aeffect__
#include "aeffect.h"
#endif

//-------------------------------------------------------------------------------------------------------
#define cMagic 				'CcnK'
#define fMagic				'FxCk'
#define bankMagic			'FxBk'
#define chunkGlobalMagic	'FxCh'
#define chunkPresetMagic	'FPCh'
#define chunkBankMagic		'FBCh'

//-------------------------------------------------------------------------------------------------------
/** Program (fxp) structure. */
//-------------------------------------------------------------------------------------------------------
struct fxProgram
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxCk'
	VstInt32 version;			///< format version
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numParams;			///< number of parameters
	char prgName[28];			///< program name
	float params[1];			///< variable sized array with parameter values
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Bank (fxb) structure. */
//-------------------------------------------------------------------------------------------------------
struct fxSet
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxBk'
	VstInt32 version;			///< format version
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numPrograms;		///< number of programs

#if VST_2_4_EXTENSIONS
	VstInt32 currentProgram;	///< current program number
	char future[124];			///< reserved, should be zero
#else
	char future[128];			///< reserved, should be zero
#endif

	fxProgram programs[1];		///< variable number of programs
//-------------------------------------------------------------------------------------------------------
};

//-------------------------------------------------------------------------------------------------------
/** Chunk structure used for fxp and fxb. */
//-------------------------------------------------------------------------------------------------------
struct fxChunkSet
{
//-------------------------------------------------------------------------------------------------------
	VstInt32 chunkMagic;		///< 'CcnK'
	VstInt32 byteSize;			///< size of this chunk, excl. magic + byteSize

	VstInt32 fxMagic;			///< 'FxCh', 'FPCh', or 'FBCh'
	VstInt32 version;			///< format version
	VstInt32 fxID;				///< fx unique ID
	VstInt32 fxVersion;			///< fx version

	VstInt32 numPrograms;		///< number of programs

#if VST_2_4_EXTENSIONS
	VstInt32 currentProgram;	///< current program number
	char future[124];			///< reserved, should be zero
#else
	char future[128];			///< reserved, should be zero
#endif

	VstInt32 chunkSize;			///< size of chunk data
	char chunk[8];				///< variable chunk data
//-------------------------------------------------------------------------------------------------------
};

#endif // __vstfxstore__
