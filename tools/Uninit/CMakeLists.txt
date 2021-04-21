
if(DEFINED IN_SOURCE_BUILD)
    set(LLVM_LINK_COMPONENTS BitWriter Core IPO IrReader InstCombine Instrumentation Target Linker Analysis ScalarOpts Support Svf Cudd)
    add_llvm_tool( uninit uninit.cpp )
else()
    add_executable( uninit uninit.cpp )

    target_link_libraries( uninit Svf Cudd ${llvm_libs} )

    set_target_properties( uninit PROPERTIES
                           RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
endif()
