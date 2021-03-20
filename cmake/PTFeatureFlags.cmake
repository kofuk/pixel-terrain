# SPDX-License-Identifier: MIT

function(define_feature_flag name description defval)
  option(${name} ${description} ${defval})
  if(${name})
    add_definitions(-D${name}=1)
  else()
    add_definitions(-D${name}=0)
  endif()
endfunction()
