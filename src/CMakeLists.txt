project(MEGABOT)
cmake_minimum_required(VERSION 2.6)
include(CheckIncludeFiles)
include(CheckSymbolExists)

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX /opt/games/megabot)
endif(NOT CMAKE_INSTALL_PREFIX)


if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
  option(SHOWNOTES "Show preprocessor notes" OFF)
  
  if(CMAKE_COMPILER_IS_GNUCC)
	# GCC specific debug options
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3 -ggdb3 -gdwarf-2")
	set(AVOID_VERSION -avoid-version)
  endif(CMAKE_COMPILER_IS_GNUCC)
endif(CMAKE_BUILD_TYPE MATCHES Debug)

set(SDL_CONFIG "sdl-config" CACHE STRING "Path to sdl-config script")
EXEC_PROGRAM(${SDL_CONFIG} ARGS "--cflags" OUTPUT_VARIABLE SDL_CFLAGS)
EXEC_PROGRAM(${SDL_CONFIG} ARGS "--libs" OUTPUT_VARIABLE SDL_LIBS)
#is there a better way to do this? i should at least check that they exist...
set(SDLIMAGE_LIBS "-lSDL_image")
set(SDLMIXER_LIBS "-lSDL_mixer")



set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ALL_CFLAGS} ${SDL_CFLAGS}")

add_executable(megabot
  megabot.c
  input.c
  scale.c
  util.c
  sprite.c
  zzjson_parse.c
  zzjson_create.c
  zzjson_print.c
  zzjson_query.c
  zzjson_free.c
)

target_link_libraries(megabot
  ${SDL_LIBS}
  ${SDLIMAGE_LIBS}
  ${SDLMIXER_LIBS}
  cwiid
)

install(TARGETS megabot DESTINATION bin)
