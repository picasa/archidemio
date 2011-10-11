# Copyright © INRA
# Author Gauthier Quesnel quesnel@users.sourceforge.net
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# The Software is provided "as is", without warranty of any kind, express or
# implied, including but not limited to the warranties of merchantability,
# fitness for a particular purpose and noninfringement. In no event shall the
# authors or copyright holders be liable for any claim, damages or other
# liability, whether in an action of contract, tort or otherwise, arising from,
# out of or in connection with the software or the use or other dealings in the
# Software.
#

# Try to find Eigen2
# Once done this will define
# Eigen2_FOUND - System has Eigen2
# Eigen2_INCLUDE_DIRS - The Eigen2 include directories


find_path(Eigen2_INCLUDE_DIR
  NAMES Eigen
  PATHS /usr/include/eigen2/Eigen /usr/include/eigen2)

set(Eigen2_INCLUDE_DIRS ${Eigen2_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Eigen2 DEFAULT_MSG Eigen2_INCLUDE_DIR)

mark_as_advanced(Eigen2_INCLUDE_DIR)


