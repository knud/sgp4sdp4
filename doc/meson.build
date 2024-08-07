doxygen = find_program('doxygen')
run_command(doxygen, check:true)

sphinx = find_program('sphinx-build')

custom_target('Sphinx html doc',
    command: [sphinx, '-Wq', '-b', 'html', meson.current_source_dir(),
        'Documentation'],
    output: 'en',
    build_by_default: true,
    )
