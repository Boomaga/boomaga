# +-----------------------------------------------------------------------------+
# |   Copyright (C) 2012                                                        |
# |   Lars B"ahren (lbaehren@gmail.com)                                         |
# |                                                                             |
# |   This program is free software; you can redistribute it and/or modify      |
# |   it under the terms of the GNU General Public License as published by      |
# |   the Free Software Foundation; either version 2 of the License, or         |
# |   (at your option) any later version.                                       |
# |                                                                             |
# |   This program is distributed in the hope that it will be useful,           |
# |   but WITHOUT ANY WARRANTY; without even the implied warranty of            |
# |   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             |
# |   GNU General Public License for more details.                              |
# |                                                                             |
# |   You should have received a copy of the GNU General Public License         |
# |   along with this program; if not, write to the                             |
# |   Free Software Foundation, Inc.,                                           |
# |   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                 |
# +-----------------------------------------------------------------------------+

# - Check for the presence of GHOSTSCRIPT
#
# The following variables are set when GHOSTSCRIPT is found:
#  GHOSTSCRIPT_FOUND      = Set to true, if all components of GHOSTSCRIPT have been found.
#  GHOSTSCRIPT_INCLUDES   = Include path for the header files of GHOSTSCRIPT
#  GHOSTSCRIPT_LIBRARIES  = Link these to use GHOSTSCRIPT
#  GHOSTSCRIPT_LFLAGS     = Linker flags (optional)

if (NOT GHOSTSCRIPT_FOUND)

  if (NOT GHOSTSCRIPT_ROOT_DIR)
    set (GHOSTSCRIPT_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT GHOSTSCRIPT_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (GHOSTSCRIPT_INCLUDES
    NAMES ghostscript/gdevdsp.h ghostscript/iapi.h
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the library

  find_library (GHOSTSCRIPT_LIBRARIES gs
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

  ##_____________________________________________________________________________
  ## Check for the executable

  find_program (GS_EXECUTABLE gs
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )

  find_program (EPS2EPS_EXECUTABLE eps2eps
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )
  
  find_program (DVIPDF_EXECUTABLE dvipdf
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )
  
  find_program (PS2PS2_EXECUTABLE ps2ps2
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )

  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  find_package_handle_standard_args (GHOSTSCRIPT DEFAULT_MSG GHOSTSCRIPT_LIBRARIES GHOSTSCRIPT_INCLUDES GS_EXECUTABLE)

  if (GHOSTSCRIPT_FOUND)
    if (NOT GHOSTSCRIPT_FIND_QUIETLY)
      message (STATUS "Found components for GHOSTSCRIPT")
      message (STATUS "GHOSTSCRIPT_ROOT_DIR  = ${GHOSTSCRIPT_ROOT_DIR}")
      message (STATUS "GHOSTSCRIPT_INCLUDES  = ${GHOSTSCRIPT_INCLUDES}")
      message (STATUS "GHOSTSCRIPT_LIBRARIES = ${GHOSTSCRIPT_LIBRARIES}")
    endif (NOT GHOSTSCRIPT_FIND_QUIETLY)
  else (GHOSTSCRIPT_FOUND)
    if (GHOSTSCRIPT_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find GHOSTSCRIPT!")
    endif (GHOSTSCRIPT_FIND_REQUIRED)
  endif (GHOSTSCRIPT_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    GHOSTSCRIPT_ROOT_DIR
    GHOSTSCRIPT_INCLUDES
    GHOSTSCRIPT_LIBRARIES
    GS_EXECUTABLE
    )

endif (NOT GHOSTSCRIPT_FOUND)
