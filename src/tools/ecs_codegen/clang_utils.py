import clang.cindex
import os.path
import sys
import subprocess
from shutil import which
import re
from clang.cindex import TranslationUnit as TU

clang_options = (TU.PARSE_DETAILED_PROCESSING_RECORD
                 | TU.PARSE_SKIP_FUNCTION_BODIES
                 | TU.PARSE_INCOMPLETE
                 # | TU.PARSE_INCLUDE_BRIEF_COMMENTS_IN_CODE_COMPLETION
)

debug_mode = True
def dbg(*args, **kwargs):
    if debug_mode:
        print(*args, **kwargs, file=sys.stderr, flush=True)

def get_output(cmd):
    status, output = subprocess.getstatusoutput(cmd)
    if status != 0:
        msg = "command failed with status {}: {}\noutput was:\n{}"
        msg = msg.format(status, cmd, output)
        raise Exception(msg)
    return output

def in_windows():
    return sys.platform == "win32"

def fileline(node):
    l = node.location
    return str(l.file) + ":" + str(l.line)

def get_diagnostics_string(diags):
    s = ""
    if diags is not None:
        for d in diags:
            fl = fileline(d)
            s += '\n{0}: {1}: {2}'.format(fl, d.severity, d.spelling)
            if d.fixits is not None:
                for f in d.fixits:
                    s += '\n{0}:     {1}'.format(fl, str(f))
        s += "\n"
    return s

def find_llvm_config():
    def _dbg(*args, **kwargs): dbg("looking for llvm-config:", *args, **kwargs)
    exe = which("llvm-config")
    _dbg("'llvm-config' in path?")
    if exe is None:
        _dbg("'llvm-config' not found")
    else:
        _dbg("'llvm-config' found:", exe)
        llvmc_version = get_output(exe + " --version")
        _dbg("'llvm-config' version:", llvmc_version)
        return exe
    return exe


def find_clangxx_exe():
    def _dbg(*args, **kwargs): dbg("looking for clang++:", *args, **kwargs)
    llvmc = find_llvm_config()
    if llvmc is not None:
        bindir = get_output(llvmc + " --bindir")
        _dbg("result for '" + llvmc + " --bindir':", bindir)
        clangexe = os.path.join(bindir, 'clang++')
        if in_windows():
            clangexe += '.exe'
        _dbg("testing clang++:", clangexe)
        if os.path.exists(clangexe):
            _dbg("gotit:", clangexe)
        return clangexe
    exe = which("clang++")
    if not exe:
        _dbg("not found in path")
        return None
    _dbg("found it:", exe)
    fullversion = get_output(exe + " --version")
    _dbg("fullversion:\n" + fullversion)
    version = re.sub(r"clang version ([0-9].[0-9]).*", r'\1',
                        fullversion.split("\n")[0])
    return exe


def load_clang(libdir=None):
    def _dbg(*args, **kwargs): dbg("loading libclang:", *args, **kwargs)
    _dbg("LD_LIBRARY_PATH=", os.environ.get("LD_LIBRARY_PATH"))
    _dbg("PATH=", os.environ["PATH"])
    _dbg("LLVM_LIB_DIR=", os.environ["LLVM_LIB_DIR"])
    _dbg("LIBCLANG_PATH=", os.environ["LIBCLANG_PATH"])

    if libdir is None:
        libdir = os.environ.get("LIBCLANG_PATH")
        if libdir is None:
             libdir = os.path.join(os.path.dirname(os.environ.get("LLVM_LIB_DIR")), "bin")

    if libdir is None:
        exe = find_llvm_config()
        _dbg("llvm-config=", exe)
        clangxx = find_clangxx_exe()
        _dbg("clang++=", clangxx)
        if exe:
            libdir = get_output(exe + " --libdir")
        else:
            _dbg("could not find llvm-config...")
            if in_windows():
                # assuming vanilla installation
                if clangxx is None:
                    raise Exception("found none of llvm-config and clang++. " +
                                    "make sure at least one of these can be found in your PATH")
        if in_windows():
            libdir = os.path.dirname(clangxx)
        if libdir is None:
            msg = ("could not find a suitable clang lib directory.")
            raise Exception(msg)
        _dbg("libdir=", libdir)
    if libdir is not None:
        clang.cindex.Config.set_library_path(libdir)  # this doesn't work, at least in Ubuntu 16.04 x64
        if in_windows():
            libname = "libclang.dll"
            _dbg("in windows: assuming libdir=", libdir)
        else:
            libname = "libclang.so"
        lib = os.path.join(libdir, libname)
        _dbg("testing libclang=", lib)
        if not os.path.exists(lib):
            _dbg("lib not found!")
            raise Exception("lib not found: " + lib)
        assert os.path.exists(lib)
        clang.cindex.Config.set_library_file(lib)
    return lib


def parse_file(filename, args=[], options=clang_options):

    Exception("Q")
    def _dbg(*args, **kwargs): dbg("parse_file:", filename + ":", *args, **kwargs)
    if not os.path.exists(filename):
        raise Exception("file not found: " + filename)
    #p = get_inc_path()
    dbg("args=", args)
   #dbg("inc_path=", ip)
   # args =args
    def _e(diags=None):
        msg = "{}: parse error: {}"
        msg = msg.format(filename, get_diagnostics_string(diags))
        return msg

    dbg("creating index...")
    idx = clang.cindex.Index.create()
    dbg("successfully created index.")
    tu = None
    try:
       # if util.is_hdr(filename):
       #     options |= TU.PARSE_INCOMPLETE
        dbg("starting parse")
        tu = idx.parse(path=filename, args=args, options=options)
        dbg("finished parse")
        if tu.diagnostics:
            dbg(_e(tu.diagnostics))
    except Exception as e:
        if tu:
            dbg(_e(tu.diagnostics))
        dbg(e)
    if not tu:
        raise Exception("Unknown error")
    return tu
