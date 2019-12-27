#!/usr/local/bin/python3.7
import lief
executable = './test'
app = lief.parse(executable)
app.remove_signature()
app.add_library('@loader_path/libpatch.dylib')
app.write('./test_new')
