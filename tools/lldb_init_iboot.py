def __lldb_init_module(debugger, internal_dict):
    print "Loading iBoot debugging from %s" % __file__
    self_path = str(__file__)
    base_dir_name = self_path[:self_path.rfind("/")]
    core_os_plugin = base_dir_name + "/lldb_os_iboot.py"
    osplugin_cmd = "settings set target.process.python-os-plugin-path \"%s\"" % core_os_plugin
    print osplugin_cmd
    debugger.HandleCommand(osplugin_cmd)
    thread_fmt_cmd = 'settings set thread-format thread #${thread.index}{ "${thread.name}"}: tid = ${thread.id}{, ${frame.pc}}{ ${module.file.basename}{`${function.name-with-args}${function.pc-offset}}}{ at ${line.file.basename}:${line.number}}{, stop reason = ${thread.stop-reason}}{\nReturn value: ${thread.return-value}}\n'
    debugger.HandleCommand(thread_fmt_cmd)
