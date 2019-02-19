
project(boomaga_selinux)


option(SELINUX "Build SELinux rules." $ENV{SELINUX})


configure_file(boomaga.fc.in boomaga.fc @ONLY)
configure_file(boomaga.if.in boomaga.if @ONLY)
configure_file(boomaga.te.in boomaga.te @ONLY)



function(buildSelinux _APP _SELINUX_VARIANT)

    message("BUILD ${_SELINUX_VARIANT}")
    add_custom_target("selinux_${_SELINUX_VARIANT}" ALL)

    add_custom_command(TARGET "selinux_${_SELINUX_VARIANT}"
        COMMAND  make NAME=${_SELINUX_VARIANT} -f /usr/share/selinux/devel/Makefile --silent
        COMMAND  mv -f "${_APP}.pp" "${CMAKE_CURRENT_BINARY_DIR}/${_APP}.pp.${_SELINUX_VARIANT}"
        COMMAND  make NAME=${_SELINUX_VARIANT} -f /usr/share/selinux/devel/Makefile clean --silent
        COMMENT "Generating SELinux ${_SELINUX_VARIANT}"
        VERBATIM
    )

    set(pp_file "${CMAKE_INSTALL_PREFIX}/share/selinux/${_SELINUX_VARIANT}/${_APP}.pp")
    set(postinstall "${CMAKE_CURRENT_BINARY_DIR}/postInstall_${_SELINUX_VARIANT}.sh")

    file(WRITE  "${postinstall}"
        "/usr/sbin/semodule -s ${_SELINUX_VARIANT} -i ${pp_file} &> /dev/null || :\n"
        "/sbin/fixfiles restore ${CUPS_BACKEND_DIR}/boomaga || :\n"
    )

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${_APP}.pp.${_SELINUX_VARIANT}"
        DESTINATION share/selinux/${_SELINUX_VARIANT}
        RENAME ${_APP}.pp
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
    )

    install(CODE "execute_process(COMMAND sh ${postinstall})")


endfunction()

if(SELINUX)
    
    buildSelinux("boomaga" "mls")
    buildSelinux("boomaga" "targeted")
else()
    status_message("For building SELinux rules use -DSELINUX=Yes option.")
endif()

