function(set_project_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /WX)
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wno-unused-parameter -Werror
        )
    endif()
endfunction()
