/*	\file   VideoLibraryFactory.h
	\date   Thursday August 15, 2013
	\author Gregory Diamos <solusstultus@gmail.com>
	\brief  The header file for the VideoLibraryFactory class.
*/

#pragma once

// Standard Library Includes
#include <vector>
#include <string>

// Forward Declarations
namespace minerva { namespace video { class VideoLibrary; } }

namespace minerva
{

namespace video
{

class VideoLibraryFactory
{
public:
	typedef std::vector<VideoLibrary*> VideoLibraryVector;
	
public:
	static VideoLibrary* create(const std::string& name);
	static VideoLibraryVector createAll();
	

};

}

}


