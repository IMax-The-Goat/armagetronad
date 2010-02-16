set(DEPS)
set(CLIENTDEPS)
set(BUILDCLIENT TRUE)
set(BUILD TRUE)

macro(DEP VAR NICK)
    if( NOT ${VAR}_FOUND )
        message( SEND_ERROR "${NICK} is required to build ${progtitle}" )
        set(BUILD FALSE)
    endif()
endmacro(DEP)

macro(CLIENTDEP VAR NICK)
    list(APPEND CLIENTDEPS "${VAR}")
    if ( NOT DEDICATEDONLY )
        if( NOT ${VAR}_FOUND )
            message( "${NICK} is required to build a ${progtitle} client" )
            set(BUILDCLIENT FALSE)
        endif()
    endif()
endmacro(CLIENTDEP)

macro(CLEANCLIENTDEPS)
    if ( DEDICATEDONLY OR DEDICATED )
        foreach(dep IN ITEMS ${CLIENTDEPS})
            set(${dep}_FOUND 0)
            set(${dep}_INCLUDE_DIRS "")
            set(${dep}_LIBS "")
        endforeach()
    endif()
endmacro(CLEANCLIENTDEPS)
