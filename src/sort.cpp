/*
 * filename  :	sort.cpp
 *
 * programmer:	Cao Jiayin
 */

// include the header file
#include "sort.h"
#include "managers/texmanager/texmanager.h"
#include "managers/logmanager.h"
#include "utility/timer.h"

// initialize sort system
bool	initSort()
{
	// initialize texture manager
	TexManager::CreateTexManager();
	// initialize log manager
	LogManager::CreateLogManager();
	// initialize the timer
	Timer::CreateTimer();

	return true;
}

// release sort
void	releaseSort()
{
	// release managers
	TexManager::DeleteSingleton();
	LogManager::DeleteSingleton();
	Timer::DeleteSingleton();
}
