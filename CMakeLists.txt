cmake_minimum_required(VERSION 3.21)
project(DrunkPlayer)

if(WIN32)
    include_directories(
        "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/include"
        )
endif()

add_executable(DrunkPlayer WIN32
    ayumi/ayumi.c
    ayumi/ayumi.h
    volutar/pt3player.c
    volutar/pt3player.h
    main.c
    music.c
    player.h
    pt3.c
    pt3.h
    sound.c
    )

if(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(dir "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/lib/x86")
    else()
        set(dir "${CMAKE_CURRENT_SOURCE_DIR}/sdl2/lib/x64")
    endif()

    if(MSVC)
        set(outdir "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
    else()
        set(outdir "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    add_custom_command(OUTPUT "${outdir}/SDL2.dll"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${outdir}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${dir}/SDL2.dll" "${outdir}/SDL2.dll"
        )

    add_custom_target(SDL2
        DEPENDS "${outdir}/SDL2.dll"
        SOURCES "${dir}/SDL2.dll"
        )

    add_dependencies(DrunkPlayer SDL2)
    target_link_libraries(DrunkPlayer "${dir}/SDL2main.lib" "${dir}/SDL2.lib")
endif()
