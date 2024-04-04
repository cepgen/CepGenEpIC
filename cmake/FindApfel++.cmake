# APFEL++

find_program(Apfel++_CONFIG apfelxx-config REQUIRED)

if (Apfel++_CONFIG)
  exec_program(${Apfel++_CONFIG}
    ARGS --cppflags
    OUTPUT_VARIABLE Apfel++_CXX_FLAGS
  )
  set(Apfel++_CXX_FLAGS ${Apfel++_CXX_FLAGS} CACHE STRING INTERNAL)
  exec_program(${Apfel++_CONFIG}
    ARGS --ldflags
    OUTPUT_VARIABLE Apfel++_LIBRARIES
  )
  set(Apfel++_LIBRARIES ${Apfel++_LIBRARIES} CACHE STRING INTERNAL)
  exec_program(${Apfel++_CONFIG}
    ARGS --version
    OUTPUT_VARIABLE Apfel++_VERSION
  )
  set(Apfel++_VERSION ${Apfel++_VERSION} CACHE STRING INTERNAL)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Apfel++ REQUIRED_VARS Apfel++_LIBRARIES VERSION_VAR Apfel++_VERSION)
else(Apfel++_CONFIG)
  message(FATAL_ERROR "Apfel++ not found")
endif(Apfel++_CONFIG)
